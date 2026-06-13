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

pthread_mutex_t	g_mutex;

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

long current_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

void	*test_func(void *arg)
{
	pthread_mutex_lock(&g_mutex);
	printf("Hello from the new thread!\n");
	pthread_mutex_unlock(&g_mutex);
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
	int				nb_coders;
	long			start;

	number_of_coders = atoi(argv[1]);
	time_to_burnout = atoi(argv[2]);
	time_to_compile = atoi(argv[3]);
	time_to_debug = atoi(argv[4]);
	time_to_refactor = atoi(argv[5]);
	number_of_compiles_required = atoi(argv[6]);
	dongle_cooldown = atoi(argv[7]);
	nb_coders = atoi(argv[1]);
	if (check_sched(argv[8]))
	{
		printf("Error scheduler is neither fifo/edf");
		return (0);
	}
	scheduler = malloc(strlen(argv[8]));
	if (!scheduler)
	{
		printf("Error malloc scheduler");
		return (0);
	}
	ft_strlcpy(scheduler, argv[8]);

	start = current_time_ms();
	while (number_of_coders != 0)
	{
		usleep(100000);
		printf("%ld %d is compiling\n", current_time_ms() - start, nb_coders - (number_of_coders - 1));
		number_of_coders--;
	}
	pthread_create(&thid, NULL, test_func, NULL);
	pthread_join(thid, &ret);
	printf("thread exited with '%p'\n", ret);
	free(scheduler);
	return (0);
}
