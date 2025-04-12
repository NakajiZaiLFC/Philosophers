#include "philo.h"

void philo_think(t_philo *philo)
{
    if (check_death(philo))
        return;
    
    pthread_mutex_lock(&philo->data->death);
    philo->state = PHILO_THINKING;
    pthread_mutex_unlock(&philo->data->death);
    
    print_status(philo, MSG_THINK);
    adjust_think_time(philo);
}

void adjust_think_time(t_philo *philo)
{
    // n%2が1（奇数の哲学者）の場合、特殊な処理
    if (philo->data->num_philos % 2 == 1)
    {
        // 死亡時間が短い場合は待機時間を短くする
        if (philo->data->time_to_die <= 410)
        {
            ft_usleep(5); // 非常に短い待機時間
        }
        else
        {
            // 偶数IDの哲学者だけ待機させる（デッドロック防止）
            if (philo->id % 2 == 0)
            {
                int wait_time = philo->data->time_to_eat / 4; // 待機時間を短縮
                ft_usleep(wait_time);
            }
        }
    }
    else
    {
        // 偶数の哲学者の場合は短い待機
        ft_usleep(5);
    }
}
