#include "philo.h"

void philo_sleep(t_philo *philo)
{
    if (check_death(philo))
        return;
    philo->state = PHILO_SLEEPING;
    print_status(philo, MSG_SLEEP);
    ft_usleep(philo->data->time_to_sleep);
}
