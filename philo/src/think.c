#include "philo.h"

void philo_think(t_philo *philo)
{
    if (check_death(philo))
        return;
    philo->state = PHILO_THINKING;
    print_status(philo, MSG_THINK);
    adjust_think_time(philo);
}

void adjust_think_time(t_philo *philo)
{
    if (philo->data->num_philos % 2 == 1)
    {
        if (philo->id % 2 == 0)
        {
            int wait_time = philo->data->time_to_eat / 2;
            ft_usleep(wait_time);
        }
    }
    else
        ft_usleep(10);
}
