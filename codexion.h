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

typedef struct data t_data;

typedef struct dongle
{
	int						id;
	int						taken;

	pthread_mutex_t			mutex;
}	t_d;

typedef struct coder
{
	int						id;
	int						done;

	long					last_compile_start;

	t_d						*left;
	t_d						*right;

	pthread_t				thread;

	struct data *data;
}	t_c;

typedef struct data
{
	long					start_time;

	int						number_of_coders;
	int						time_to_burnout;
	int						time_to_compile;
	int						time_to_debug;
	int						time_to_refactor;
	int						number_of_compiles_required;
	int						dongle_cooldown;

	char					*scheduler;

	t_c						*coders;
	t_d						*dongles;

	pthread_t				c_thread;

	pthread_mutex_t			log_mutex;
}	t_data;

void				*coder_routine(void *arg);
void				*coder_routine(void *arg);
void				log_state(t_data *data, int id, char *msg);

size_t				ft_strlcpy(char *dst, const char *src);
int					check_sched(char *s);
long				current_time_ms(void);

#endif
