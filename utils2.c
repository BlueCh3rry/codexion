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

void	single_coder(t_c *coder)
{
	pthread_mutex_lock(&coder->left->mutex);
	log_state(coder->data, coder->id, "has taken a dongle");
	while (!sim_sleep(coder->data, 1))
		;
	pthread_mutex_unlock(&coder->left->mutex);
}

void	swap(t_request *x, t_request *y)
{
	t_request	tmp;

	tmp = *x;
	*x = *y;
	*y = tmp;
}

long	max(long a, long b)
{
	if (a > b)
		return (a);
	return (b);
}
