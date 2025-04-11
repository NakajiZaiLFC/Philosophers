#include "philo.h"

int cleanup_single_mutex(pthread_mutex_t *mutex)
{
	int result;

	result = pthread_mutex_destroy(mutex);
	if (result == 0)
		return (0);
	else
		return (1);
}

int cleanup_forks(t_fork *forks, int count)
{
	int i;
	int success;

	i = 0;
	success = 1;
	while (i < count)
	{
		if (cleanup_single_mutex(&forks[i].mutex) != 0)
			success = 0;
		i++;
	}
	free(forks);
	if (success)
		return (0);
	else
		return (1);
}

int free_resources(t_data *data)
{
	if (data->philos)
	{
		free(data->philos);
		data->philos = NULL;
	}
	if (data->forks)
	{
		cleanup_forks(data->forks, data->num_philos);
		data->forks = NULL;
	}
	
	cleanup_single_mutex(&data->print);
	cleanup_single_mutex(&data->death);
	cleanup_single_mutex(&data->start_lock);
	cleanup_single_mutex(&data->meal_lock);

	return (0);
}
