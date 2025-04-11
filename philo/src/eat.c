#include "philo.h"

void eat(t_philo *philo)
{
	if (!check_and_take_both_forks_safe(philo))
		return;
	philo->state = PHILO_EATING;
	print_status(philo, MSG_EAT);
	long long current_time = get_time();
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_eat_time = current_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	ft_usleep(philo->data->time_to_eat);
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->eat_count++;
	pthread_mutex_unlock(&philo->data->meal_lock);
	release_both_forks(philo);
}

int check_and_take_both_forks_safe(t_philo *philo)
{
	if (is_dead(philo->data))
		return (0);
	if (philo->data->single_philo)
	{
		take_fork_safe(philo, philo->left_fork);
		ft_usleep(philo->data->time_to_die + 1);
		return (0);
	}
	return (check_and_take_both_forks(philo));
}

int check_all_ate(t_data *data)
{
	int i;
	int all_ate;

	if (data->must_eat < 0)
		return (0);
	pthread_mutex_lock(&data->meal_lock);
	i = 0;
	all_ate = 1;
	while (i < data->num_philos)
	{
		if (data->philos[i].eat_count < data->must_eat)
		{
			all_ate = 0;
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&data->meal_lock);
	return (all_ate);
}

void handle_meal_completion(t_data *data)
{
	pthread_mutex_lock(&data->print);
	printf("%lld All philosophers have eaten enough\n",
		   time_elapsed(data->start_time));
	pthread_mutex_unlock(&data->print);
	set_simulation_state(data, SIM_COMPLETED);
}
