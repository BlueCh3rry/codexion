/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   compile.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:52:52 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/14 17:52:53 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	take_dongles(t_c *coder)
{
	if (coder->left->id < coder->right->id)
	{
		pthread_mutex_lock(&coder->left->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
		pthread_mutex_lock(&coder->right->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
	}
	else
	{
		pthread_mutex_lock(&coder->right->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
		pthread_mutex_lock(&coder->left->mutex);
		log_state(coder->data, coder->id, "has taken a dongle");
	}
}

static void	release_dongles(t_c *coder)
{
	long	now;

	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_unlock(&coder->right->mutex);
	pthread_mutex_lock(&coder->data->state_mutex);
	now = elapsed_ms(coder->data);
	coder->left->available_at = now + coder->data->dongle_cooldown;
	coder->right->available_at = now + coder->data->dongle_cooldown;
	coder->left->in_use = 0;
	coder->right->in_use = 0;
	pthread_cond_broadcast(&coder->data->cond_thread);
	pthread_mutex_unlock(&coder->data->state_mutex);
}

void	compile(t_c *coder)
{
	take_dongles(coder);
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = elapsed_ms(coder->data);
	coder->deadline = coder->last_compile_start + coder->data->time_to_burnout;
	pthread_mutex_unlock(&coder->data->state_mutex);
	log_state(coder->data, coder->id, "is compiling");
	sim_sleep(coder->data, coder->data->time_to_compile);
	release_dongles(coder);
}

static int	shares_dongle(t_c *a, t_c *b)
{
	return (a->left == b->left || a->left == b->right
		|| a->right == b->left || a->right == b->right);
}

static int	blocked_by_higher(t_c *coder)
{
	t_h		*heap;
	int		me;
	int		i;

	heap = &coder->data->heap;
	me = heap_find(heap, coder->id);
	if (me < 0)
		return (1);
	i = 0;
	while (i < heap->size)
	{
		if (i != me && hn_before(&heap->array[i], &heap->array[me])
			&& shares_dongle(coder,
				&coder->data->coders[heap->array[i].id - 1]))
			return (1);
		i++;
	}
	return (0);
}

int	coder_can_compile(t_c *coder)
{
	long	now;

	if (coder->left == coder->right)
		return (0);
	if (coder->left->in_use || coder->right->in_use)
		return (0);
	now = elapsed_ms(coder->data);
	if (now < coder->left->available_at || now < coder->right->available_at)
		return (0);
	return (!blocked_by_higher(coder));
}
