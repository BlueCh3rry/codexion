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
	long	metric;

	d = coder->data;
	if (d->edf)
		metric = coder->deadline * (d->number_of_coders + 1) + coder->id;
	else
		metric = -(d->next_ticket++);
	pre_register_heaps(coder->left, coder->right, coder, metric);
	fprintf(stderr, "[REQ]   t=%ld coder %d metric %ld\n", elapsed_ms(d), coder->id, metric);
	pthread_cond_broadcast(&d->cond_thread);
	if (coder->started == 0)
    {
        d->first_reqs++;
        pthread_cond_broadcast(&d->cond_thread);
        while (!d->done && d->first_reqs < d->number_of_coders)
            pthread_cond_wait(&d->cond_thread, &d->state_mutex);
    }
}

static void	wait_for_turn(t_c *coder)
{
	t_data	*d;
	long	delta;

	d = coder->data;
	while (!d->done && coder_can_compile(coder) == 1)
	{
		delta = max(coder->left->available_at, coder->right->available_at)
			- elapsed_ms(d);
		if (delta > 0)
		{
			pthread_mutex_unlock(&d->state_mutex);
			usleep(delta);
			pthread_mutex_lock(&d->state_mutex);
		}
		else
			pthread_cond_wait(&d->cond_thread, &d->state_mutex);
	}
}

static int	acquire_turn(t_c *coder)
{
	t_data	*d;

	d = coder->data;
	pthread_mutex_lock(&d->state_mutex);
	wait_gate(coder);
	if (!d->done)
		request_slot(coder);
	if (!d->done)
		wait_for_turn(coder);
	if (d->done)
		return (pthread_mutex_unlock(&d->state_mutex), 0);
	fprintf(stderr, "[GRANT] t=%ld coder %d (top left=%d, top right=%d)\n", elapsed_ms(d), coder->id, coder->left->heap.requests[0].id, coder->right->heap.requests[0].id);
	remove_request_id(&coder->left->heap, coder->id);
	remove_request_id(&coder->right->heap, coder->id);
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	coder->started++;
	coder->last_compile_start = elapsed_ms(d);
	coder->deadline = coder->last_compile_start + d->time_to_burnout;
	pthread_cond_broadcast(&d->cond_thread);
	pthread_mutex_unlock(&d->state_mutex);
	take_dongles(coder);
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
