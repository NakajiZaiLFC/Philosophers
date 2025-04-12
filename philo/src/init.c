/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: snakajim <snakajim@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:21:27 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/12 15:24:04 by snakajim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	init_data(t_data *data, int argc, char **argv)
{
	if (validate_args(argc, argv))
		return (1);
	if (parse_args(data, argc, argv))
		return (1);
	data->is_dead = 0;
	data->sim_state = SIM_RUNNING;
	if (data->num_philos == 1)
		data->single_philo = 1;
	else
		data->single_philo = 0;
	if (init_mutex(data) != 0)
		return (1);
	if (init_forks(data) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		cleanup_single_mutex(&data->start_lock);
		cleanup_single_mutex(&data->meal_lock);
		return (1);
	}
	return (0);
}

int	init_mutex(t_data *data)
{
	if (pthread_mutex_init(&data->print, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->death, NULL) != 0)
		return (cleanup_single_mutex(&data->print), 1);
	if (pthread_mutex_init(&data->start_lock, NULL) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		return (1);
	}
	if (pthread_mutex_init(&data->meal_lock, NULL) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		cleanup_single_mutex(&data->start_lock);
		return (1);
	}
	return (0);
}

int	init_forks(t_data *data)
{
	int	i;

	data->forks = malloc(sizeof(t_fork) * data->num_philos);
	if (!data->forks)
		return (1);
	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_mutex_init(&data->forks[i].mutex, NULL) != 0)
		{
			while (--i >= 0)
				cleanup_single_mutex(&data->forks[i].mutex);
			free(data->forks);
			return (1);
		}
		data->forks[i].state = FORK_AVAILABLE;
		data->forks[i].owner_id = -1;
		i++;
	}
	return (0);
}

int	init_philos(t_data *data)
{
	int	i;

	data->philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!data->philos)
		return (1);
	i = 0;
	while (i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].state = PHILO_THINKING;
		data->philos[i].left_fork = i;
		data->philos[i].right_fork = (i + 1) % data->num_philos;
		data->philos[i].eat_count = 0;
		data->philos[i].last_eat_time = 0;
		data->philos[i].data = data;
		i++;
	}
	return (0);
}

int	parse_args(t_data *data, int argc, char **argv)
{
	data->num_philos = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->must_eat = ft_atoi(argv[5]);
	else
		data->must_eat = -1;
	if (data->num_philos <= 0 || data->time_to_die <= 0
		|| data->time_to_eat <= 0 || data->time_to_sleep <= 0 || (argc == 6
			&& data->must_eat <= 0))
		return (1);
	return (0);
}
