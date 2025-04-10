/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 01:48:22 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int can_take_fork(t_philo *philo, int fork_index)
{
	int can_take;

	pthread_mutex_lock(&philo->data->forks[fork_index].mutex);
	can_take = (philo->data->forks[fork_index].state == FORK_AVAILABLE);
	pthread_mutex_unlock(&philo->data->forks[fork_index].mutex);
	return (can_take);
}

int take_fork_safe(t_philo *philo, int fork_index)
{
	pthread_mutex_lock(&philo->data->forks[fork_index].mutex);
	if (philo->data->forks[fork_index].state == FORK_AVAILABLE && !is_dead(philo->data))
	{
		philo->data->forks[fork_index].state = FORK_IN_USE;
		philo->data->forks[fork_index].owner_id = philo->id;
		pthread_mutex_unlock(&philo->data->forks[fork_index].mutex);
		print_status(philo, MSG_FORK);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->forks[fork_index].mutex);
	return (0);
}

int is_time_to_die(t_philo *philo)
{
	long long time_since_meal;

	time_since_meal = time_since_last_meal(philo);
	return (time_since_meal > philo->data->time_to_die);
}

long long time_since_last_meal(t_philo *philo)
{
	long long current_time;
	long long result;

	current_time = get_time();
	pthread_mutex_lock(&philo->data->meal_lock);
	if (philo->last_eat_time == 0)
		result = time_elapsed(philo->data->start_time);
	else
		result = current_time - philo->last_eat_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	return (result);
}

int check_and_take_both_forks(t_philo *philo)
{
	int first_fork;
	int second_fork;

	if (philo->id % 2 == 0)
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	else
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	if (!take_fork_safe(philo, first_fork))
		return (0);
	if (!take_fork_safe(philo, second_fork))
	{
		philo->data->forks[first_fork].state = FORK_AVAILABLE;
		philo->data->forks[first_fork].owner_id = -1;
		pthread_mutex_unlock(&philo->data->forks[first_fork].mutex);
		return (0);
	}
	return (1);
}