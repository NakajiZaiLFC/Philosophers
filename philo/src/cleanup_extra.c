/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup_extra.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 13:31:59 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

int	cleanup_thread(pthread_t thread, int should_join)
{
	int result;

	result = 0;
	if (should_join)
		result = pthread_join(thread, NULL);
	unregister_resource((void *)&thread);
	if (result == 0)
		return (0);
	return (1);
}

int	cleanup_threads(pthread_t *threads, int count, int should_join)
{
	int i;
	int success;

	i = 0;
	success = 1;
	while (i < count)
	{
		if (cleanup_thread(threads[i], should_join) != 0)
			success = 0;
		i++;
	}
	if (success)
		return (0);
	return (1);
}

int cleanup_memory(void *ptr)
{
	if (ptr)
	{
		free(ptr);
		unregister_resource(ptr);
	}
	return (0);
}

int cleanup_all_resources_by_type(int type)
{
	DEBUG_PRINT("タイプ %d のすべてのリソースをクリーンアップします", type);
	return (0);
}

int cleanup_all_resources(void)
{
	cleanup_all_resources_by_type(RESOURCE_MUTEX);
	cleanup_all_resources_by_type(RESOURCE_THREAD);
	cleanup_all_resources_by_type(RESOURCE_MEMORY);
	return (0);
}
