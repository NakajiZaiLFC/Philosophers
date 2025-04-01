/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
 * Check if the simulation should continue for this philosopher.
 * Returns TRUE if the philosopher should stop, FALSE otherwise.
 */
static int	should_stop(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	return (get_simulation_state(data) != STATE_RUNNING || 
			check_if_anyone_died(data) || 
			check_if_all_ate(data));
}

/*
 * Handle the case where there is only one philosopher.
 * With only one fork, the philosopher can't eat and will die.
 */
static void	*handle_single_philosopher(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	// Take the only available fork
	pthread_mutex_lock(&data->forks[philo->left_fork_id]);
	print_status(data, philo->id, FORK_MSG);
	
	// Wait until death
	smart_sleep(data->time_to_die, data);
	
	// Release the fork before thread exits
	pthread_mutex_unlock(&data->forks[philo->left_fork_id]);
	
	return (NULL);
}

/*
 * Take forks in an order that prevents deadlock.
 * Even philosophers take right fork first, odd philosophers take left fork first.
 * This asymmetry ensures that not all philosophers try to take the same fork first.
 */
static void	take_forks(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	
	// Even philosophers take right fork first, odd take left first
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(&data->forks[philo->right_fork_id]);
		print_status(data, philo->id, FORK_MSG);
		pthread_mutex_lock(&data->forks[philo->left_fork_id]);
		print_status(data, philo->id, FORK_MSG);
	}
	else
	{
		pthread_mutex_lock(&data->forks[philo->left_fork_id]);
		print_status(data, philo->id, FORK_MSG);
		pthread_mutex_lock(&data->forks[philo->right_fork_id]);
		print_status(data, philo->id, FORK_MSG);
	}
}

/*
 * Release both forks after eating.
 */
static void	release_forks(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	
	// Release in reverse order of acquisition to minimize contention
	if (philo->id % 2 == 0)
	{
		pthread_mutex_unlock(&data->forks[philo->left_fork_id]);
		pthread_mutex_unlock(&data->forks[philo->right_fork_id]);
	}
	else
	{
		pthread_mutex_unlock(&data->forks[philo->right_fork_id]);
		pthread_mutex_unlock(&data->forks[philo->left_fork_id]);
	}
}

/*
 * Handles the eating state of a philosopher.
 * The philosopher picks up two forks, eats for time_to_eat,
 * and then puts down the forks.
 */
void	eating(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	
	// Take forks with deadlock prevention strategy
	take_forks(philo);
	
	// Update last meal time with mutex protection
	pthread_mutex_lock(&data->meal_check);
	philo->last_meal_time = get_time_in_ms();
	print_status(data, philo->id, EAT_MSG);
	pthread_mutex_unlock(&data->meal_check);
	
	// Sleep for time_to_eat, checking for simulation state changes
	smart_sleep(data->time_to_eat, data);
	
	// Update meal count and check if philosopher is full
	pthread_mutex_lock(&data->meal_check);
	philo->meals_eaten++;
	if (data->nbr_times_to_eat != -1 && 
		philo->meals_eaten >= data->nbr_times_to_eat)
		philo->is_full = TRUE;
	pthread_mutex_unlock(&data->meal_check);
	
	// Release forks
	release_forks(philo);
}

/*
 * Handles the sleeping state of a philosopher.
 * The philosopher sleeps for time_to_sleep milliseconds.
 */
void	sleeping(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	
	// Only proceed if simulation is still running
	if (should_stop(philo))
		return;
	
	print_status(data, philo->id, SLEEP_MSG);
	smart_sleep(data->time_to_sleep, data);
}

/*
 * Handles the thinking state of a philosopher.
 * The philosopher thinks until they can start eating.
 */
void	thinking(t_philosopher *philo)
{
	t_data	*data;

	data = philo->data;
	
	// Only proceed if simulation is still running
	if (should_stop(philo))
		return;
	
	print_status(data, philo->id, THINK_MSG);
	
	// Add a small delay for even philosophers to prevent all philosophers
	// from trying to eat at the same time
	if (philo->id % 2 == 0)
		usleep(500);
}

/*
 * The main routine for each philosopher thread.
 * The philosopher alternates between eating, sleeping, and thinking.
 */
void	*philosopher_routine(void *arg)
{
	t_philosopher	*philo;
	t_data			*data;

	philo = (t_philosopher *)arg;
	data = philo->data;
	
	// Initialize last meal time at the start of simulation
	pthread_mutex_lock(&data->meal_check);
	philo->last_meal_time = data->start_time;
	pthread_mutex_unlock(&data->meal_check);
	
	// Special case: only one philosopher
	if (data->nbr_of_philos == 1)
		return (handle_single_philosopher(philo));
	
	// Stagger start times to prevent initial deadlock
	// Odd philosophers wait a bit to start, creating asymmetry
	if (philo->id % 2 != 0)
		usleep(data->time_to_eat * 500); // 50% of time_to_eat in microseconds
	
	// Main philosopher loop
	while (!should_stop(philo))
	{
		eating(philo);
		sleeping(philo);
		thinking(philo);
	}
	
	return (NULL);
} 