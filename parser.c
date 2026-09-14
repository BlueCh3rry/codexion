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
	if (atoi(argv[1]) <= 1)
		return (printf("Error [1] number of coders is too low\n"), -1);
	data->number_of_coders = atoi(argv[1]);
	if (atoi(argv[2]) <= 1)
		return (printf("Error [2] time to burnout is too low\n"), -1);
	data->time_to_burnout = atoi(argv[2]) * 1000;
	if (atoi(argv[3]) <= 1)
		return (printf("Error [3] time to compile is too low\n"), -1);
	data->time_to_compile = atoi(argv[3]) * 1000;
	if (atoi(argv[4]) <= 1)
		return (printf("Error [4] time to debug is too low\n"), -1);
	data->time_to_debug = atoi(argv[4]) * 1000;
	return (0);
}

int	parse_args2(char **argv, t_data *data)
{
	if (atoi(argv[5]) <= 1)
		return (printf("Error [5] time to refactor is too low\n"), -1);
	data->time_to_refactor = atoi(argv[5]) * 1000;
	if (atoi(argv[6]) <= 0)
		return (printf("Error [6] number of compiles required low\n"), -1);
	data->number_of_compiles_required = atoi(argv[6]);
	if (atoi(argv[7]) <= 1)
		return (printf("Error [7] dongle cooldown is under 0\n"), -1);
	data->dongle_cooldown = atoi(argv[7]) * 1000;
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
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
		i++;
	}
}

void	init_coders(t_data *data)
{
	int	i;
	int	nbr;

	i = 0;
	nbr = 0;
	while (i < data->number_of_coders)
	{
		data->coders[i].left = &data->dongles[i];
		nbr = (i + 1) % data->number_of_coders;
		data->coders[i].right = &data->dongles[nbr];
		data->coders[i].last_compile_start = 0;
		data->coders[i].id = i + 1;
		data->coders[i].data = data;
		i++;
	}
}
