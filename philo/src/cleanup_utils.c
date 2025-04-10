/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 02:20:19 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int register_thread(pthread_t thread, char *description)
{
	return (register_resource((void *)&thread, RESOURCE_THREAD, description));
}

void print_resource_inventory(void)
{
	DEBUG_PRINT("リソースインベントリステータス:");
	DEBUG_PRINT("詳細は実装されていません");
}

int cleanup_single_mutex(pthread_mutex_t *mutex)
{
	int result;

	result = pthread_mutex_destroy(mutex);
	if (result == 0)
		unregister_resource(mutex);
	return (result == 0 ? 0 : 1);
}

int cleanup_multiple_mutexes(pthread_mutex_t *mutexes, int count)
{
	int i;
	int success;

	i = 0;
	success = 1;
	while (i < count)
	{
		if (cleanup_single_mutex(&mutexes[i]) != 0)
			success = 0;
		i++;
	}
	return (success ? 0 : 1);
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
	unregister_resource(forks);
	return (success ? 0 : 1);
}