#include "philo.h"

void philo_sleep(t_philo *philo)
{
    if (check_death(philo))
        return;
    
    pthread_mutex_lock(&philo->data->death);
    philo->state = PHILO_SLEEPING;
    pthread_mutex_unlock(&philo->data->death);
    
    print_status(philo, MSG_SLEEP);
    ft_usleep(philo->data->time_to_sleep);
}
