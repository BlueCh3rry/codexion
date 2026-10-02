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

long	elapsed_ms(t_data *data)
{
	return (current_time_ms() - data->start_time);
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
	if (!data->done)
		printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}

void	log_forced(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}
