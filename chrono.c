#include "codexion.h"

static int	check_burnout(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		if (data->coders[i].last_compile_start != 0 &&
			current_time_ms() - data->coders[i].last_compile_start
				>= data->time_to_burnout / 1000)
		{
			log_state(data, data->coders[i].id, "burned out");
			data->done = 1;
			pthread_cond_broadcast(&data->cond_thread);
			return (1);
		}
		i++;
	}
	return (0);
}

void	*coder_chrono(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&data->state_mutex);
		if (data->done)
		{
			pthread_mutex_unlock(&data->state_mutex);
			break ;
		}
		if (check_burnout(data))
		{
			pthread_mutex_unlock(&data->state_mutex);
			return (NULL);
		}
		pthread_mutex_unlock(&data->state_mutex);
		usleep(1);
	}
	return (NULL);
}