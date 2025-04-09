/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
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
 * Print detailed error message for system call failures
 */
void	print_system_error(const char *syscall_name)
{
	printf("Error: %s failed: %s\n", syscall_name, strerror(errno));
}

/*
 * Handle thread creation error with proper cleanup
 */
int	handle_thread_error(t_data *data, int created_count)
{
	int	i;

	printf("Error: Failed to create thread %d\n", created_count + 1);
	
	// Detach already created threads
	i = 0;
	while (i < created_count)
	{
		pthread_detach(data->philosophers[i].thread);
		i++;
	}
	
	return (ERROR_THREAD_CREATE);
}

/*
 * Handle mutex initialization error with proper cleanup
 */
int	handle_mutex_error(t_data *data, int mutex_count, const char *mutex_name)
{
	printf("Error: Failed to initialize %s mutex\n", mutex_name);
	cleanup_mutexes(data, mutex_count);
	return (ERROR_MUTEX_INIT);
}

/*
 * Prints the usage message showing the correct format for command line arguments
 */
void	print_usage(char *program_name)
{
	printf("Usage: %s number_of_philosophers time_to_die time_to_eat ", program_name);
	printf("time_to_sleep [number_of_times_each_philosopher_must_eat]\n\n");
	printf("Arguments:\n");
	printf("  number_of_philosophers: The number of philosophers (1-%d)\n", MAX_PHILOSOPHERS);
	printf("  time_to_die: Time in milliseconds until a philosopher dies from starvation\n");
	printf("  time_to_eat: Time in milliseconds it takes to eat\n");
	printf("  time_to_sleep: Time in milliseconds it takes to sleep\n");
	printf("  number_of_times_each_philosopher_must_eat: [Optional] Stop after all philosophers eat this many times\n");
}

/*
 * Prints a specific error message based on the error code for argument validation
 */
void	print_error_message(int error_code, char *arg)
{
	printf("Error: ");
	
	if (error_code == ERROR_NOT_NUMBER)
		printf("'%s' is not a valid number", arg);
	else if (error_code == ERROR_NEGATIVE_OR_ZERO)
		printf("'%s' must be a positive number greater than zero", arg);
	else if (error_code == ERROR_TOO_LARGE)
		printf("'%s' is too large (exceeds maximum integer value)", arg);
	else if (error_code == ERROR_PHILO_COUNT)
		printf("Number of philosophers '%s' must be between %d and %d",
			arg, MIN_PHILOSOPHERS, MAX_PHILOSOPHERS);
	else
		printf("Unknown error occurred");
	
	printf("\n");
}

/*
 * Prints an initialization error message based on the error code
 */
void	print_init_error(int error_code)
{
	printf("Error: ");
	
	if (error_code == ERROR_MALLOC_FORKS)
		printf("Failed to allocate memory for forks");
	else if (error_code == ERROR_MALLOC_PHILOS)
		printf("Failed to allocate memory for philosophers");
	else if (error_code == ERROR_MUTEX_INIT)
		printf("Failed to initialize mutex");
	else if (error_code == ERROR_THREAD_CREATE)
		printf("Failed to create thread");
	else if (error_code == ERROR_INVALID_ARGS)
		printf("Invalid arguments provided");
	else if (error_code == ERROR_SYSTEM_CALL)
		printf("System call failed");
	else
		printf("Initialization failed");
	
	printf("\n");
} 