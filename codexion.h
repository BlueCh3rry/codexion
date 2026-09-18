/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 15:45:34 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/06/21 15:45:37 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

typedef struct heap_node
{
	int						id;
	long					key;
	long					tie;
}	t_hn;

typedef struct heap
{
	t_hn					*array;
	int						size;
	int						capacity;
}	t_h;

typedef struct dongle
{
	int						id;

	long					available_at;

	int						in_use;

	pthread_mutex_t			mutex;
}	t_d;

typedef struct coder
{
	int						id;

	int						queued;
	int						finished;

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

	t_h						heap;
	t_c						*coders;
	t_d						*dongles;

	pthread_t				c_thread;

	pthread_cond_t			cond_thread;

	pthread_mutex_t			state_mutex;
	pthread_mutex_t			log_mutex;
}	t_data;

/* heap.c */
int					heap_init(t_h *heap, int capacity);
void				heap_free(t_h *heap);
void				heap_push(t_h *heap, int id, long key, long tie);
void				heap_remove_id(t_h *heap, int id);
int					heap_find(t_h *heap, int id);
int					heap_peek_id(t_h *heap);
int					hn_before(t_hn *a, t_hn *b);

/* routine / monitor / compile */
void				*coder_routine(void *arg);
void				*coder_monitor(void *arg);
void				compile(t_c *coder);
int					coder_can_compile(t_c *coder);

/* init / parsing */
int					parse_args1(char **argv, t_data *data);
int					parse_args2(char **argv, t_data *data);
int					init_scheduler(t_data *data, char *sched);
void				init_dongles(t_data *data);
void				init_coders(t_data *data);

/* utils */
void				log_state(t_data *data, int id, char *msg);
void				log_forced(t_data *data, int id, char *msg);
size_t				ft_strcpy(char *dst, const char *src);
int					check_sched(char *s);
int					ft_atoi_safe(const char *s, long *out);
long				current_time_ms(void);
long				elapsed_ms(t_data *data);
int					sim_sleep(t_data *data, long ms);
void				wait_tick(t_data *data);

#endif