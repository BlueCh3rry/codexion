/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:52:43 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/14 17:52:44 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

static void	start_threads(t_data *data)
{
	int	i;

	pthread_create(&data->c_thread, NULL, coder_chrono, data);
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_create(
			&data->coders[i].thread,
			NULL,
			coder_routine,
			&data->coders[i]);
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
