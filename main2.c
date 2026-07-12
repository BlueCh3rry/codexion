#include "codexion.h"

void ft_signal(t_data *data)
{
    pthread_mutex_lock(&data->state_mutex);
    data->signal_count++;
    pthread_cond_broadcast(&data->cond_thread);
    pthread_mutex_unlock(&data->state_mutex);
}

void    *coder_chrono(void *arg)
{
    t_data  *data;
    int     i;

    data = (t_data *)arg;
    while (1)
    {
        pthread_mutex_lock(&data->state_mutex);
        if (data->done)
        {
            pthread_mutex_unlock(&data->state_mutex);
            break;
        }
        i = 0;
        while (i < data->number_of_coders)
        {
            if (data->coders[i].last_compile_start != 0 &&
                current_time_ms() - data->coders[i].last_compile_start >= data->time_to_burnout / 1000)
            {
                log_state(data, data->coders[i].id, "burned ouuuuuuuuuuuuuuuuuuuuuuuuuuuut");
                data->done = 1;
                pthread_cond_broadcast(&data->cond_thread);
                pthread_mutex_unlock(&data->state_mutex);
                return (NULL);
            }
            i++;
        }
        pthread_mutex_unlock(&data->state_mutex);
        usleep(1000);
    }
    return (NULL);
}

int    compile(t_c *coder)
{
    long    cd;

    if (coder->left->id < coder->right->id)
    {
        pthread_mutex_lock(&coder->left->mutex);
        log_state(coder->data, coder->id, "has taken a dongle");
        pthread_mutex_lock(&coder->right->mutex);
        log_state(coder->data, coder->id, "has taken a dongle");
    }
    else
    {
        pthread_mutex_lock(&coder->right->mutex);
        log_state(coder->data, coder->id, "has taken a dongle");
        pthread_mutex_lock(&coder->left->mutex);
        log_state(coder->data, coder->id, "has taken a dongle");
    }
    
    pthread_mutex_lock(&coder->data->state_mutex);
    if (coder->data->done)
    {
        pthread_mutex_unlock(&coder->data->state_mutex);
        pthread_mutex_unlock(&coder->left->mutex);
        pthread_mutex_unlock(&coder->right->mutex);
        return (1);
    }
    log_state(coder->data, coder->id, "is compiling");
    coder->last_compile_start = current_time_ms();
    pthread_mutex_unlock(&coder->data->state_mutex);
    
    while (current_time_ms() - coder->last_compile_start < coder->data->time_to_compile / 1000)
    {
        pthread_mutex_lock(&coder->data->state_mutex);
        if (coder->data->done)
        {
            pthread_mutex_unlock(&coder->data->state_mutex);
            pthread_mutex_unlock(&coder->left->mutex);
            pthread_mutex_unlock(&coder->right->mutex);
            return (1); 
        }
        pthread_mutex_unlock(&coder->data->state_mutex);
        usleep(500); 
    }
    
    pthread_mutex_lock(&coder->data->state_mutex);
    coder->last_compile_start = 0;
    cd = coder->data->dongle_cooldown;
    coder->left->available_at = current_time_ms() - coder->data->start_time + cd;
    coder->right->available_at = current_time_ms() - coder->data->start_time + cd;
    pthread_mutex_unlock(&coder->data->state_mutex);
    
    pthread_mutex_unlock(&coder->left->mutex);
    pthread_mutex_unlock(&coder->right->mutex);
    return (0);
}

int coder_can_compile(t_c *coder)
{
    long now;

    now = current_time_ms() - coder->data->start_time;
    if (coder->left->available_at > 0 && now < coder->left->available_at)
        return (0);
    if (coder->right->available_at > 0 && now < coder->right->available_at)
        return (0);
    return (1);
}

void    *coder_routine(void *arg)
{
    t_c *coder;
    int j;

    coder = (t_c *)arg;
    j = 0;
    while (j < coder->data->number_of_compiles_required)
    {   
        pthread_mutex_lock(&coder->data->state_mutex);
        while (coder_can_compile(coder) == 0 && coder->data->done == 0)
            pthread_cond_wait(&coder->data->cond_thread, &coder->data->state_mutex);
        
        if (coder->data->done)
        {
            pthread_mutex_unlock(&coder->data->state_mutex);
            break;
        }
        pthread_mutex_unlock(&coder->data->state_mutex);
        
        if (compile(coder))
            break;
        
        pthread_mutex_lock(&coder->data->state_mutex);
        if (coder->data->done)
        {
            pthread_mutex_unlock(&coder->data->state_mutex);
            break;
        }
        if (!strcmp(coder->data->scheduler, "edf"))
            pthread_cond_broadcast(&coder->data->cond_thread);
        else
            pthread_cond_signal(&coder->data->cond_thread);
        pthread_mutex_unlock(&coder->data->state_mutex);
        
        log_state(coder->data, coder->id, "is debugging");
        usleep(coder->data->time_to_debug);
        
        pthread_mutex_lock(&coder->data->state_mutex);
        if (coder->data->done)
        {
            pthread_mutex_unlock(&coder->data->state_mutex);
            break;
        }
        pthread_mutex_unlock(&coder->data->state_mutex);

        log_state(coder->data, coder->id, "is refactoring");
        usleep(coder->data->time_to_refactor);
        j++;
    }
    return (NULL);
}

int main(int argc, char **argv)
{
    t_c             *coders;
    t_d             *dongles;
    t_data          data;
    int             i;

    if (argc != 9)
    {
        printf("Error arg\n");
        return (0);
    }
    printf("\n===Codexion===\n\n");
    if (atoi(argv[1]) <= 0)
    {
        printf("Error [1] number of coders is under 1\n");
        return (0);
    }
    data.number_of_coders = atoi(argv[1]);
    data.time_to_burnout = atoi(argv[2]) * 1000;
    data.time_to_compile = atoi(argv[3]) * 1000;
    data.time_to_debug = atoi(argv[4]) * 1000;
    data.time_to_refactor = atoi(argv[5]) * 1000;
    data.number_of_compiles_required = atoi(argv[6]);
    data.dongle_cooldown = atoi(argv[7]) * 1000;

    if (!check_sched(argv[8]))
    {
        printf("Error [8] scheduler is neither fifo nor edf\n");
        return (0);
    }
    data.scheduler = malloc(strlen(argv[8]) + 1);
    if (!data.scheduler)
    {
        printf("Error malloc scheduler\n");
        return (0);
    }
    ft_strlcpy(data.scheduler, argv[8]);

    coders = malloc(sizeof(t_c) * data.number_of_coders);
    if (!coders)
    {
        printf("Error malloc coders\n");
        free(data.scheduler);
        return (0);
    }
    dongles = malloc(sizeof(t_d) * data.number_of_coders);
    if (!dongles)
    {
        printf("Error malloc dongles\n");
        free(data.scheduler);
        free(coders);
        return (0);
    }
    data.dongles = dongles;
    data.coders = coders;
    
    pthread_cond_init(&data.cond_thread, NULL);
    pthread_mutex_init(&data.log_mutex, NULL);
    pthread_mutex_init(&data.state_mutex, NULL);
    data.done = 0;
    data.start_time = current_time_ms();
    
    i = 0;
    while (i < data.number_of_coders)
    {
        dongles[i].id = i + 1;
        dongles[i].available_at = 0;
        pthread_mutex_init(&dongles[i].mutex, NULL);
        i++;
    }
    i = 0;
    while (i < data.number_of_coders)
    {
        coders[i].left  = &dongles[i];
        coders[i].right = &dongles[(i + 1) % data.number_of_coders];
        coders[i].last_compile_start = 0;
        coders[i].id = i + 1;
        coders[i].data = &data;
        i++;
    }
    
    pthread_create(&data.c_thread, NULL, coder_chrono, &data);
    
    i = 0;
    while (i < data.number_of_coders)
    {
        pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
        i++;
    }
    
    i = 0;
    while (i < data.number_of_coders)
    {
        pthread_join(coders[i].thread, NULL);
        i++;
    }
    
    pthread_mutex_lock(&data.state_mutex);
    data.done = 1;
    pthread_cond_broadcast(&data.cond_thread);
    pthread_mutex_unlock(&data.state_mutex);
    
    pthread_join(data.c_thread, NULL);
    
    i = 0;
    while (i < data.number_of_coders)
    {
        pthread_mutex_destroy(&dongles[i].mutex);
        i++;
    }
    pthread_cond_destroy(&data.cond_thread);
    pthread_mutex_destroy(&data.log_mutex);
    pthread_mutex_destroy(&data.state_mutex);
    
    printf("END\n");
    free(data.scheduler);
    free(coders);
    free(dongles);
    return (0);
}