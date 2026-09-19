/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/18 18:50:54 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/18 18:50:56 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	elapsed_ms(t_data *data)
{
	return (current_time_ms() - data->start_time);
}

/* [ADDED] Now it "Reject invalid inputs such as negative numbers,
** non-integers". atoi() silently accepted "12abc" and "" as 12 and 0.*/
int	ft_atoi_safe(const char *s, long *out)
{
	long	res;
	int		i;

	res = 0;
	i = 0;
	if (!s || !s[0])
		return (-1);
	while (s[i] != '\0')
	{
		if (s[i] < '0' || s[i] > '9')
			return (-1);
		res = (res * 10) + (s[i] - '0');
		if (res > 2147483647)
			return (-1);
		i++;
	}
	*out = res;
	return (0);
}

int	sim_sleep(t_data *data, long ms)
{
	long	end;

	end = current_time_ms() + ms;
	while (current_time_ms() < end)
	{
		pthread_mutex_lock(&data->state_mutex);
		if (data->done)
		{
			pthread_mutex_unlock(&data->state_mutex);
			return (1);
		}
		pthread_mutex_unlock(&data->state_mutex);
		usleep(100);
	}
	return (0);
}

void	wait_tick(t_data *data)
{
	struct timespec	ts;

	clock_gettime(current_time_ms(), &ts);
	ts.tv_nsec += 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&data->cond_thread, &data->state_mutex, &ts);
}

int	shares_dongle(t_c *a, t_c *b)
{
	return (a->left == b->left || a->left == b->right
		|| a->right == b->left || a->right == b->right);
}
