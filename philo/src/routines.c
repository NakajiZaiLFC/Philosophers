/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routines.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:39:10 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/14 09:09:37 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	pthread_mutex_lock(&philo->data->start_lock);
	pthread_mutex_unlock(&philo->data->start_lock);
	if (philo->data->single_philo)
	{
		print_status(philo, MSG_FORK);
		ft_usleep(philo->data->time_to_die);
		return (NULL);
	}
	if (philo->id % 2 == 0)
	{
		ft_usleep(philo->data->time_to_eat / 2);
		philo_think(philo);
		try_get_forks(philo);
	}
	while (get_simulation_state(philo->data) == SIM_RUNNING)
	{
		eat(philo);
		if (get_simulation_state(philo->data) != SIM_RUNNING)
			break ;
		sleep_and_think(philo);
	}
	return (NULL);
}

int	handle_thread_creation_error(t_data *data, char *err_msg)
{
	data->is_dead = 1;
	set_simulation_state(data, SIM_ERROR);
	printf("%s\n", err_msg);
	return (pthread_mutex_unlock(&data->start_lock), handle_termination(data),
		free_resources(data), 1);
}

void	print_status(t_philo *philo, char *status)
{
	long long	time;

	pthread_mutex_lock(&philo->data->print);
	time = time_elapsed(philo->data->start_time);
	printf("%lld %d %s\n", time, philo->id, status);
	pthread_mutex_unlock(&philo->data->print);
}
