/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/25 18:30:00 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/25 18:30:01 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wakeup_coders(t_data *data)
{
	pthread_mutex_lock(&data->state_mutex);
	pthread_cond_broadcast(&data->cond_thread);
	pthread_mutex_unlock(&data->state_mutex);
}

static int  burnout_handle(t_data *data, int i)
{
    pthread_mutex_lock(&data->state_mutex);
    if (elapsed_ms(data) - data->coders[i].last_compile_start
        < data->time_to_burnout)
    {
        pthread_mutex_unlock(&data->state_mutex);
        return (0);
    }
    data->done = 1;
    pthread_mutex_unlock(&data->state_mutex);
    log_forced(data, data->coders[i].id, "burned out");
    wakeup_coders(data);
    return (1);
}

static int	routine_primer(t_data *data, int *i, int *all_done)
{
	if (*all_done)
	{
		pthread_mutex_lock(&data->state_mutex);
		data->done = 1;
		pthread_mutex_unlock(&data->state_mutex);
		wakeup_coders(data);
		return (1);
	}
	*i = 0;
	*all_done = 1;
	usleep(100);
	return (0);
}

static int	read_coder(t_data *data, int i, long *last_compile)
{
	int	completed;

	pthread_mutex_lock(&data->state_mutex);
	completed = data->coders[i].completed_compiles;
	*last_compile = data->coders[i].last_compile_start;
	pthread_mutex_unlock(&data->state_mutex);
	return (completed);
}

void	*coder_monitor(void *arg)
{
	t_data	*data;
	int		all_done;
	int		i;
	long	last_compile;

	data = (t_data *)arg;
	i = 0;
	all_done = 1;
	while (1)
	{
		if (read_coder(data, i, &last_compile)
			< data->number_of_compiles_required)
		{
			all_done = 0;
			if (burnout_handle(data, i))
				return (NULL);
		}
		if (++i == data->number_of_coders
			&& routine_primer(data, &i, &all_done))
			return (NULL);
	}
}
