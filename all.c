#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <pthread.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>

typedef struct data	t_data;

typedef struct request
{
	int						id;
	long					metric;
}	t_request;

typedef struct heap
{
	t_request				requests[2];
	int						size;
}	t_heap;

typedef struct dongle
{
	int						id;

	long					available_at;

	int						in_use;

	t_heap					heap;

	pthread_mutex_t			mutex;
}	t_d;

typedef struct coder
{
	int						id;

	int						queued;
	int						completed_compiles;

	long					request_time;
	long					last_compile_start;
	long					deadline;

	t_d						*left;
	t_d						*right;

	pthread_t				thread;

	struct data				*data;
}	t_c;

typedef struct data
{
	long					start_time;

	int						done;
	int						number_of_coders;
	int						number_of_compiles_required;
	int						edf;

	long					time_to_burnout;
	long					time_to_compile;
	long					time_to_debug;
	long					time_to_refactor;
	long					dongle_cooldown;
	long					next_ticket;

	char					*scheduler;

	t_c						*coders;
	t_d						*dongles;

	pthread_t				c_thread;

	pthread_cond_t			cond_thread;

	pthread_mutex_t			state_mutex;
	pthread_mutex_t			log_mutex;
}	t_data;

void				*coder_routine(void *arg);
void				*coder_monitor(void *arg);
void				compile(t_c *coder);
int					coder_can_compile(t_c *coder);

void				add_request(t_heap *heap, int id, long metric);
void				remove_request_top(t_heap *heap);
void				pre_register_heaps(t_d *first, t_d *second,
						t_c *coder, long metric);

int					parse_args1(char **argv, t_data *data);
int					parse_args2(char **argv, t_data *data);
int					init_scheduler(t_data *data, char *sched);
void				init_dongles(t_data *data);
void				init_coders(t_data *data);

void				log_state(t_data *data, int id, char *msg);
void				log_forced(t_data *data, int id, char *msg);
size_t				ft_strcpy(char *dst, const char *src);
int					check_sched(char *s);
int					ft_atoi_safe(const char *s, long *out);
long				current_time_ms(void);
long				elapsed_ms(t_data *data);
int					sim_sleep(t_data *data, long ms);
void				wait_tick(t_data *data);
void				swap(t_request *x, t_request *y);
void				wakeup_coders(t_data *data);

#endif


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

long	current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

void	log_state(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	if (!data->done)
		printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}

void	log_forced(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n", elapsed_ms(data), id, msg);
	pthread_mutex_unlock(&data->log_mutex);
}


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
	struct timeval	tv;
	struct timespec	ts;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&data->cond_thread, &data->state_mutex, &ts);
}

void	swap(t_request *x, t_request *y)
{
	t_request	tmp;

	tmp = *x;
	*x = *y;
	*y = tmp;
}

#include "codexion.h"

/* metric = FIFO ticket, or EDF deadline. For EDF the coder id goes in the
** low part so two metrics are never equal (no tie can block both coders). */
static void	request_slot(t_c *coder)
{
	t_data	*d;
	long	metric;

	d = coder->data;
	if (d->edf)
		metric = coder->deadline * (d->number_of_coders + 1) + coder->id;
	else
		metric = d->next_ticket++;
	coder->request_time = elapsed_ms(d);
	pre_register_heaps(coder->left, coder->right, coder, metric);
	coder->queued = 1;
	pthread_cond_broadcast(&d->cond_thread);
}

static int	acquire_turn(t_c *coder)
{
	t_data	*d;

	d = coder->data;
	pthread_mutex_lock(&d->state_mutex);
	request_slot(coder);
	while (!d->done && !coder_can_compile(coder))
		wait_tick(d);
	coder->queued = 0;
	if (d->done)
	{
		pthread_mutex_unlock(&d->state_mutex);
		return (0);
	}
	remove_request_top(&coder->left->heap);
	remove_request_top(&coder->right->heap);
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	coder->request_time = 0;
	pthread_mutex_unlock(&d->state_mutex);
	return (1);
}

/* a compile cycle counts as completed once debug + refactor are done */
static int	debug_and_refactor(t_c *coder)
{
	log_state(coder->data, coder->id, "is debugging");
	if (sim_sleep(coder->data, coder->data->time_to_debug))
		return (1);
	log_state(coder->data, coder->id, "is refactoring");
	if (sim_sleep(coder->data, coder->data->time_to_refactor))
		return (1);
	pthread_mutex_lock(&coder->data->state_mutex);
	coder->completed_compiles++;
	pthread_mutex_unlock(&coder->data->state_mutex);
	return (0);
}

static void	single_coder(t_c *coder)
{
	pthread_mutex_lock(&coder->left->mutex);
	log_state(coder->data, coder->id, "has taken a dongle");
	while (!sim_sleep(coder->data, 1))
		;
	pthread_mutex_unlock(&coder->left->mutex);
}

void	*coder_routine(void *arg)
{
	t_c	*coder;
	int	j;

	coder = (t_c *)arg;
	if (coder->left == coder->right)
		return (single_coder(coder), NULL);
	j = 0;
	while (j < coder->data->number_of_compiles_required)
	{
		if (!acquire_turn(coder))
			return (NULL);
		compile(coder);
		if (debug_and_refactor(coder))
			return (NULL);
		j++;
	}
	return (NULL);
}


#include "codexion.h"

static void	bubble_up(t_request *requests, int i)
{
	int	parent_idx;

	while (i > 0)
	{
		parent_idx = (i - 1) / 2;
		if (requests[i].metric < requests[parent_idx].metric)
			swap(&requests[parent_idx], &requests[i]);
		else
			break ;
		i = parent_idx;
	}
}

static void	heapify(t_request *requests, int parent, int size)
{
	int	left_child_idx;
	int	right_child_idx;
	int	least;

	least = parent;
	while (1)
	{
		left_child_idx = (parent * 2) + 1;
		right_child_idx = (parent * 2) + 2;
		if (left_child_idx < size && requests[left_child_idx].metric
			< requests[parent].metric)
			least = left_child_idx;
		if (right_child_idx < size && requests[right_child_idx].metric
			< requests[least].metric)
			least = right_child_idx;
		if (least == parent)
			break ;
		swap(&requests[least], &requests[parent]);
		parent = least;
	}
}

void	add_request(t_heap *heap, int id, long metric)
{
	if (!heap || (heap->size == 2))
		return ;
	heap->requests[heap->size].id = id;
	heap->requests[heap->size].metric = metric;
	bubble_up(heap->requests, heap->size);
	heap->size += 1;
}

void	remove_request_top(t_heap *heap)
{
	if (!heap || !heap->size)
		return ;
	if (&heap->requests[0] != &heap->requests[heap->size - 1])
		swap(&heap->requests[0], &heap->requests[heap->size - 1]);
	heap->size -= 1;
	heapify(heap->requests, 0, heap->size);
	return ;
}

/* heaps are protected by state_mutex (held by the caller), not by the
** dongle mutex, because the dongle mutex is held during a whole compile. */
void	pre_register_heaps(t_d *first, t_d *second,
	t_c *coder, long metric)
{
	add_request(&first->heap, coder->id, metric);
	add_request(&second->heap, coder->id, metric);
}

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

/* a coder may compile only if it is at the top of both its dongle heaps */
static int	is_top(t_d *dongle, int id)
{
	return (dongle->heap.size > 0 && dongle->heap.requests[0].id == id);
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
	return (is_top(coder->left, coder->id)
		&& is_top(coder->right, coder->id));
}


#include "codexion.h"

int	parse_args1(char **argv, t_data *data)
{
	long	v;

	if (ft_atoi_safe(argv[1], &v) == -1 || v <= 0)
		return (printf("Error [1] invalid number of coders\n"), -1);
	data->number_of_coders = (int)v;
	if (ft_atoi_safe(argv[2], &v) == -1 || v <= 0)
		return (printf("Error [2] invalid time to burnout\n"), -1);
	data->time_to_burnout = v;
	if (ft_atoi_safe(argv[3], &v) == -1)
		return (printf("Error [3] invalid time to compile\n"), -1);
	data->time_to_compile = v;
	if (ft_atoi_safe(argv[4], &v) == -1)
		return (printf("Error [4] invalid time to debug\n"), -1);
	data->time_to_debug = v;
	return (0);
}

int	parse_args2(char **argv, t_data *data)
{
	long	v;

	if (ft_atoi_safe(argv[5], &v) == -1)
		return (printf("Error [5] invalid time to refactor\n"), -1);
	data->time_to_refactor = v;
	if (ft_atoi_safe(argv[6], &v) == -1 || v < 1)
		return (printf("Error [6] invalid number of compiles required\n"), -1);
	data->number_of_compiles_required = (int)v;
	if (ft_atoi_safe(argv[7], &v) == -1)
		return (printf("Error [7] invalid dongle cooldown\n"), -1);
	data->dongle_cooldown = v;
	if (!check_sched(argv[8]))
		return (printf("Error [8] scheduler is neither fifo nor edf\n"), -1);
	return (0);
}

int	init_scheduler(t_data *data, char *sched)
{
	data->scheduler = malloc(strlen(sched) + 1);
	if (!data->scheduler)
	{
		printf("Error malloc scheduler\n");
		return (-1);
	}
	ft_strcpy(data->scheduler, sched);
	data->edf = !strcmp(sched, "edf");
	return (0);
}

void	init_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		data->dongles[i].id = i + 1;
		data->dongles[i].available_at = 0;
		data->dongles[i].in_use = 0;
		data->dongles[i].heap.size = 0;
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
		i++;
	}
}

void	init_coders(t_data *data)
{
	int	i;
	int	nbr;

	i = 0;
	while (i < data->number_of_coders)
	{
		data->coders[i].left = &data->dongles[i];
		nbr = (i + 1) % data->number_of_coders;
		data->coders[i].right = &data->dongles[nbr];
		data->coders[i].last_compile_start = 0;
		data->coders[i].request_time = 0;
		data->coders[i].queued = 0;
		data->coders[i].completed_compiles = 0;
		data->coders[i].deadline = data->time_to_burnout;
		data->coders[i].id = i + 1;
		data->coders[i].data = data;
		i++;
	}
}

#include "codexion.h"

void	wakeup_coders(t_data *data)
{
	pthread_mutex_lock(&data->state_mutex);
	pthread_cond_broadcast(&data->cond_thread);
	pthread_mutex_unlock(&data->state_mutex);
}

/* burns out when now - last_compile >= burnout (use > if it fires early).
** last_compile is 0 before the first compile, so it counts from the start. */
static int	burnout_handle(t_data *data, int i, long last_compile)
{
	if (elapsed_ms(data) - last_compile < data->time_to_burnout)
		return (0);
	pthread_mutex_lock(&data->state_mutex);
	data->done = 1;
	pthread_mutex_unlock(&data->state_mutex);
	log_forced(data, data->coders[i].id, "burned out");
	wakeup_coders(data);
	return (1);
}

static int	routine_primer(t_data *data, int *i, int *all_done)
{
	if (*all_done)
	{
		pthread_mutex_lock(&data->state_mutex);
		data->done = 1;
		pthread_mutex_unlock(&data->state_mutex);
		wakeup_coders(data);
		return (1);
	}
	*i = 0;
	*all_done = 1;
	usleep(100);
	return (0);
}

static int	read_coder(t_data *data, int i, long *last_compile)
{
	int	completed;

	pthread_mutex_lock(&data->state_mutex);
	completed = data->coders[i].completed_compiles;
	*last_compile = data->coders[i].last_compile_start;
	pthread_mutex_unlock(&data->state_mutex);
	return (completed);
}

void	*coder_monitor(void *arg)
{
	t_data	*data;
	int		all_done;
	int		i;
	long	last_compile;

	data = (t_data *)arg;
	i = 0;
	all_done = 1;
	while (1)
	{
		if (read_coder(data, i, &last_compile)
			< data->number_of_compiles_required)
		{
			all_done = 0;
			if (burnout_handle(data, i, last_compile))
				return (NULL);
		}
		if (++i == data->number_of_coders
			&& routine_primer(data, &i, &all_done))
			return (NULL);
	}
}


#include "codexion.h"

static int	alloc_structures(t_data *data)
{
	data->coders = malloc(sizeof(t_c) * data->number_of_coders);
	if (!data->coders)
	{
		printf("Error malloc coders\n");
		return (-1);
	}
	data->dongles = malloc(sizeof(t_d) * data->number_of_coders);
	if (!data->dongles)
	{
		printf("Error malloc dongles\n");
		free(data->coders);
		return (-1);
	}
	return (0);
}

static void	start_threads(t_data *data)
{
	int	i;

	pthread_create(&data->c_thread, NULL, coder_monitor, data);
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_create(
			&data->coders[i].thread,
			NULL,
			coder_routine,
			&data->coders[i]);
		i++;
	}
}

static void	join_threads(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
}

static void	cleanup(t_data *data)
{
	int	i;

	pthread_mutex_lock(&data->state_mutex);
	data->done = 1;
	pthread_cond_broadcast(&data->cond_thread);
	pthread_mutex_unlock(&data->state_mutex);
	pthread_join(data->c_thread, NULL);
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_mutex_destroy(&data->dongles[i].mutex);
		i++;
	}
	pthread_cond_destroy(&data->cond_thread);
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->state_mutex);
	printf("END\n");
	free(data->scheduler);
	free(data->coders);
	free(data->dongles);
}

int	main(int argc, char **argv)
{
	t_data	data;

	if (argc != 9)
		return (printf("Error arg\n"), 0);
	memset(&data, 0, sizeof(t_data));
	if (parse_args1(argv, &data) == -1 || parse_args2(argv, &data) == -1)
		return (0);
	if (init_scheduler(&data, argv[8]) == -1)
		return (0);
	if (alloc_structures(&data) == -1)
		return (free(data.scheduler), 0);
	data.done = 0;
	data.next_ticket = 1;
	pthread_cond_init(&data.cond_thread, NULL);
	pthread_mutex_init(&data.log_mutex, NULL);
	pthread_mutex_init(&data.state_mutex, NULL);
	data.start_time = current_time_ms();
	init_dongles(&data);
	init_coders(&data);
	start_threads(&data);
	join_threads(&data);
	cleanup(&data);
	return (0);
}