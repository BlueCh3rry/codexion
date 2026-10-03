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

static int	check_coder(t_data *d, int i, int *pending)
{
	pthread_mutex_lock(&d->state_mutex);
	if (d->coders[i].completed_compiles < d->number_of_compiles_required)
	{
		*pending = 1;
		if (elapsed_ms(d) - d->coders[i].last_compile_start
			>= d->time_to_burnout)
		{
			d->done = 1;
			pthread_cond_broadcast(&d->cond_thread);
			pthread_mutex_unlock(&d->state_mutex);
			log_forced(d, d->coders[i].id, "burned out");
			return (1);
		}
	}
	pthread_mutex_unlock(&d->state_mutex);
	return (0);
}

void	*coder_monitor(void *arg)
{
	t_data	*d;
	int		i;
	int		pending;

	d = (t_data *)arg;
	while (1)
	{
		i = 0;
		pending = 0;
		while (i < d->number_of_coders)
			if (check_coder(d, i++, &pending))
				return (NULL);
		if (!pending)
			return (NULL);
		usleep(100);
	}
}
