/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:52:15 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/14 17:52:16 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_turn(t_c *coder)
{
	if (!strcmp(coder->data->scheduler, "fifo"))
	{
		while (coder->id != coder->data->order && coder->data->done == 0)
			pthread_cond_wait(&coder->data->cond_thread,
				&coder->data->state_mutex);
	}
	else
	{
		while (coder_can_compile(coder) == 0 && coder->data->done == 0)
			pthread_cond_wait(&coder->data->cond_thread,
				&coder->data->state_mutex);
	}
}

static void	signal_next(t_c *coder)
{
	if (!strcmp(coder->data->scheduler, "edf"))
		pthread_cond_broadcast(&coder->data->cond_thread);
	else
		ft_signal(coder->data);
}

static void	debug_and_refactor(t_c *coder)
{
	log_state(coder->data, coder->id, "is debugging");
	usleep(coder->data->time_to_debug);
	log_state(coder->data, coder->id, "is refactoring");
	usleep(coder->data->time_to_refactor);
}

static int	check_done(t_c *coder)
{
	if (coder->data->done)
	{
		pthread_mutex_unlock(&coder->data->state_mutex);
		return (1);
	}
	return (0);
}

void	*coder_routine(void *arg)
{
	t_c	*coder;
	int	j;

	coder = (t_c *)arg;
	j = 0;
	while (j < coder->data->number_of_compiles_required)
	{
		pthread_mutex_lock(&coder->data->state_mutex);
		wait_turn(coder);
		if (check_done(coder))
			break ;
		pthread_mutex_unlock(&coder->data->state_mutex);
		compile(coder);
		pthread_mutex_lock(&coder->data->state_mutex);
		if (check_done(coder))
			break ;
		signal_next(coder);
		pthread_mutex_unlock(&coder->data->state_mutex);
		debug_and_refactor(coder);
		j++;
	}
	return (NULL);
}
