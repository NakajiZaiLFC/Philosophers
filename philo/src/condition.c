#include "philo.h"

int is_state(t_data *data, int target_state)
{
    int result;

    pthread_mutex_lock(&data->death);
    result = (data->sim_state == target_state);
    pthread_mutex_unlock(&data->death);
    return (result);
}

int is_dead(t_data *data)
{
    int result;

    pthread_mutex_lock(&data->death);
    result = data->is_dead;
    pthread_mutex_unlock(&data->death);
    return (result);
}

void release_both_forks(t_philo *philo)
{
	int first_fork;
	int second_fork;

	if (philo->id % 2 == 0)
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	else
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	pthread_mutex_lock(&philo->data->forks[first_fork].mutex);
	philo->data->forks[first_fork].state = FORK_AVAILABLE;
	philo->data->forks[first_fork].owner_id = -1;
	pthread_mutex_unlock(&philo->data->forks[first_fork].mutex);

	pthread_mutex_lock(&philo->data->forks[second_fork].mutex);
	philo->data->forks[second_fork].state = FORK_AVAILABLE;
	philo->data->forks[second_fork].owner_id = -1;
	pthread_mutex_unlock(&philo->data->forks[second_fork].mutex);
}
