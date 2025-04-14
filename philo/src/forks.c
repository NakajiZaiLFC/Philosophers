/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   forks.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:26:46 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/14 12:36:24 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"


static int	check_and_take_both_forks(t_philo *philo, int first_fork, int second_fork);

int	can_take_fork(t_philo *philo, int fork_index)
{
	int	can_take;

	pthread_mutex_lock(&philo->data->forks[fork_index].mutex);
	can_take = (philo->data->forks[fork_index].state == FORK_AVAILABLE);
	pthread_mutex_unlock(&philo->data->forks[fork_index].mutex);
	return (can_take);
}

int	take_fork_safe(t_philo *philo, int fork_index)
{
	pthread_mutex_lock(&philo->data->forks[fork_index].mutex);
	if (philo->data->forks[fork_index].state == FORK_AVAILABLE)
	{
		philo->data->forks[fork_index].state = FORK_IN_USE;
		philo->data->forks[fork_index].owner_id = philo->id;
		pthread_mutex_unlock(&philo->data->forks[fork_index].mutex);
		print_status(philo, MSG_FORK);
		return (1);
	}
	else if (philo->id == philo->data->num_philos && 
			time_since_last_meal(philo) > (philo->data->time_to_die * 3 / 4))
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

int	check_and_take_both_forks_safe(t_philo *philo)
{
	if (is_dead(philo->data))
		return (0);
	if (philo->data->single_philo)
	{
		take_fork_safe(philo, philo->left_fork);
		ft_usleep(philo->data->time_to_die + 1);
		return (0);
	}
	return (check_and_take_both_forks(philo, 0, 0));
}

static int	check_and_take_both_forks(t_philo *philo, int first_fork, int second_fork)
{
	if (philo->id == philo->data->num_philos)
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	else if (philo->id % 2 == 0)
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
		pthread_mutex_lock(&philo->data->forks[first_fork].mutex);
		philo->data->forks[first_fork].state = FORK_AVAILABLE;
		philo->data->forks[first_fork].owner_id = -1;
		return (pthread_mutex_unlock(&philo->data->forks[first_fork].mutex),0);
	}
	return (1);
}

void	release_both_forks(t_philo *philo, int first_fork, int second_fork)
{
	if (philo->id == philo->data->num_philos)
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	else if (philo->id % 2 == 0)
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	else
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	pthread_mutex_lock(&philo->data->forks[first_fork].mutex);
	philo->data->forks[first_fork].state = FORK_AVAILABLE;
	philo->data->forks[first_fork].owner_id = -1;
	pthread_mutex_unlock(&philo->data->forks[first_fork].mutex);
	pthread_mutex_lock(&philo->data->forks[second_fork].mutex);
	philo->data->forks[second_fork].state = FORK_AVAILABLE;
	philo->data->forks[second_fork].owner_id = -1;
	pthread_mutex_unlock(&philo->data->forks[second_fork].mutex);
}
