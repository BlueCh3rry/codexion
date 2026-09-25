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

/* metric = FIFO ticket, or EDF deadline. For EDF the coder id goes in the
** low part so two metrics are never equal (no tie can block both coders). */
static void	request_slot(t_c *coder)
{
	t_data	*d;
	long	metric;

	d = coder->data;
	if (d->edf)
		metric = coder->deadline * (d->number_of_coders + 1) + coder->id;
	else
		metric = d->next_ticket++;
	coder->request_time = elapsed_ms(d);
	pre_register_heaps(coder->left, coder->right, coder, metric);
	coder->queued = 1;
	pthread_cond_broadcast(&d->cond_thread);
}

static int	acquire_turn(t_c *coder)
{
	t_data	*d;
	struct timespec ts;

	d = coder->data;
	pthread_mutex_lock(&d->state_mutex);
	request_slot(coder);
	while (!d->done && !coder_can_compile(coder))
	{
		wait_tick(&ts);
        pthread_cond_timedwait(&d->cond_thread, &d->state_mutex, &ts);
	}
	coder->queued = 0;
	if (d->done)
	{
		pthread_mutex_unlock(&d->state_mutex);
		return (0);
	}
	remove_request_top(&coder->left->heap);
	remove_request_top(&coder->right->heap);
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	coder->request_time = 0;
	pthread_mutex_unlock(&d->state_mutex);
	return (1);
}

/* a compile cycle counts as completed once debug + refactor are done */
static int	debug_and_refactor(t_c *coder)
{
	log_state(coder->data, coder->id, "is debugging");
	if (sim_sleep(coder->data, coder->data->time_to_debug))
		return (1);
	log_state(coder->data, coder->id, "is refactoring");
	if (sim_sleep(coder->data, coder->data->time_to_refactor))
		return (1);
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->completed_compiles++;
	pthread_mutex_unlock(&coder->data->state_mutex);
	return (0);
}

static void	single_coder(t_c *coder)
{
	pthread_mutex_lock(&coder->left->mutex);
	log_state(coder->data, coder->id, "has taken a dongle");
	while (!sim_sleep(coder->data, 1))
		;
	pthread_mutex_unlock(&coder->left->mutex);
}

void	*coder_routine(void *arg)
{
	t_c	*coder;
	int	j;

	coder = (t_c *)arg;
	if (coder->left == coder->right)
		return (single_coder(coder), NULL);
	j = 0;
	while (j < coder->data->number_of_compiles_required)
	{
		if (!acquire_turn(coder))
			return (NULL);
		compile(coder);
		if (debug_and_refactor(coder))
			return (NULL);
		j++;
	}
	return (NULL);
}
