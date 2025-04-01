/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

/*
 * Checks if the string contains only digits
 * Returns 1 if valid, 0 if not
 */
int	ft_is_number(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '+')
		i++;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

/*
 * Convert a string to an integer with overflow protection
 * Returns the converted value or -1 on error
 */
int	ft_atoi(const char *str)
{
	int			sign;
	long long	result;

	sign = 1;
	result = 0;
	while (*str == ' ' || (*str >= '\t' && *str <= '\r'))
		str++;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			sign = -1;
		str++;
	}
	while (ft_isdigit(*str))
	{
		result = result * 10 + (*str - '0');
		if (result * sign > INT_MAX || result * sign < INT_MIN)
			return (-1);
		str++;
	}
	return (result * sign);
}

/*
 * Convert a string to a long long integer with overflow protection
 * Returns the converted value or -1 on error
 */
long long	ft_atoll(const char *str)
{
	int			sign;
	long long	result;

	sign = 1;
	result = 0;
	while (*str == ' ' || (*str >= '\t' && *str <= '\r'))
		str++;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			sign = -1;
		str++;
	}
	while (ft_isdigit(*str))
	{
		if (result > (LLONG_MAX - (*str - '0')) / 10)
			return (-1);
		result = result * 10 + (*str - '0');
		str++;
	}
	return (result * sign);
}

/* Note: get_time_in_ms() has been moved to time.c */

void	print_status(t_data *data, int id, char *status)
{
	long long	current_time;
	int			simulation_state;

	// Get current time before locking to minimize lock time
	current_time = get_time_in_ms() - data->start_time;
	
	// Check simulation state
	simulation_state = get_simulation_state(data);
	if (simulation_state == STATE_ERROR)
		return;
	
	pthread_mutex_lock(&data->writing);
	
	// Only print if:
	// 1. Simulation is still running, or
	// 2. This is a death message, or
	// 3. All philosophers have eaten enough (final status)
	if (simulation_state == STATE_RUNNING || 
		!strcmp(status, DIED_MSG) ||
		(simulation_state == STATE_FINISHED && check_if_all_ate(data)))
	{
		// Ensure consistent format with leading zeros for timestamps
		printf("%6lld %3d %s\n", current_time, id + 1, status);
	}
	
	pthread_mutex_unlock(&data->writing);
}

/*
 * Validates that all arguments are valid positive numbers within range
 * Returns an error code:
 * 0 = SUCCESS
 * 1 = ERROR_NOT_NUMBER (not a valid number)
 * 2 = ERROR_NEGATIVE_OR_ZERO (number is <= 0)
 * 3 = ERROR_TOO_LARGE (number is too large)
 * 4 = ERROR_PHILO_COUNT (invalid philosopher count, must be between 1-200)
 */
int	validate_arg(char *arg, int arg_idx)
{
	long long	num;

	if (!ft_is_number(arg))
		return (ERROR_NOT_NUMBER);
	
	num = ft_atoll(arg);
	
	if (num <= 0)
		return (ERROR_NEGATIVE_OR_ZERO);
	
	if (num > INT_MAX)
		return (ERROR_TOO_LARGE);
	
	// For philosopher count (first argument), we limit to 200
	if (arg_idx == 1 && (num < 1 || num > 200))
		return (ERROR_PHILO_COUNT);
	
	return (SUCCESS);
}

/*
 * Checks all command-line arguments for validity
 * Returns SUCCESS (0) if all arguments are valid, otherwise returns an error code
 */
int	check_args(int argc, char **argv)
{
	int	i;
	int	error_code;

	i = 1;
	while (i < argc)
	{
		error_code = validate_arg(argv[i], i);
		if (error_code != SUCCESS)
			return (error_code);
		i++;
	}
	return (SUCCESS);
} 