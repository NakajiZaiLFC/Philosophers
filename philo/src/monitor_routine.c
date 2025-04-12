/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_routine.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: snakajim <snakajim@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 14:55:58 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/12 15:45:28 by snakajim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	*handle_single_philosopher(t_data *data)
{
	ft_usleep(data->time_to_die + 10);
	pthread_mutex_lock(&data->death);
	data->is_dead = 1;
	data->philos[0].state = PHILO_DEAD;
	pthread_mutex_unlock(&data->death);
	print_status(&data->philos[0], MSG_DIED);
	set_simulation_state(data, SIM_STOPPED);
	return (NULL);
}

static int	is_philosopher_dead(t_data *data, int philo_index)
{
	long long	last_meal_time;
	long long	elapsed;

	pthread_mutex_lock(&data->meal_lock);
	last_meal_time = data->philos[philo_index].last_eat_time;
	pthread_mutex_unlock(&data->meal_lock);
	elapsed = time_elapsed(last_meal_time);
	return (elapsed > data->time_to_die);
}

static void	*handle_philosopher_death(t_data *data, int philo_index)
{
	pthread_mutex_lock(&data->death);
	if (!data->is_dead)
	{
		data->philos[philo_index].state = PHILO_DEAD;
		data->is_dead = 1;
		pthread_mutex_unlock(&data->death);
		print_status(&data->philos[philo_index], MSG_DIED);
		set_simulation_state(data, SIM_STOPPED);
		return (NULL);
	}
	pthread_mutex_unlock(&data->death);
	return (NULL);
}

static void	*run_monitoring_loop(t_data *data)
{
	int	i;

	i = 0;
	while (is_state(data, SIM_RUNNING))
	{
		if (i < data->num_philos)
		{
			if (is_philosopher_dead(data, i))
				return (handle_philosopher_death(data, i), NULL);
			i++;
		}
		else
		{
			i = 0;
			if (check_all_ate(data) && is_state(data, SIM_RUNNING))
				return (handle_meal_completion(data), NULL);
			ft_usleep(1);
		}
	}
	return (NULL);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	pthread_mutex_lock(&data->start_lock);
	pthread_mutex_unlock(&data->start_lock);
	if (data->single_philo)
		return (handle_single_philosopher(data), NULL);
	return (run_monitoring_loop(data), NULL);
}
