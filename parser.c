/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 17:52:27 by mmakhmae          #+#    #+#             */
/*   Updated: 2026/09/14 17:52:28 by mmakhmae         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		data->coders[i].finished = 0;
		data->coders[i].deadline = data->time_to_burnout;
		data->coders[i].id = i + 1;
		data->coders[i].data = data;
		i++;
	}
}
