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

static void	request_slot(t_c *coder)
{
	t_data	*d;
	long	ticket;

	d = coder->data;
	ticket = d->next_ticket++;
	coder->request_time = elapsed_ms(d);
	if (d->edf)
		heap_push(&d->heap, coder->id, coder->deadline, ticket);
	else
		heap_push(&d->heap, coder->id, ticket, ticket);
	coder->queued = 1;
	pthread_cond_broadcast(&d->cond_thread);
}

static int	acquire_turn(t_c *coder)
{
	t_data	*d;

	d = coder->data;
	pthread_mutex_lock(&d->state_mutex);
	request_slot(coder);
	while (!d->done && !coder_can_compile(coder))
		wait_tick(d);
	heap_remove_id(&d->heap, coder->id);
	coder->queued = 0;
	if (d->done)
	{
		pthread_mutex_unlock(&d->state_mutex);
		return (0);
	}
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	coder->request_time = 0;
	pthread_mutex_unlock(&d->state_mutex);
	return (1);
}

static int	debug_and_refactor(t_c *coder)
{
	log_state(coder->data, coder->id, "is debugging");
	if (sim_sleep(coder->data, coder->data->time_to_debug))
		return (1);
	log_state(coder->data, coder->id, "is refactoring");
	if (sim_sleep(coder->data, coder->data->time_to_refactor))
		return (1);
	return (0);
}

static void	mark_finished(t_c *coder)
{
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->finished = 1;
	pthread_cond_broadcast(&coder->data->cond_thread);
	pthread_mutex_unlock(&coder->data->state_mutex);
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
	mark_finished(coder);
	return (NULL);
}
