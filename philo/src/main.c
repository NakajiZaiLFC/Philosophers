#include "philo.h"

static int initialize_philos(t_data *data, int argc, char **argv)
{
	if (argc != 5 && argc != 6)
		return (printf("Error: wrong number of arguments\n"), 1);
	if (init_data(data, argc, argv))
		return (printf("Error: initialization failed\n"), 1);
	if (init_philos(data) != 0)
		return (printf("Error: philosopher initialization failed\n"), free_resources(data), 1);
	return (0);
}

int main(int argc, char **argv)
{
	t_data data;
	int	i;

	if (initialize_philos(&data, argc, argv) != 0)
		return (1);
	
	data.start_time = get_time();
	pthread_mutex_lock(&data.start_lock);
	
	i = 0;
	while (i < data.num_philos)
	{
		data.philos[i].last_eat_time = get_time();
		if (pthread_create(&data.philos[i].thread, NULL, philo_routine, &data.philos[i]) != 0)
			return (handle_thread_creation_error(&data, "Error: failed to create thread"));
		i++;
	}
	
	if (i == data.num_philos)
	{
		if (pthread_create(&data.monitor_thread, NULL, monitor_routine, &data) != 0)
			return (handle_thread_creation_error(&data, "Error: failed to create monitor thread"));
	}
	
	// スレッドが全て起動したら開始ロックを解除
	pthread_mutex_unlock(&data.start_lock);
	
	// 全スレッドの終了を待つ
	handle_termination(&data);
	free_resources(&data);
	return (0);
}
