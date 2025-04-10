/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threading.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 01:48:02 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"

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

int update_meal_count(t_philo *philo)
{
    int result;

    result = 1;
    pthread_mutex_lock(&philo->data->meal_lock);
    if (!is_dead(philo->data))
    {
        philo->eat_count++;
        philo->last_eat_time = get_time();
    }
    else
        result = 0;
    pthread_mutex_unlock(&philo->data->meal_lock);
    return (result);
}

int bidirectional_lock(pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    if (m1 < m2)
    {
        first = m1;
        second = m2;
    }
    else
    {
        first = m2;
        second = m1;
    }
    pthread_mutex_lock(first);
    pthread_mutex_lock(second);
    return (1);
}

void bidirectional_unlock(pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    if (m1 < m2)
    {
        first = m1;
        second = m2;
    }
    else
    {
        first = m2;
        second = m1;
    }
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
}