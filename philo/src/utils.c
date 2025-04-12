/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: snakajim <snakajim@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 15:17:45 by snakajim          #+#    #+#             */
/*   Updated: 2025/04/12 15:20:58 by snakajim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_atoi(const char *str)
{
	int	i;
	int	sign;
	int	result;

	i = 0;
	sign = 1;
	result = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result * sign);
}

long long	get_time(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

long long	time_elapsed(long long start_time)
{
	return (get_time() - start_time);
}

void	ft_usleep(long long time)
{
	long long	start;
	long long	elapsed;
	long long	remaining;

	start = get_time();
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= time)
			break ;
		remaining = time - elapsed;
		if (remaining > 50)
			usleep(500);
		else if (remaining > 10)
			usleep(200);
		else
			usleep(50);
	}
}

int	is_state(t_data *data, int target_state)
{
	int	result;

	pthread_mutex_lock(&data->death);
	result = (data->sim_state == target_state);
	pthread_mutex_unlock(&data->death);
	return (result);
}
