/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chrono.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:52:03 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/14 17:52:05 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	activate_burn(t_data *data, int i)
{
	data->done = 1;
	log_forced(data, data->coders[i].id, "burned out");
	pthread_cond_broadcast(&data->cond_thread);
	return (1);
}

static int	check_burnout(t_data *data)
{
	int		i;
	long	now;

	i = 0;
	now = elapsed_ms(data);
	while (i < data->number_of_coders)
	{
		if (!data->coders[i].finished
			&& now - data->coders[i].last_compile_start
			>= data->time_to_burnout) // update here to > if not working
			return (activate_burn(data, i));
		i++;
	}
	return (0);
}

static int	all_finished(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		if (!data->coders[i].finished)
			return (0);
		i++;
	}
	return (1);
}

void	*coder_monitor(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&data->state_mutex);
		if (data->done || all_finished(data))
		{
			pthread_mutex_unlock(&data->state_mutex);
			break ;
		}
		if (check_burnout(data))
		{
			pthread_mutex_unlock(&data->state_mutex);
			return (NULL);
		}
		pthread_mutex_unlock(&data->state_mutex);
		usleep(500);
	}
	return (NULL);
}
