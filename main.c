/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 15:46:57 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/06/09 15:47:08 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void ft_signal(t_data *data)
{
    // pthread_mutex_lock(&data->state_mutex);
    data->signal_count++;
    pthread_cond_broadcast(&data->cond_thread);
    // pthread_mutex_unlock(&data->state_mutex);
}

// void	clean_up(t_data *data)
// {
// 	int i;

// 	i = 0;
// 	while (i < data->number_of_coders)
// 	{
// 		pthread_join(data->coders[data->number_of_coders].thread, NULL);
// 		pthread_mutex_destroy(&data->dongles[data->number_of_coders].mutex);
// 		i++;
// 	}
// 	pthread_cond_destroy(&data->cond_thread);
// 	pthread_mutex_destroy(&data->log_mutex);
// 	printf("ENeD\n");
// 	free(data->scheduler);
// 	free(data->coders);
// 	free(data->dongles);
// 	exit(0);
// }

void	*coder_chrono(void *arg)
{
	t_data	*data;
	int		i;

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
				// pthread_cond_broadcast(&data->cond_thread);
				pthread_mutex_unlock(&data->state_mutex);
				return (NULL);
			}
			i++;
		}
		pthread_mutex_unlock(&data->state_mutex);
        // ENLEVE CA POUR PAS CRASH EN LEAK
		usleep(1);
	}
	return (NULL);
}

void	compile(t_c *coder)
{
	long	cd;

	// printf("number=%d coder=%p left=%p right=%p\n",
	// 	coder->id,
	// 	(void *)coder,
	// 	(void *)coder->left,
	// 	(void *)coder->right);
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
	log_state(coder->data, coder->id, "is compiling");
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = current_time_ms();
	pthread_mutex_unlock(&coder->data->state_mutex);
	// usleep(coder->data->time_to_compile);
	while (current_time_ms() - coder->last_compile_start < coder->data->time_to_compile / 1000)
	{
		pthread_mutex_lock(&coder->data->state_mutex);
		if (coder->data->done)
		{
			// pthread_mutex_unlock(&coder->data->state_mutex);
			// pthread_mutex_unlock(&coder->left->mutex);
			// pthread_mutex_unlock(&coder->right->mutex);
			// clean_up(coder->data);
			exit(0);
			// return;
		}
		pthread_mutex_unlock(&coder->data->state_mutex);
	}
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = 0;
	cd = coder->data->dongle_cooldown;
	coder->left->available_at = current_time_ms() - coder->data->start_time + cd;
	coder->right->available_at = current_time_ms() - coder->data->start_time + cd;
	pthread_mutex_unlock(&coder->data->state_mutex);
	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_unlock(&coder->right->mutex);
}

int coder_can_compile(t_c *coder)
{
    long now;

	now = current_time_ms() - coder->data->start_time;
	// printf("NOW =%ld\nGOAL =%ld\n", now, coder->left->available_at);
	if (coder->left->available_at > 0 && now >= coder->left->available_at)
		return (0);
	if (coder->right->available_at > 0 && now >= coder->right->available_at)
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
			printf("STOP\n");
			break;
		}
		pthread_mutex_unlock(&coder->data->state_mutex);
		compile(coder);
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
        
		// ft_signal(coder->data); A FINIR ICI ########################################################################################################################################################################
		// pthread_mutex_unlock(&coder->data->state_mutex);
		log_state(coder->data, coder->id, "is debugging");
		usleep(coder->data->time_to_debug);
		log_state(coder->data, coder->id, "is refactoring");
		usleep(coder->data->time_to_refactor);
		pthread_mutex_lock(&coder->data->state_mutex);
		// printf("YEP CODER=%d\n", coder->id);
		pthread_mutex_unlock(&coder->data->state_mutex);
		j++;
	}
	return (NULL);
}

int	main(int argc, char **argv)
{
	t_c				*coders;
	t_d				*dongles;
	t_data			data;
	int				i;

	if (argc != 9)
	{
		printf("Error arg");
		return (0);
	}
	printf("\n===Codexion===\n\n");
	if (atoi(argv[1]) <= 0)
	{
		printf("Error [1] number of coders is under 1\n");
		return (0);
	}
	data.number_of_coders = atoi(argv[1]);
	if (atoi(argv[2]) <= 0)
	{
		printf("Error [2] time to burnout is under 1\n");
		return (0);
	}
	data.time_to_burnout = atoi(argv[2]) * 1000;
	if (atoi(argv[3]) <= 0)
	{
		printf("Error [3] time to compile is under 1\n");
		return (0);
	}
	data.time_to_compile = atoi(argv[3]) * 1000;
	if (atoi(argv[4]) <= 0)
	{
		printf("Error [4] time to debug is under 1\n");
		return (0);
	}
	data.time_to_debug = atoi(argv[4]) * 1000;
	if (atoi(argv[5]) <= 0)
	{
		printf("Error [5] time to refactor is under 1\n");
		return (0);
	}
	data.time_to_refactor = atoi(argv[5]) * 1000;
	if (atoi(argv[6]) <= 0)
	{
		printf("Error [6] number of compiles required is UNDER 1\n");
		return (0);
	}
	else if (atoi(argv[6]) > data.number_of_coders)
	{
		printf("Error [6] number of compiles required is ABOVE the number of coders\n");
		return (0);
	}
	data.number_of_compiles_required = atoi(argv[6]);
	if (atoi(argv[7]) < 0)
	{
		printf("Error [7] dongle cooldown is under 0\n");
		return (0);
	}
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
	// Program starts here ##############################

	coders = malloc(sizeof(t_c) * data.number_of_coders);
	if (!coders)
	{
		printf("Error malloc coders\n");
		return (free(data.scheduler), 0);
	}
	dongles = malloc(sizeof(t_d) * data.number_of_coders);
	if (!dongles)
	{
		printf("Error malloc dongles\n");
		return (free(data.scheduler), free(coders), 0);
	}
	data.dongles = dongles;
	data.coders = coders;
	pthread_cond_init(&data.cond_thread, NULL);
	pthread_mutex_init(&data.log_mutex, NULL);
	pthread_mutex_init(&data.state_mutex, NULL);
	data.done = 0;
	i = 0;
	data.start_time = current_time_ms();
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
		i++;
	}
	pthread_create(&data.c_thread, NULL, coder_chrono, &data);
	i = 0;
	while (i < data.number_of_coders)
	{
		pthread_mutex_lock(&data.state_mutex);
		coders[i].last_compile_start = 0;
		pthread_mutex_unlock(&data.state_mutex);
		coders[i].id = i + 1;
		coders[i].data = &data;
		pthread_create(&data.coders[i].thread, NULL, coder_routine, &data.coders[i]);
		i++;
	}
	i = 0;
	while (i < data.number_of_coders)
	{
		pthread_join(data.coders[i].thread, NULL);
		pthread_mutex_destroy(&data.dongles[i].mutex);
		i++;
	}
	pthread_mutex_lock(&data.state_mutex);
	data.done = 1;
	pthread_cond_broadcast(&data.cond_thread);
	pthread_mutex_unlock(&data.state_mutex);
	pthread_join(data.c_thread, NULL);
	pthread_cond_destroy(&data.cond_thread);
	pthread_mutex_destroy(&data.log_mutex);
	printf("END\n");
	free(data.scheduler);
	free(data.coders);
	free(data.dongles);
	return (0);
}
