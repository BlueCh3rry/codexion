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

void	*coder_chrono(void *arg)
{
	t_data			*data;
	t_c				*coder;
	int				i;
	long 			start;

	i = 0;
	data = (t_data *)arg;
	while (1)
	{
		while (&data->coders[i].done == 0 && i < data->number_of_coders)
		{
			coder = &data->coders[i];
			if (coder->last_compile_start != 0)
			{
				start = coder->last_compile_start;
				if (current_time_ms() - start >= data->time_to_burnout)
					log_state(data, coder->id, "burned out");
			}
			i++;
		}
		i %= data->number_of_coders; 
	}
}

void	compile(t_c *cod)
{
	t_c		*coder;

	coder = cod;
	if (coder->left->id < coder->right->id)
	{
		pthread_mutex_lock(&coder->left->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
		usleep(coder->data->dongle_cooldown);
		pthread_mutex_lock(&coder->right->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
	}
	else
	{
		pthread_mutex_lock(&coder->right->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
		usleep(coder->data->dongle_cooldown);
		pthread_mutex_lock(&coder->left->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
	}
	log_state(coder->data, coder->id, "is compiling");
	coder->last_compile_start = current_time_ms();
	usleep(coder->data->time_to_compile);
	pthread_mutex_unlock(&coder->left->mutex);
	coder->left->taken = 0;
	pthread_mutex_unlock(&coder->right->mutex);
	coder->right->taken = 0;
}

void	*coder_routine(void *arg)
{
	t_c		*coder;

	coder = (t_c *)arg;
	if (coder->left && coder->right)
	{
		// wait
	}
	else if (!coder->left && coder->right)
	{
		// wait
	}
	else if (coder->left && !coder->right)
	{
		// wait
	}
	compile(coder);
	if (!strcmp(coder->data->scheduler, "edf"))
		pthread_cond_broadcast(&coder->data->cond_thread);
	else
		pthread_cond_signal(&coder->data->cond_thread);
	log_state(coder->data, coder->id, "is debugging");
	usleep(coder->data->time_to_debug);
	log_state(coder->data, coder->id, "is refactoring");
	usleep(coder->data->time_to_refactor);
	/*
	if (!strcmp(coder->data->scheduler, "edf"))
	{
		long deadline = coder->last_compile_start + coder->data->time_to_burnout;
		if (current_time_ms() - coder->last_compile_start == deadline)
		{
			usleep(&coder->data->time_to_burnout);
			log_state(coder->data, coder->id, "burned out");
			return (NULL);
		}
	}
	*/
	return (NULL);
}

int	main(int argc, char **argv)
{
	t_c				*coders;
	t_d				*dongles;
	t_data			data;
	void			*ret;
	int				i;
	int 			last;

	if (argc != 9)
	{
		printf("Error arg");
		return (0);
	}
	printf("\n===Codexion===\n\n");
	if (atoi(argv[1]) <= 0)
	{
		printf("Error [1] number of coders is under 1");
		return (0);
	}
	data.number_of_coders = atoi(argv[1]);
	if (atoi(argv[2]) <= 0)
	{
		printf("Error [2] time to burnout is under 1");
		return (0);
	}
	data.time_to_burnout = atoi(argv[2]) * 1000;
	if (atoi(argv[3]) <= 0)
	{
		printf("Error [3] time to compile is under 1");
		return (0);
	}
	data.time_to_compile = atoi(argv[3]) * 1000;
	if (atoi(argv[4]) <= 0)
	{
		printf("Error [4] time to debug is under 1");
		return (0);
	}
	data.time_to_debug = atoi(argv[4]) * 1000;
	if (atoi(argv[5]) <= 0)
	{
		printf("Error [5] time to refactor is under 1");
		return (0);
	}
	data.time_to_refactor = atoi(argv[5]) * 1000;
	if (atoi(argv[6]) <= 0)
	{
		printf("Error [6] number of compiles required is UNDER 1");
		return (0);
	}
	else if (atoi(argv[6]) > data.number_of_coders)
	{
		printf("Error [6] number of compiles required is ABOVE the number of coders");
		return (0);
	}
	data.number_of_compiles_required = atoi(argv[6]);
	if (atoi(argv[7]) < 0)
	{
		printf("Error [7] dongle cooldown is under 0");
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
		printf("Error malloc scheduler");
		return (0);
	}
	ft_strlcpy(data.scheduler, argv[8]);
	// Program starts here ##############################
	coders = malloc(sizeof(t_c) * data.number_of_coders);
	dongles = malloc(sizeof(t_d) * data.number_of_coders);
	data.dongles = dongles;
	data.coders = coders;
	i = 0;
	data.start_time = current_time_ms();
	while (i < data.number_of_coders)
	{
		dongles[i].taken = 0;
		dongles[i].id = i + 1;
		i++;
	}
	i = 0;
	while (i < data.number_of_coders)
	{
		if (dongles[i].taken == 0)
		{
			dongles[i].taken = 1;
			coders[i].left = &dongles[i];
		}
		last = data.number_of_coders - (i + 1);
		if (dongles[last].taken == 0)
		{
			dongles[last].taken = 1;
			coders[i].right = &dongles[last];
		}
		i++;
	}
	i = 0;
	pthread_create(&data.c_thread, NULL, coder_chrono, &data);
	while (i < data.number_of_coders)
	{
		coders[i].last_compile_start = 0;
		coders[i].id = i + 1;
		coders[i].done = 0;
		pthread_create(&data.coders[i].thread, NULL, coder_routine, &data.coders[i]);
		i++;
	}
	while (i > 0)
	{
		pthread_join(coders[i].thread, &ret);
		i--;
	}
	printf("thread exited with '%p'\n", ret);
	free(data.scheduler);
	free(coders);
	free(dongles);
	return (0);
}
