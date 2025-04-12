/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: snakajim <snakajim@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:36:36 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/12 15:38:57 by snakajim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	get_simulation_state(t_data *data)
{
	int	state;

	pthread_mutex_lock(&data->death);
	state = data->sim_state;
	pthread_mutex_unlock(&data->death);
	return (state);
}

int	set_simulation_state(t_data *data, int state)
{
	pthread_mutex_lock(&data->death);
	data->sim_state = state;
	if (state == SIM_STOPPED)
		data->is_dead = 1;
	pthread_mutex_unlock(&data->death);
	return (0);
}

void	handle_termination(t_data *data)
{
	int	i;

	pthread_join(data->monitor_thread, NULL);
	i = 0;
	while (i < data->num_philos)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
}

void	handle_meal_completion(t_data *data)
{
	pthread_mutex_lock(&data->print);
	printf("%lld All philosophers have eaten enough\n",
		time_elapsed(data->start_time));
	pthread_mutex_unlock(&data->print);
	set_simulation_state(data, SIM_COMPLETED);
}

int	try_get_forks(t_philo *philo)
{
	int	attempts;
	int	max_attempts;

	attempts = 0;
	max_attempts = 100;
	while (!check_death(philo)
		&& get_simulation_state(philo->data) == SIM_RUNNING)
	{
		if (can_take_fork(philo, philo->left_fork) && can_take_fork(philo,
				philo->right_fork))
			return (1);
		if (++attempts > max_attempts)
		{
			ft_usleep(philo->data->time_to_eat / 10);
			attempts = 0;
		}
		else
			ft_usleep(1);
	}
	return (0);
}
