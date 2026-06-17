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

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef struct dongle
{
    pthread_mutex_t mutex;
}	t_d;

typedef struct coder
{
    int id;
    long last_compile_start;

	t_data      *data;

    t_d *left;
    t_d *right;

    pthread_t thread;
}	t_c;

typedef struct data
{
	long start_time;
	unsigned int number_of_coders;
	unsigned int time_to_burnout;
	unsigned int time_to_compile;
	unsigned int time_to_debug;
	unsigned int time_to_refactor;
	unsigned int number_of_compiles_required;
	unsigned int dongle_cooldown;
	char			*scheduler;

    t_c *coders;
    t_d *dongles;

    pthread_mutex_t log_mutex;
}	t_data;

size_t	ft_strlcpy(char *dst, const char *src)
{
	unsigned int	i;
	unsigned int	j;

	i = 0;
	j = strlen(src);
	while (src[i] != '\0')
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return (j);
}

int check_sched(char *s)
{
    return (!strcmp(s, "fifo") || !strcmp(s, "edf"));
}

long	current_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

void	log_state(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n",
		current_time_ms() - data->start_time,
		id,
		msg);
	pthread_mutex_unlock(&data->log_mutex);
}

void	*coder_routine(void *arg)
{
	t_c *coder;

    coder = (t_c *)arg;
	if (coder->left && coder->right)
	{
		pthread_mutex_lock(&coder->left->mutex);
		log_state(&coder->data->start_time, coder->id, "has taken a dongle");
		usleep(&coder->data->dongle_cooldown);
		pthread_mutex_lock(&coder->right->mutex);
		log_state(&coder->data->start_time, coder->id, "has taken a dongle");
	}
	log_state(&coder->data->start_time, coder->id, "is compiling");
	coder->last_compile_start = current_time_ms();
	usleep(&coder->data->time_to_compile);
	log_state(&coder->data->start_time, coder->id, "is debugging");
	usleep(&coder->data->time_to_debug);
	log_state(&coder->data->start_time, coder->id, "is refactoring");
	usleep(&coder->data->time_to_refactor);

	if (!strcmp(coder->data->scheduler, "edf"))
	{
		long deadline = coder->last_compile_start + coder->data->time_to_burnout;
		if (current_time_ms() - coder->last_compile_start == deadline)
		{
			usleep(&coder->data->time_to_burnout);
			log_state(&coder->data->start_time, coder->id, "burned out");
			exit(0);
		}
	}
	return (NULL);
}

int	main(int argc, char **argv)
{
	if (argc != 9)
	{
		printf("Error arg");
		return (0);
	}
	printf("\n===Codexion===\n\n");

	t_c				*coders;
	t_d				*dongles;
	t_data			data;
	void			*ret;
	int				i;
	long			start;

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
	i = 0;
	data.start_time = current_time_ms();
	while (i <= data.number_of_coders)
	{
		coders[i].id = i;
		coders[i].data = &data;
    	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}
	while (i >= 0)
	{
		pthread_join(&coders[i].thread, &ret);
		i--;
	}
	printf("thread exited with '%p'\n", ret);
	free(data.scheduler);
	free(coders);
	free(dongles);
	return (0);
}
