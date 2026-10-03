/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 17:24:58 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/10/03 17:24:59 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	ready(t_c *c, long now)
{
	return (!c->left->in_use && !c->right->in_use
		&& now >= c->left->available_at
		&& now >= c->right->available_at);
}

static int	blocked_by(t_d *dongle, t_c *c, long now)
{
	t_c	*other;

	if (dongle->heap.size == 0 || dongle->heap.requests[0].id == c->id)
		return (0);
	other = &c->data->coders[dongle->heap.requests[0].id - 1];
	return (ready(other, now));
}

int	coder_can_compile(t_c *c)
{
	long	now;

	if (c->left == c->right)
		return (0);
	now = elapsed_ms(c->data);
	if (!ready(c, now))
		return (1);
	return (blocked_by(c->left, c, now) || blocked_by(c->right, c, now));
}

static int	started_is_min(t_data *d, t_c *c)
{
	int	i;

	i = 0;
	while (i < d->number_of_coders)
	{
		if (d->coders[i].started < c->started)
			return (0);
		i++;
	}
	return (1);
}

void	wait_gate(t_c *c)
{
	while (!c->data->done && !started_is_min(c->data, c))
		pthread_cond_wait(&c->data->cond_thread, &c->data->state_mutex);
}
