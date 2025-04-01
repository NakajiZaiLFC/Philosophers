/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"
#include <errno.h>

/*
 * Safely cleanup a single mutex, ignoring any errors
 */
void	cleanup_mutex(pthread_mutex_t *mutex)
{
	if (mutex)
	{
		if (pthread_mutex_destroy(mutex) != 0)
			print_system_error("pthread_mutex_destroy");
	}
}

/*
 * Cleanup an array of mutexes up to a specified count
 */
void	cleanup_mutexes(t_data *data, int count)
{
	int	i;

	if (data->forks)
	{
		i = 0;
		while (i < count)
		{
			cleanup_mutex(&data->forks[i]);
			i++;
		}
	}
}

/*
 * Wait for all philosopher threads to finish
 */
static void	join_philosopher_threads(t_data *data)
{
	int	i;
	int	ret;

	if (!data->philosophers)
		return;

	i = 0;
	while (i < data->nbr_of_philos)
	{
		pthread_mutex_lock(&data->state_check);
		if (data->philosophers[i].thread)
		{
			pthread_t thread = data->philosophers[i].thread;
			data->philosophers[i].thread = 0;  // Clear thread ID before joining
			pthread_mutex_unlock(&data->state_check);
			
			ret = pthread_join(thread, NULL);
			if (ret != 0 && ret != ESRCH)  // ESRCH means thread not found
			{
				print_system_error("pthread_join");
			}
		}
		else
		{
			pthread_mutex_unlock(&data->state_check);
		}
		i++;
	}
}

/*
 * Free all resources allocated for the simulation
 * This function ensures proper cleanup of all resources:
 * - Sets simulation state to finished
 * - Waits for threads to finish
 * - Destroys all mutexes
 * - Frees allocated memory
 */
void	free_resources(t_data *data)
{
	if (!data)
		return;

	// Set simulation state to finished and wait
	set_simulation_state(data, STATE_FINISHED);
	usleep(200000); // Increase wait time for threads to notice the state change

	// Wait for all philosopher threads to finish
	join_philosopher_threads(data);

	// Free philosophers array
	if (data->philosophers)
	{
		free(data->philosophers);
		data->philosophers = NULL;
	}

	// Destroy all mutexes
	if (data->forks)
	{
		cleanup_mutexes(data, data->nbr_of_philos);
		free(data->forks);
		data->forks = NULL;
	}

	// Destroy individual mutexes
	cleanup_mutex(&data->writing);
	cleanup_mutex(&data->meal_check);
	cleanup_mutex(&data->death_check);
	cleanup_mutex(&data->state_check);
} 