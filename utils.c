/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 15:45:05 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/06/21 15:45:09 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

size_t	ft_strcpy(char *dst, const char *src)
{
	unsigned int	i;
	unsigned int	j;

	i = 0;
	j = strlen(src);
	while (src[i] != '\0')
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return (j);
}

int	check_sched(char *s)
{
	return (!strcmp(s, "fifo") || !strcmp(s, "edf"));
}

/* [ADDED] Now it "Reject invalid inputs such as negative numbers,
** non-integers". atoi() silently accepted "12abc" and "" as 12 and 0, so it
** cannot be used for validation. Returns 0 on success, -1 on any garbage,
** empty string, sign, or overflow above INT_MAX.                           */
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

long	current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

/* [ADDED] every timestamp in the program is now "ms since start_time",
** so nothing mixes absolute ms with relative ms any more (that mix was the
** reason available_at / last_compile_start comparisons were nonsense).      */
long	elapsed_ms(t_data *data)
{
	return (current_time_ms() - data->start_time);
}

/* [FIX] log_state used to print even after a burnout. It now drops late
** messages. The read of data->done is done without the state mutex on
** purpose: log_state is also called from paths that must not take it, and a
** 1-message race here is harmless.                                          */
void	log_state(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	if (!data->done)
		printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}

/* [ADDED] used only for the burnout line, which must always be printed. */
void	log_forced(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}

/* [FIX] replaces wait_compile_done(): that function busy-spun at 100% CPU
** and called exit(0) from inside a worker thread while holding mutexes.
** Returns 1 if the simulation ended during the sleep.                       */
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
		usleep(200);
	}
	return (0);
}

/* [ADDED] timed wait (1 ms). A plain pthread_cond_wait would sleep forever
** when the only thing we are waiting for is a dongle *cooldown* expiring,
** because nobody signals when time passes.                                  */
void	wait_tick(t_data *data)
{
	struct timespec	ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&data->cond_thread, &data->state_mutex, &ts);
}
