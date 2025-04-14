/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:32:34 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/14 10:42:29 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	is_dead(t_data *data)
{
	int	result;

	pthread_mutex_lock(&data->death);
	result = data->is_dead;
	pthread_mutex_unlock(&data->death);
	return (result);
}

int	is_time_to_die(t_philo *philo)
{
	long long	time_since_meal;

	time_since_meal = time_since_last_meal(philo);
	return (time_since_meal >= philo->data->time_to_die);
}

long long	time_since_last_meal(t_philo *philo)
{
	long long	current_time;
	long long	result;

	current_time = get_time();
	pthread_mutex_lock(&philo->data->meal_lock);
	if (philo->last_eat_time == 0)
		result = time_elapsed(philo->data->start_time);
	else
		result = current_time - philo->last_eat_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	return (result);
}

int	check_death(t_philo *philo)
{
	int	is_dead;

	pthread_mutex_lock(&philo->data->death);
	is_dead = philo->data->is_dead;
	pthread_mutex_unlock(&philo->data->death);
	return (is_dead);
}

int	check_all_ate(t_data *data)
{
	int	i;
	int	all_ate;

	if (data->must_eat < 0)
		return (0);
	pthread_mutex_lock(&data->meal_lock);
	i = 0;
	all_ate = 1;
	while (i < data->num_philos)
	{
		if (data->philos[i].eat_count < data->must_eat)
		{
			all_ate = 0;
			break ;
		}
		i++;
	}
	pthread_mutex_unlock(&data->meal_lock);
	return (all_ate);
}
