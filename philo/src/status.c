#include "philo.h"

void print_status(t_philo *philo, char *status)
{
	long long time;

	pthread_mutex_lock(&philo->data->print);
	time = time_elapsed(philo->data->start_time);
	printf("%lld %d %s\n", time, philo->id, status);
	pthread_mutex_unlock(&philo->data->print);
}

int check_death(t_philo *philo)
{
	int is_dead;

	pthread_mutex_lock(&philo->data->death);
	is_dead = philo->data->is_dead;
	pthread_mutex_unlock(&philo->data->death);
	return (is_dead);
}

int set_simulation_state(t_data *data, int state)
{
	pthread_mutex_lock(&data->death);
	data->sim_state = state;
	if (state == SIM_STOPPED)
		data->is_dead = 1;
	pthread_mutex_unlock(&data->death);
	return (0);
}
