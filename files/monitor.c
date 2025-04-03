/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
 * Check if any philosopher has died.
 * Returns 1 if a philosopher has died, 0 otherwise.
 * Thread-safe function to check the global death flag.
 */
int	check_if_anyone_died(t_data *data)
{
	int	died;

	pthread_mutex_lock(&data->death_check);
	died = data->anyone_died;
	pthread_mutex_unlock(&data->death_check);
	return (died);
}

/*
 * Mark a philosopher as dead and set the simulation state to finished.
 * This ensures that all threads will detect the death and terminate properly.
 */
void	mark_philosopher_death(t_data *data, int philo_id)
{
	// Set death flag first
	pthread_mutex_lock(&data->death_check);
	data->anyone_died = TRUE;
	pthread_mutex_unlock(&data->death_check);
	
	// Update simulation state
	set_simulation_state(data, STATE_FINISHED);
	
	// Print death message - this must be the last action to ensure
	// accurate timestamps and prevent further activity output
	print_status(data, philo_id, DIED_MSG);
}

/*
 * Check if all philosophers have eaten the required number of times.
 * Returns 1 if all philosophers have eaten enough, 0 otherwise.
 * Thread-safe function to check the global meal completion flag.
 */
int	check_if_all_ate(t_data *data)
{
	int	done;

	pthread_mutex_lock(&data->meal_check);
	done = data->all_ate;
	pthread_mutex_unlock(&data->meal_check);
	return (done);
}

/*
 * Check if a philosopher has died due to starvation.
 * Uses precise time checking to ensure detection within 10ms.
 * Returns 1 if the philosopher has died, 0 otherwise.
 */
int	check_death(t_data *data, int philo_id)
{
	long long	current_time;
	long long	time_since_last_meal;
	
	// Early exit if simulation already finished
	if (get_simulation_state(data) != STATE_RUNNING)
		return (0);
	
	pthread_mutex_lock(&data->meal_check);
	current_time = get_time_in_ms();
	time_since_last_meal = time_diff_ms(current_time, 
								data->philosophers[philo_id].last_meal_time);
	
	// Check if philosopher has died of starvation
	if (time_since_last_meal > data->time_to_die)
	{
		pthread_mutex_unlock(&data->meal_check);
		mark_philosopher_death(data, philo_id);
		return (1);
	}
	
	pthread_mutex_unlock(&data->meal_check);
	return (0);
}

/*
 * Check if all philosophers have eaten the required number of times.
 * This is only used if the nbr_times_to_eat argument is provided.
 * If all have eaten enough, sets the all_ate flag and ends the simulation.
 */
void	check_all_ate(t_data *data)
{
	int	i;
	int	all_full;
	int	total_meals;

	// Skip check if no meal limit set or simulation already finished
	if (data->nbr_times_to_eat == -1 || get_simulation_state(data) != STATE_RUNNING)
		return;
	
	all_full = TRUE;
	total_meals = 0;
	i = 0;
	
	pthread_mutex_lock(&data->meal_check);
	// Check each philosopher's meal count
	while (i < data->nbr_of_philos)
	{
		total_meals += data->philosophers[i].meals_eaten;
		if (data->philosophers[i].meals_eaten < data->nbr_times_to_eat)
		{
			all_full = FALSE;
			break;
		}
		i++;
	}
	
	// If all philosophers have eaten enough, end the simulation
	if (all_full)
	{
		data->all_ate = TRUE;
		// Print final status showing total meals eaten
		printf("All philosophers have eaten %d meals each (total: %d meals)\n",
			data->nbr_times_to_eat, total_meals);
		set_simulation_state(data, STATE_FINISHED);
	}
	pthread_mutex_unlock(&data->meal_check);
}

/*
 * Calculate the time until the next philosopher will die, if no more meals are eaten.
 * This allows the monitor to sleep intelligently instead of constantly polling.
 * Returns time in microseconds, capped at 10ms (10000us) for guaranteed death detection.
 */
long	calculate_next_check_interval(t_data *data)
{
	long long	next_death_time;
	long long	current_time;
	long long	time_remaining;
	long		interval_us;
	int			i;

	next_death_time = LLONG_MAX;
	current_time = get_time_in_ms();
	
	// Find the philosopher closest to death
	for (i = 0; i < data->nbr_of_philos; i++)
	{
		pthread_mutex_lock(&data->meal_check);
		// Calculate when this philosopher would die
		time_remaining = (data->philosophers[i].last_meal_time + data->time_to_die) - current_time;
		pthread_mutex_unlock(&data->meal_check);
		
		// Keep track of the earliest death time
		if (time_remaining < next_death_time)
			next_death_time = time_remaining;
	}
	
	// Convert to microseconds for usleep
	interval_us = next_death_time * 1000;
	
	// Cap at 10ms to ensure we never miss a death by more than 10ms
	if (interval_us > 10000)
		interval_us = 10000;
	else if (interval_us < 100)
		interval_us = 100;  // Minimum sleep to avoid excessive CPU usage
	
	return (interval_us);
}

/*
 * The monitor thread routine.
 * Continuously checks if any philosopher has died or if all have eaten enough.
 * Uses adaptive sleep intervals to ensure death detection within 10ms while
 * minimizing CPU usage.
 */
void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;
	long	sleep_interval;

	data = (t_data *)arg;
	while (get_simulation_state(data) == STATE_RUNNING)
	{
		// Check all philosophers for death
		i = 0;
		while (i < data->nbr_of_philos && get_simulation_state(data) == STATE_RUNNING)
		{
			if (check_death(data, i))
				return (NULL);
			i++;
		}
		
		// Check if all philosophers have eaten enough
		check_all_ate(data);
		
		// Calculate optimal sleep interval based on next potential death
		sleep_interval = calculate_next_check_interval(data);
		usleep(sleep_interval);
	}
	return (NULL);
} 