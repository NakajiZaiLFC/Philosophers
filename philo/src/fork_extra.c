/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_extra.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 01:48:36 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int check_and_take_both_forks_safe(t_philo *philo)
{
	if (is_dead(philo->data))
		return (0);
	if (philo->data->single_philo)
	{
		take_fork_safe(philo, philo->left_fork);
		ft_usleep(philo->data->time_to_die + 1);
		return (0);
	}
	return (check_and_take_both_forks(philo));
}

void release_both_forks(t_philo *philo)
{
	int first_fork;
	int second_fork;

	if (philo->id % 2 == 0)
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