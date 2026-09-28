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



static void	release_dongles(t_c *coder)
{
	long	now;

	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_unlock(&coder->right->mutex);
	pthread_mutex_lock(&coder->data->state_mutex);
	now = elapsed_ms(coder->data);
	coder->left->available_at = now + coder->data->dongle_cooldown;
	coder->right->available_at = now + coder->data->dongle_cooldown;
	if (DEBUG == 1)
	{
		printf("REELEaSE-LL %ld\n", coder->right->available_at);
		printf("REELEaSE-RR %ld\n", coder->right->available_at);
	}
	coder->left->in_use = 0;
	coder->right->in_use = 0;
	pthread_cond_broadcast(&coder->data->cond_thread);
	pthread_mutex_unlock(&coder->data->state_mutex);
}

void	compile(t_c *coder)
{
	if (DEBUG == 1)
		printf("CcoooDDeerrr id %d \n", coder->id);
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = elapsed_ms(coder->data);
	coder->deadline = coder->last_compile_start + coder->data->time_to_burnout;
	pthread_mutex_unlock(&coder->data->state_mutex);
	log_state(coder->data, coder->id, "is compiling");
	sim_sleep(coder->data, coder->data->time_to_compile);
	release_dongles(coder);
}

/* a coder may compile only if it is at the top of both its dongle heaps */
static int	is_top(t_d *dongle, int id)
{
	return (dongle->heap.size > 0 && dongle->heap.requests[0].id == id);
}

int coder_can_compile(t_c *coder)
{
    long now;

    if (coder->left == coder->right)
        return (0);
    if (coder->left->in_use == 1 || coder->right->in_use == 1)
        return (1);
    now = elapsed_ms(coder->data);
	if (DEBUG == 1)
	{
		printf("NOW %ld\n", now);
		printf("NOW-LL %ld\n", coder->left->available_at);
		printf("NOW-RR %ld\n", coder->right->available_at);
	}
    if (now < coder->left->available_at || now < coder->right->available_at)
        return (1);          // not time yet -> keep waiting
    return (!(is_top(coder->left, coder->id) && is_top(coder->right, coder->id)));
    // ready & top of both heaps -> 0 -> stop waiting
}
