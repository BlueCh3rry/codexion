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

void    take_dongles(t_c *coder)
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
	if (DEBUG == 1)
		printf("CcoooDDeerrr id %d \n", coder->id);
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

static int  ready(t_c *c, long now)
{
    return (!c->left->in_use && !c->right->in_use
        && now >= c->left->available_at
        && now >= c->right->available_at);
}

/* blocked only by a higher-priority neighbour that can run RIGHT NOW */
static int  blocked_by(t_d *dongle, t_c *c, long now)
{
    t_c *other;

    if (dongle->heap.size == 0 || dongle->heap.requests[0].id == c->id)
        return (0);
    other = &c->data->coders[dongle->heap.requests[0].id - 1];
    return (ready(other, now));
}

int coder_can_compile(t_c *c)      /* 1 = must keep waiting */
{
    long    now;

    if (c->left == c->right)
        return (0);
    now = elapsed_ms(c->data);
    if (!ready(c, now))
        return (1);
    return (blocked_by(c->left, c, now) || blocked_by(c->right, c, now));
}

static int  started_is_min(t_data *d, t_c *c)
{
    int i;

    i = 0;
    while (i < d->number_of_coders)
    {
        if (d->coders[i].started < c->started)
            return (0);
        i++;
    }
    return (1);
}

void    wait_gate(t_c *c)
{
    while (!c->data->done && !started_is_min(c->data, c))
        pthread_cond_wait(&c->data->cond_thread, &c->data->state_mutex);
}


// static void count_compile(t_c *coder)
// {
//     t_data  *d;
//     int     i;
//     int     all;

//     d = coder->data;
//     pthread_mutex_lock(&d->state_mutex);
//     coder->completed_compiles++;
//     all = 1;
//     i = 0;
//     while (i < d->number_of_coders)
//     {
//         if (d->coders[i].completed_compiles
//             < d->number_of_compiles_required)
//             all = 0;
//         i++;
//     }
//     if (all)
//         d->done = 1;
//     pthread_cond_broadcast(&d->cond_thread);
//     pthread_mutex_unlock(&d->state_mutex);
// }