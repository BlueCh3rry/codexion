/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 15:45:05 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/06/21 15:45:09 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

size_t	ft_strcpy(char *dst, const char *src)
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
	return (!strcmp(s, "fifo") || !strcmp(s, "edf"));
}

long	current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

void	log_state(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n", current_time_ms() - data->start_time, id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}

void	ft_signal(t_data *data)
{
	data->order++;
	if (data->order == data->number_of_coders + 1)
		data->order = 1;
	pthread_cond_broadcast(&data->cond_thread);
}
