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

void	take_dongles(t_c *coder)
{
	if (coder->left->id < coder->right->id)
	{
		pthread_mutex_lock(&coder->left->mutex);
		pthread_mutex_lock(&coder->right->mutex);
	}
	else
	{
		pthread_mutex_lock(&coder->right->mutex);
		pthread_mutex_lock(&coder->left->mutex);
	}
	log_state(coder->data, coder->id, "has taken a dongle");
	log_state(coder->data, coder->id, "has taken a dongle");
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

int	check_all_compiled(t_data *d)
{
	int	i;

	i = 0;
	while (i < d->number_of_coders)
	{
		if (d->coders[i].completed_compiles
			< d->number_of_compiles_required)
			return (0);
		i++;
	}
	d->done = 1;
	pthread_cond_broadcast(&d->cond_thread);
	return (1);
}

void	compile(t_c *coder)
{
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = elapsed_ms(coder->data);
	coder->deadline = coder->last_compile_start + coder->data->time_to_burnout;
	pthread_mutex_unlock(&coder->data->state_mutex);
	log_state(coder->data, coder->id, "is compiling");
	sim_sleep(coder->data, coder->data->time_to_compile);
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->completed_compiles++;
	check_all_compiled(coder->data);
	pthread_mutex_unlock(&coder->data->state_mutex);
	release_dongles(coder);
}
