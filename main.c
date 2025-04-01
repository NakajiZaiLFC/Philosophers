/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"
#include <signal.h>

static t_data *g_data = NULL;

/*
 * Gets the current simulation state in a thread-safe manner
 */
int	get_simulation_state(t_data *data)
{
	int	state;

	pthread_mutex_lock(&data->state_check);
	state = data->simulation_state;
	pthread_mutex_unlock(&data->state_check);
	return (state);
}

/*
 * Sets the simulation state in a thread-safe manner
 */
void	set_simulation_state(t_data *data, int state)
{
	pthread_mutex_lock(&data->state_check);
	data->simulation_state = state;
	pthread_mutex_unlock(&data->state_check);
}

/*
 * Start the simulation by creating threads for each philosopher and the monitor
 * Returns SUCCESS if successful, ERROR otherwise
 */
int	start_simulation(t_data *data)
{
	int			i;
	pthread_t	monitor;

	// Record start time
	data->start_time = get_time_in_ms();
	if (data->start_time == -1)
		return (ERROR);
	
	// Create philosopher threads
	i = 0;
	while (i < data->nbr_of_philos)
	{
		data->philosophers[i].last_meal_time = data->start_time;
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]))
		{
			set_simulation_state(data, STATE_ERROR);
			return (ERROR_THREAD_CREATE);
		}
		i++;
	}
	
	// Create monitor thread
	if (pthread_create(&monitor, NULL, monitor_routine, data))
	{
			set_simulation_state(data, STATE_ERROR);
			return (ERROR_THREAD_CREATE);
	}
	
	// Wait for philosopher threads to complete
	i = 0;
	while (i < data->nbr_of_philos)
	{
		if (pthread_join(data->philosophers[i++].thread, NULL) != 0)
			return (ERROR);
	}
	
	// Wait for monitor thread to complete
	if (pthread_join(monitor, NULL) != 0)
		return (ERROR);
	
	return (SUCCESS);
}

/*
 * Parses and validates all command line arguments
 * Returns SUCCESS (0) if arguments are valid, otherwise returns an error code
 */
int	parse_arguments(int argc, char **argv)
{
	int	error_code;
	int	i;

	if (argc != 5 && argc != 6)
	{
		print_usage(argv[0]);
		return (ERROR);
	}
	
	// Check each argument
	i = 1;
	while (i < argc)
	{
		error_code = validate_arg(argv[i], i);
		if (error_code != SUCCESS)
		{
			print_error_message(error_code, argv[i]);
			return (ERROR);
		}
		i++;
	}
	
	return (SUCCESS);
}

static void	signal_handler(int signum)
{
	if (signum == SIGINT && g_data)
	{
		printf("\nReceived interrupt signal. Cleaning up...\n");
		set_simulation_state(g_data, STATE_FINISHED);
	}
}

int	main(int argc, char **argv)
{
	t_data	data;
	int		result;

	memset(&data, 0, sizeof(t_data));
	g_data = &data;
	signal(SIGINT, signal_handler);
	
	if (parse_arguments(argc, argv) != SUCCESS)
		return (ERROR);
	
	result = init_data(&data, argc, argv);
	if (result != SUCCESS)
	{
		print_init_error(result);
		free_resources(&data);
		return (ERROR);
	}
	
	result = start_simulation(&data);
	if (result != SUCCESS)
	{
		print_init_error(result);
		free_resources(&data);
		return (ERROR);
	}
	
	free_resources(&data);
	g_data = NULL;
	return (SUCCESS);
} 
