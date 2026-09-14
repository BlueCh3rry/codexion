#include "codexion.h"

static int	parse_args1(char **argv, t_data *data)
{
	if (atoi(argv[1]) <= 1)
		return (printf("Error [1] number of coders is too low\n"), -1);
	data->number_of_coders = atoi(argv[1]);
	if (atoi(argv[2]) <= 1)
		return (printf("Error [2] time to burnout is too low\n"), -1);
	data->time_to_burnout = atoi(argv[2]) * 1000;
	if (atoi(argv[3]) <= 1)
		return (printf("Error [3] time to compile is too low\n"), -1);
	data->time_to_compile = atoi(argv[3]) * 1000;
	if (atoi(argv[4]) <= 1)
		return (printf("Error [4] time to debug is too low\n"), -1);
	data->time_to_debug = atoi(argv[4]) * 1000;
	return (0);
}

static int	parse_args2(char **argv, t_data *data)
{
	if (atoi(argv[5]) <= 1)
		return (printf("Error [5] time to refactor is too low\n"), -1);
	data->time_to_refactor = atoi(argv[5]) * 1000;
	if (atoi(argv[6]) <= 0)
		return (printf("Error [6] number of compiles required is too low\n"), -1);
	data->number_of_compiles_required = atoi(argv[6]);
	if (atoi(argv[7]) <= 1)
		return (printf("Error [7] dongle cooldown is under 0\n"), -1);
	data->dongle_cooldown = atoi(argv[7]) * 1000;
	if (!check_sched(argv[8]))
		return (printf("Error [8] scheduler is neither fifo nor edf\n"), -1);
	return (0);
}

static int	init_scheduler(t_data *data, char *sched)
{
	data->scheduler = malloc(strlen(sched) + 1);
	if (!data->scheduler)
	{
		printf("Error malloc scheduler\n");
		return (-1);
	}
	ft_strcpy(data->scheduler, sched);
	return (0);
}

static int	alloc_structures(t_data *data)
{
	data->coders = malloc(sizeof(t_c) * data->number_of_coders);
	if (!data->coders)
	{
		printf("Error malloc coders\n");
		return (-1);
	}
	data->dongles = malloc(sizeof(t_d) * data->number_of_coders);
	if (!data->dongles)
	{
		printf("Error malloc dongles\n");
		free(data->coders);
		return (-1);
	}
	return (0);
}

static void	init_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		data->dongles[i].id = i + 1;
		data->dongles[i].available_at = 0;
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
		i++;
	}
}

static void	init_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		data->coders[i].left = &data->dongles[i];
		data->coders[i].right = &data->dongles[(i + 1) % data->number_of_coders];
		data->coders[i].last_compile_start = 0;
		data->coders[i].id = i + 1;
		data->coders[i].data = data;
		i++;
	}
}

static void	start_threads(t_data *data)
{
	int	i;

	pthread_create(&data->c_thread, NULL, coder_chrono, data);
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_create(&data->coders[i].thread, NULL, coder_routine, &data->coders[i]);
		i++;
	}
}

static void	join_threads(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_mutex_destroy(&data->dongles[i].mutex);
		i++;
	}
}

static void	cleanup(t_data *data)
{
	pthread_mutex_lock(&data->state_mutex);
	data->done = 1;
	pthread_cond_broadcast(&data->cond_thread);
	pthread_mutex_unlock(&data->state_mutex);
	pthread_join(data->c_thread, NULL);
	pthread_cond_destroy(&data->cond_thread);
	pthread_mutex_destroy(&data->log_mutex);
	printf("END\n");
	free(data->scheduler);
	free(data->coders);
	free(data->dongles);
}

int	main(int argc, char **argv)
{
	t_data	data;

	if (argc != 9)
	{
		printf("Error arg");
		return (0);
	}
	if (parse_args1(argv, &data) == -1 || parse_args2(argv, &data) == -1)
		return (0);
	if (init_scheduler(&data, argv[8]) == -1)
		return (0);
	if (alloc_structures(&data) == -1)
		return (free(data.scheduler), 0);
	data.order = 1;
	data.done = 0;
	pthread_cond_init(&data.cond_thread, NULL);
	pthread_mutex_init(&data.log_mutex, NULL);
	pthread_mutex_init(&data.state_mutex, NULL);
	data.start_time = current_time_ms();
	init_dongles(&data);
	init_coders(&data);
	start_threads(&data);
	join_threads(&data);
	cleanup(&data);
	return (0);
}
