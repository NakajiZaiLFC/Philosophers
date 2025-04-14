/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:30:13 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/14 12:16:54 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	eat(t_philo *philo)
{
	long long	current_time;

	if (!check_and_take_both_forks_safe(philo))
	{
		philo_think(philo);
		return (0);
	}
	pthread_mutex_lock(&philo->data->death);
	philo->state = PHILO_EATING;
	pthread_mutex_unlock(&philo->data->death);
	print_status(philo, MSG_EAT);
	current_time = get_time();
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_eat_time = current_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	ft_usleep(philo->data->time_to_eat);
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->eat_count++;
	pthread_mutex_unlock(&philo->data->meal_lock);
	release_both_forks(philo, 0, 0);
	return (1);
}

void	philo_sleep(t_philo *philo)
{
	if (check_death(philo))
		return ;
	pthread_mutex_lock(&philo->data->death);
	philo->state = PHILO_SLEEPING;
	pthread_mutex_unlock(&philo->data->death);
	print_status(philo, MSG_SLEEP);
	ft_usleep(philo->data->time_to_sleep);
}

void	philo_think(t_philo *philo)
{
	if (check_death(philo))
		return ;
	pthread_mutex_lock(&philo->data->death);
	philo->state = PHILO_THINKING;
	pthread_mutex_unlock(&philo->data->death);
	print_status(philo, MSG_THINK);
	adjust_think_time(philo);
}

void	adjust_think_time(t_philo *philo)
{
	int	wait_time;

	if (philo->data->num_philos % 2 == 1)
	{
		if (philo->data->time_to_die <= 500)
			ft_usleep(5);
		else
		{
			if (philo->id % 2 == 0)
			{
				wait_time = philo->data->time_to_eat / 4;
				ft_usleep(wait_time);
			}
		}
	}
	else
		ft_usleep(5);
}

void	sleep_and_think(t_philo *philo)
{
	philo_sleep(philo);
	if (check_death(philo))
		return ;
	philo_think(philo);
	try_get_forks(philo);
}
