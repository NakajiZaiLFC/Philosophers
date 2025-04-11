#include "philo.h"

void handle_termination(t_data *data)
{
	int i;

	// モニタースレッドが終了するのを待つ
	pthread_join(data->monitor_thread, NULL);

	// 各哲学者スレッドが終了するのを待つ
	i = 0;
	while (i < data->num_philos)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
}
