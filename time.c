/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
 * Get the current time in milliseconds since epoch.
 * Uses gettimeofday to provide millisecond precision.
 * Returns:
 *   - Current time in milliseconds
 *   - -1 if gettimeofday fails
 */
long long	get_time_in_ms(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/*
 * Wrapper for the get_time_in_ms function for backward compatibility.
 */
long long	get_current_time(void)
{
	return (get_time_in_ms());
}

/*
 * Calculate the time difference between two timestamps in milliseconds.
 * Parameters:
 *   - end_time: The later timestamp
 *   - start_time: The earlier timestamp
 * Returns:
 *   - Time difference in milliseconds
 */
long long	time_diff_ms(long long end_time, long long start_time)
{
	return (end_time - start_time);
}

/*
 * Check if a philosopher should be dead based on their last meal time.
 * Parameters:
 *   - philo: Pointer to philosopher structure
 * Returns:
 *   - TRUE if the philosopher should be dead
 *   - FALSE if the philosopher is still alive
 */
int	is_philosopher_dead(t_philosopher *philo)
{
	long long	current_time;
	long long	time_since_last_meal;

	pthread_mutex_lock(&philo->data->meal_check);
	current_time = get_time_in_ms();
	time_since_last_meal = time_diff_ms(current_time, philo->last_meal_time);
	pthread_mutex_unlock(&philo->data->meal_check);

	return (time_since_last_meal > philo->data->time_to_die);
}

/*
 * A precise sleep function that ensures accurate waiting.
 * Uses short usleep intervals and continuously checks if target time is reached.
 * Also checks if philosophers died during sleep to exit early if needed.
 * Parameters:
 *   - time_to_wait: Time to wait in milliseconds
 *   - data: Pointer to simulation data structure
 */
void	smart_sleep(long long time_to_wait, t_data *data)
{
	long long	start_time;
	long long	current_time;
	long long	elapsed;

	start_time = get_time_in_ms();
	while (TRUE)
	{
		if (get_simulation_state(data) != STATE_RUNNING)
			break;
		
		current_time = get_time_in_ms();
		elapsed = time_diff_ms(current_time, start_time);
		
		if (elapsed >= time_to_wait)
			break;
		
		// Sleep for shorter intervals for better precision
		// Use smaller sleep for short remaining times to avoid oversleeping
		if (time_to_wait - elapsed > 5)
			usleep(1000); // 1ms sleep
		else
			usleep(100);  // 0.1ms sleep for finer control near the end
	}
} 