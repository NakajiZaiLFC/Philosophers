/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_mgmt.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 01:49:46 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int emergency_cleanup(t_data *data)
{
	pthread_mutex_lock(&data->death);
	data->is_dead = 1;
	pthread_mutex_unlock(&data->death);

	if (data->philos)
	{
		free(data->philos);
		unregister_resource(data->philos);
	}
	if (data->forks)
	{
		cleanup_forks(data->forks, data->num_philos);
	}

	cleanup_single_mutex(&data->print);
	cleanup_single_mutex(&data->death);
	cleanup_single_mutex(&data->start_lock);
	cleanup_single_mutex(&data->meal_lock);

	return (0);
}

int free_resources(t_data *data)
{
	if (data->philos)
	{
		free(data->philos);
		unregister_resource(data->philos);
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

int verify_cleanup(t_data *data)
{
	int remaining_resources;

	remaining_resources = cleanup_all_resources();
	if (remaining_resources == 0)
		DEBUG_PRINT("すべてのリソースが正常に解放されました");
	else
		DEBUG_PRINT("警告: %d個の未解放リソースがあります", remaining_resources);

	return (remaining_resources);
}

void safe_exit(t_data *data, int exit_code)
{
	pthread_mutex_lock(&data->death);
	data->is_dead = 1;
	pthread_mutex_unlock(&data->death);

	free_resources(data);
	verify_cleanup(data);

	if (exit_code != 0)
		DEBUG_PRINT("エラーが発生しました。コード: %d", exit_code);
	exit(exit_code);
}