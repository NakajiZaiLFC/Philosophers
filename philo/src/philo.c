#include "philo.h"

int handle_thread_creation_error(t_data *data, char *err_msg)
{
	data->is_dead = 1;
	set_simulation_state(data, SIM_ERROR);
	printf("%s\n", err_msg);
	return (pthread_mutex_unlock(&data->start_lock), handle_termination(data), free_resources(data), 1);
}

int get_simulation_state(t_data *data)
{
	int state;

	pthread_mutex_lock(&data->death);
	state = data->sim_state;
	pthread_mutex_unlock(&data->death);
	return (state);
}

void *philo_routine(void *arg)
{
	t_philo *philo;

	philo = (t_philo *)arg;

	pthread_mutex_lock(&philo->data->start_lock);
	pthread_mutex_unlock(&philo->data->start_lock);
	if (philo->data->single_philo)
	{
		print_status(philo, MSG_FORK);
		ft_usleep(philo->data->time_to_die);
		return (NULL);
	}
	if (philo->id % 2 == 0)
	{
		ft_usleep(philo->data->time_to_eat / 2);
	}
	while (get_simulation_state(philo->data) == SIM_RUNNING)
	{
		eat(philo);
		if (get_simulation_state(philo->data) != SIM_RUNNING)
			break;
		sleep_and_think(philo);
	}
	return (NULL);
}

void sleep_and_think(t_philo *philo)
{
    philo_sleep(philo);
    if (check_death(philo))
        return; 
    philo_think(philo);
    try_get_forks(philo);
}

int try_get_forks(t_philo *philo)
{
    int attempts;
    int max_attempts;

	attempts = 0;
	max_attempts = 100;
    while (!check_death(philo) && get_simulation_state(philo->data) == SIM_RUNNING)
    {
        if (can_take_fork(philo, philo->left_fork) && can_take_fork(philo, philo->right_fork))
            return (1);
        if (++attempts > max_attempts)
        {
            ft_usleep(philo->data->time_to_eat / 10);
            attempts = 0;
        }
        else
            ft_usleep(1);
    }
    return (0);
}
