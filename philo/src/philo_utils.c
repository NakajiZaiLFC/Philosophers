/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 01:48:52 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int set_simulation_state(t_data *data, int state)
{
	pthread_mutex_lock(&data->death);
	data->sim_state = state;
	if (state == SIM_STOPPED)
		data->is_dead = 1;
	pthread_mutex_unlock(&data->death);
	return (0);
}

int get_simulation_state(t_data *data)
{
	int state;

	pthread_mutex_lock(&data->death);
	state = data->sim_state;
	pthread_mutex_unlock(&data->death);
	return (state);
}

int check_all_ate(t_data *data)
{
	int i;
	int all_ate;

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
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&data->meal_lock);
	return (all_ate);
}

void handle_meal_completion(t_data *data)
{
	pthread_mutex_lock(&data->print);
	printf("%lld All philosophers have eaten enough\n",
		   time_elapsed(data->start_time));
	pthread_mutex_unlock(&data->print);
	set_simulation_state(data, SIM_COMPLETED);
}

void handle_termination(t_data *data)
{
	int i;
	int should_terminate;

	i = 0;
	should_terminate = 0;
	while (i < data->num_philos)
	{
		pthread_mutex_lock(&data->meal_lock);
		if (time_elapsed(data->start_time) - data->philos[i].last_eat_time > data->time_to_die && data->philos[i].state != PHILO_DEAD)
		{
			pthread_mutex_unlock(&data->meal_lock);
			data->philos[i].state = PHILO_DEAD;
			print_status(&data->philos[i], MSG_DIED);
			set_simulation_state(data, SIM_STOPPED);
			should_terminate = 1;
			break;
		}
		pthread_mutex_unlock(&data->meal_lock);
		i++;
	}
}