#include "philo.h"

long long get_time(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

int ft_atoi(const char *str)
{
	int i;
	int sign;
	int result;

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

long long time_elapsed(long long start_time)
{
	return (get_time() - start_time);
}

static void sleep_strategy(long long remaining)
{
	if (remaining > 50)
		usleep(500);
	else if (remaining > 10)
		usleep(200);
	else
		usleep(50);
}

void ft_usleep(long long time)
{
	long long start;
	long long elapsed;
	long long remaining;

	start = get_time();
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= time)
			break;
		remaining = time - elapsed;
		sleep_strategy(remaining);
	}
}
