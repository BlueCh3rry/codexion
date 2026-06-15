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
    int compile_count;

    t_d *left;
    t_d *right;

    pthread_t thread;
}	t_c;

typedef struct data
{
    int stop;

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

int	check_sched(char *s)
{
	if (strcmp(s, "fifo") || strcmp(s, "edf"))
		return (0);
	return (1);
}

long	current_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

void	show_log(long s, int nb, char *m)
{
	printf("%ld %d %s\n", current_time_ms() - s, nb, m);
}

void	*test_func(void *arg)
{
	pthread_mutex_lock(&g_mutex);
	printf("Hello from the new thread!\n");
	pthread_mutex_unlock(&g_mutex);
	return (NULL);
}

int coder_routine()
{
	int count_d;

	count_d = 0;
	while (count_d != 2 && dongles != 0)
	{
		show_log(start, coder, "has taken a dongle");
		dongles--;
		count_d++;
		if (count_d < 2)
			usleep(dongle_cooldown);
	}
	if (count_d > 0)
	{
		show_log(start, coder, "is compiling");
		usleep(time_to_compile);
		dongles += count_d;
		show_log(start, coder, "is debugging");
		usleep(time_to_debug);
		show_log(start, coder, "is refactoring");
		usleep(time_to_refactor);
	}
	if (coder == number_of_compiles_required || count_d == 0)
	{
		usleep(time_to_burnout);
		show_log(start, coder, "burned out");
		return(1);
	}
	return (0);
}

int	main(int argc, char **argv)
{
	if (argc != 9)
	{
		printf("Error arg");
		return (0);
	}
	printf("\n===Codexion===\n\n");
	unsigned int	number_of_coders;
	unsigned int	time_to_burnout;
	unsigned int	time_to_compile;
	unsigned int	time_to_debug;
	unsigned int	time_to_refactor;
	unsigned int	number_of_compiles_required;
	unsigned int	dongle_cooldown;
	char			*scheduler;
	pthread_t		thid;
	void			*ret;
	int				dongles;
	int				coder;
	int				count_d;
	long			start;

	if (atoi(argv[1]) <= 0)
	{
		printf("Error [1] number of coders is under 1");
		return (0);
	}
	number_of_coders = atoi(argv[1]);
	if (atoi(argv[2]) <= 0)
	{
		printf("Error [2] time to burnout is under 1");
		return (0);
	}
	time_to_burnout = atoi(argv[2]) * 1000;
	if (atoi(argv[3]) <= 0)
	{
		printf("Error [3] time to compile is under 1");
		return (0);
	}
	time_to_compile = atoi(argv[3]) * 1000;
	if (atoi(argv[4]) <= 0)
	{
		printf("Error [4] time to debug is under 1");
		return (0);
	}
	time_to_debug = atoi(argv[4]) * 1000;
	if (atoi(argv[5]) <= 0)
	{
		printf("Error [5] time to refactor is under 1");
		return (0);
	}
	time_to_refactor = atoi(argv[5]) * 1000;
	if (atoi(argv[6]) <= 0)
	{
		printf("Error [6] number of compiles required is UNDER 1");
		return (0);
	}
	else if (atoi(argv[6]) > number_of_coders)
	{
		printf("Error [6] number of compiles required is ABOVE the number of coders");
		return (0);
	}
	number_of_compiles_required = atoi(argv[6]);
	if (atoi(argv[7]) < 0)
	{
		printf("Error [7] dongle cooldown is under 0");
		return (0);
	}
	dongle_cooldown = atoi(argv[7]) * 1000;
	if (check_sched(argv[8]))
	{
		printf("Error [8] scheduler is neither fifo/edf");
		return (0);
	}
	scheduler = malloc(strlen(argv[8]));
	if (!scheduler)
	{
		printf("Error malloc scheduler");
		return (0);
	}
	ft_strlcpy(scheduler, argv[8]);

	// Program starts here ##############################

	thid = malloc(sizeof(pthread_t) * number_of_coders);
	int i = 0;
	coder = 1;
	dongles = number_of_coders;
	start = current_time_ms();
	while (i <= number_of_coders)
	{
    	pthread_create(&thid[i], NULL, coder_routine, &coders[i]);
		i++;
	}
	while (i >= 0)
	{
		pthread_join(thid, &ret);
		i--;
	}
	printf("thread exited with '%p'\n", ret);
	free(scheduler);
	free(thid);
	free(coders);
	return (0);
}
