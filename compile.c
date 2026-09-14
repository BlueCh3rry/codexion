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

static void	wait_compile_done(t_c *coder)
{
	// usleep(coder->data->time_to_compile);
	while (current_time_ms() - coder->last_compile_start
			< coder->data->time_to_compile / 1000)
	{
		pthread_mutex_lock(&coder->data->state_mutex);
		if (coder->data->done)
		{
			pthread_mutex_unlock(&coder->data->state_mutex);
			exit(0);
		}
		pthread_mutex_unlock(&coder->data->state_mutex);
	}
}

static void	release_dongles(t_c *coder)
{
	long	cd;

	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = 0;
	cd = coder->data->dongle_cooldown;
	coder->left->available_at = current_time_ms() - coder->data->start_time + cd;
	coder->right->available_at = current_time_ms() - coder->data->start_time + cd;
	pthread_mutex_unlock(&coder->data->state_mutex);
	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_unlock(&coder->right->mutex);
}

void	compile(t_c *coder)
{
	take_dongles(coder);
	log_state(coder->data, coder->id, "is compiling");
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->last_compile_start = current_time_ms();
	pthread_mutex_unlock(&coder->data->state_mutex);
	wait_compile_done(coder);
	release_dongles(coder);
}

int	coder_can_compile(t_c *coder)
{
	long	now;

	now = current_time_ms() - coder->data->start_time;
	if (coder->left->available_at > 0 && now >= coder->left->available_at)
		return (0);
	if (coder->right->available_at > 0 && now >= coder->right->available_at)
		return (0);
	return (1);
}