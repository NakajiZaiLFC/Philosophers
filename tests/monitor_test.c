/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_test.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"
#include <assert.h>

#define TEST_PHILOSOPHRES 5
#define TEST_TIME_TO_DIE 100    // 100ms
#define TEST_TIME_TO_EAT 20     // 20ms
#define TEST_TIME_TO_SLEEP 20   // 20ms
#define TEST_NUM_MEALS -1       // infinite

// テストフラグ
static int test_philosopher_died = 0;

// テスト用の時間測定
static long long test_death_detection_time = 0;

// テスト用のグローバル変数
static pthread_mutex_t g_death_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_is_dead = 0;

// テスト用のphilo_routine関数（実際には呼ばれないがシンボル解決のため必要）
void	*philo_routine(void *arg)
{
    (void)arg; // 未使用パラメータの警告を抑制
    return (NULL);
}

// 時間関連の関数
long long get_current_time_ms(void)
{
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) != 0)
        return (-1);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

// 死亡チェック関数
int check_philosopher_death(t_philo *philo, int time_to_die)
{
    long long current_time = get_current_time_ms();
    long long time_since_last_meal = current_time - philo->last_eat_time;
    
    if (time_since_last_meal > time_to_die)
    {
        pthread_mutex_lock(&g_death_mutex);
        if (!g_is_dead)
        {
            g_is_dead = 1;
            printf("[%lld] Philosopher %d died\n", 
                   current_time - ((t_data *)philo->data)->start_time, philo->id);
            test_philosopher_died = 1;
            test_death_detection_time = current_time - ((t_data *)philo->data)->start_time;
        }
        pthread_mutex_unlock(&g_death_mutex);
        return (1);
    }
    return (0);
}

// すべての哲学者が死亡していないか確認する関数
int check_if_dead(t_philo *philos)
{
    int i;
    t_data *data = (t_data *)philos[0].data;
    
    i = 0;
    while (i < data->num_philos)
    {
        if (check_philosopher_death(&philos[i], data->time_to_die))
            return (1);
        i++;
    }
    return (0);
}

// モニタースレッド関数
void *monitor_test_routine(void *_philos)
{
    t_philo *philos = (t_philo *)_philos;
    
    // 主要ループ - 哲学者の死亡をチェック
    while (!g_is_dead)
    {
        if (check_if_dead(philos))
            break;
        usleep(1000); // 1ms待機
    }
    return (NULL);
}

// テスト用の初期化関数
void init_test_data(t_data *data)
{
    data->num_philos = TEST_PHILOSOPHRES;
    data->time_to_die = TEST_TIME_TO_DIE;
    data->time_to_eat = TEST_TIME_TO_EAT;
    data->time_to_sleep = TEST_TIME_TO_SLEEP;
    data->must_eat = TEST_NUM_MEALS;
    data->is_dead = 0;
    data->start_time = 0;
    
    // フォークの初期化
    if (init_forks(data) != 0)
    {
        printf("Error: Could not initialize forks\n");
        exit(1);
    }
    
    // 哲学者の初期化 - 実際のinit_philos内部の処理を一部再現
    data->philos = malloc(sizeof(t_philo) * data->num_philos);
    if (!data->philos)
    {
        printf("Error: Could not allocate memory for philosophers\n");
        exit(1);
    }
    
    for (int i = 0; i < data->num_philos; i++)
    {
        data->philos[i].id = i + 1;
        data->philos[i].state = PHILO_THINKING;
        data->philos[i].left_fork = i;
        data->philos[i].right_fork = (i + 1) % data->num_philos;
        data->philos[i].eat_count = 0;
        data->philos[i].last_eat_time = 0;
        data->philos[i].data = data;
    }
}

// リソース解放関数
void free_test_resources(t_data *data)
{
    int i;
    
    if (data->forks)
    {
        for (i = 0; i < data->num_philos; i++)
            pthread_mutex_destroy(&data->forks[i].mutex);
        free(data->forks);
        data->forks = NULL;
    }
    
    if (data->philos)
    {
        free(data->philos);
        data->philos = NULL;
    }
    
    pthread_mutex_destroy(&data->print);
    pthread_mutex_destroy(&data->death);
    pthread_mutex_destroy(&g_death_mutex);
}

// テスト専用のmain関数
int monitor_test_main(void)
{
    t_data data;
    pthread_t monitor_thread;
    int i;
    
    printf("=== Starting Death Monitoring Test ===\n");
    
    // テストデータの初期化
    memset(&data, 0, sizeof(t_data));
    if (pthread_mutex_init(&data.print, NULL) != 0 ||
        pthread_mutex_init(&data.death, NULL) != 0)
    {
        printf("Error: Could not initialize mutexes\n");
        return (1);
    }
    
    init_test_data(&data);
    data.start_time = get_current_time_ms();
    
    // 全哲学者の食事時間を現在に設定
    for (i = 0; i < data.num_philos; i++)
        data.philos[i].last_eat_time = data.start_time;
    
    // 哲学者2の最後の食事時間をtime_to_die - 10ms前に設定
    data.philos[2].last_eat_time = data.start_time - (data.time_to_die - 10);
    
    // モニタースレッドの作成
    if (pthread_create(&monitor_thread, NULL, monitor_test_routine, data.philos))
    {
        printf("Error creating monitor thread\n");
        free_test_resources(&data);
        return (1);
    }
    
    // 死亡検出を待機
    int wait_counter = 0;
    while (!test_philosopher_died && wait_counter < 30)
    {
        usleep(5000); // 5ms待機
        wait_counter++;
    }
    
    // 死亡が検出されたか確認
    if (test_philosopher_died)
    {
        printf("Philosopher death correctly detected in %lld ms\n", 
               test_death_detection_time);
        
        // 検出時間が期待値に近いか確認
        if (test_death_detection_time >= 0 &&
            test_death_detection_time <= data.time_to_die + 20)
        {
            printf("Death detection time is within acceptable range\n");
        }
        else
        {
            printf("Error: Death detection time outside acceptable range\n");
            pthread_join(monitor_thread, NULL);
            free_test_resources(&data);
            return (1);
        }
    }
    else
    {
        printf("Error: Philosopher death not detected within timeout\n");
        pthread_join(monitor_thread, NULL);
        free_test_resources(&data);
        return (1);
    }
    
    // スレッド終了待機
    pthread_join(monitor_thread, NULL);
    
    // リソース解放
    free_test_resources(&data);
    
    printf("=== Death Monitoring Test Passed ===\n");
    return (0);
}

// main関数
int main(int argc, char **argv)
{
    (void)argc;  // 未使用パラメータの警告を抑制
    (void)argv;  // 未使用パラメータの警告を抑制
    return monitor_test_main();
} 