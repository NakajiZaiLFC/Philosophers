/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
 * Initialize the state-related mutexes and variables
 * Returns SUCCESS if initialization is successful, ERROR otherwise
 */
int	init_state_mutexes(t_data *data)
{
	// Initialize writing mutex (for synchronized console output)
	if (pthread_mutex_init(&data->writing, NULL))
		return (ERROR_MUTEX_INIT);
	
	// Initialize meal check mutex (for tracking last meal time)
	if (pthread_mutex_init(&data->meal_check, NULL))
	{
		cleanup_mutex(&data->writing);
		return (ERROR_MUTEX_INIT);
	}
	
	// Initialize death check mutex (for checking if anyone died)
	if (pthread_mutex_init(&data->death_check, NULL))
	{
		cleanup_mutex(&data->writing);
		cleanup_mutex(&data->meal_check);
		return (ERROR_MUTEX_INIT);
	}
	
	// Initialize state check mutex (for checking simulation state)
	if (pthread_mutex_init(&data->state_check, NULL))
	{
		cleanup_mutex(&data->writing);
		cleanup_mutex(&data->meal_check);
		cleanup_mutex(&data->death_check);
		return (ERROR_MUTEX_INIT);
	}
	
	return (SUCCESS);
}

/*
 * Initialize all mutexes needed for the simulation
 * Returns SUCCESS if initialization is successful, or an error code otherwise
 */
int	init_mutexes(t_data *data)
{
	int	i;
	int	result;

	// Allocate memory for fork mutexes
	data->forks = malloc(sizeof(pthread_mutex_t) * data->nbr_of_philos);
	if (!data->forks)
		return (ERROR_MALLOC_FORKS);
	
	// Initialize each fork mutex
	i = 0;
	while (i < data->nbr_of_philos)
	{
		if (pthread_mutex_init(&data->forks[i], NULL))
		{
			// Cleanup already initialized mutexes
			cleanup_mutexes(data, i);
			free(data->forks);
			data->forks = NULL;
			return (ERROR_MUTEX_INIT);
		}
		i++;
	}
	
	// Initialize state-related mutexes
	result = init_state_mutexes(data);
	if (result != SUCCESS)
	{
		// Cleanup resources and return error
		cleanup_mutexes(data, data->nbr_of_philos);
		free(data->forks);
		data->forks = NULL;
		return (result);
	}
	
	return (SUCCESS);
}

/*
 * Initialize the philosophers array with proper values
 * Returns SUCCESS if initialization is successful, or an error code otherwise
 */
int	init_philosophers(t_data *data)
{
	int	i;

	// Allocate memory for philosophers array
	data->philosophers = malloc(sizeof(t_philosopher) * data->nbr_of_philos);
	if (!data->philosophers)
		return (ERROR_MALLOC_PHILOS);
	
	// Initialize each philosopher
	i = 0;
	while (i < data->nbr_of_philos)
	{
		// Set basic properties
		data->philosophers[i].id = i;
		data->philosophers[i].meals_eaten = 0;
		data->philosophers[i].last_meal_time = 0;
		data->philosophers[i].is_full = FALSE;
		
		// Assign forks (to avoid deadlock, even philosophers pick left fork first,
		// odd philosophers pick right fork first)
		data->philosophers[i].left_fork_id = i;
		data->philosophers[i].right_fork_id = (i + 1) % data->nbr_of_philos;
		
		// Link back to main data structure
		data->philosophers[i].data = data;
		i++;
	}
	
	return (SUCCESS);
}

/*
 * Initialize the simulation state
 */
void	init_state(t_data *data)
{
	data->all_ate = FALSE;
	data->anyone_died = FALSE;
	data->simulation_state = STATE_RUNNING;
}

/*
 * Initialize main data structure with values from command line arguments
 * Returns SUCCESS if initialization is successful, or an error code otherwise
 */
int	init_data(t_data *data, int argc, char **argv)
{
	int	result;

	// Parse command line arguments
	data->nbr_of_philos = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->nbr_times_to_eat = ft_atoi(argv[5]);
	else
		data->nbr_times_to_eat = -1;
	
	// Initialize simulation state
	init_state(data);
	
	// Double-check validation (should already be validated in parse_arguments)
	if (data->nbr_of_philos < MIN_PHILOSOPHERS || 
		data->nbr_of_philos > MAX_PHILOSOPHERS ||
		data->time_to_die <= 0 || 
		data->time_to_eat <= 0 || 
		data->time_to_sleep <= 0)
		return (ERROR);
	
	// Initialize mutexes
	result = init_mutexes(data);
	if (result != SUCCESS)
		return (result);
	
	// Initialize philosophers
	result = init_philosophers(data);
	if (result != SUCCESS)
	{
		// Clean up mutexes if philosopher initialization fails
		free_resources(data);
		return (result);
	}
	
	return (SUCCESS);
} 