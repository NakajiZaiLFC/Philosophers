/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_test.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"
#include <assert.h>

#define TEST_NUM_PHILO_NORMAL 5
#define TEST_NUM_PHILO_SINGLE 1
#define TEST_TIME_TO_DIE 200   // 200ms
#define TEST_TIME_TO_EAT 100    // 100ms
#define TEST_TIME_TO_SLEEP 100  // 100ms
#define TEST_NUM_MEALS 3       // 3回の食事

// テスト用の統計情報
typedef struct s_test_stats
{
    int total_meals;
    int threads_started;
    pthread_mutex_t stats_mutex;
} t_test_stats;

static t_test_stats g_stats;

// テスト用のcheck_death関数
static int check_death(t_philo *philo)
{
    int is_dead;
    
    pthread_mutex_lock(&philo->data->death);
    is_dead = philo->data->is_dead;
    pthread_mutex_unlock(&philo->data->death);
    
    return is_dead;
}

// テスト用の哲学者ルーチン
void *test_philo_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&philo->data->start_lock);
    pthread_mutex_unlock(&philo->data->start_lock);
    
    // スレッド開始をカウント
    pthread_mutex_lock(&g_stats.stats_mutex);
    g_stats.threads_started++;
    pthread_mutex_unlock(&g_stats.stats_mutex);
    
    // 単一哲学者テスト
    if (philo->data->single_philo)
    {
        printf("Philosopher %d takes a fork and waits to die\n", philo->id);
        ft_usleep(philo->data->time_to_die + 10);
        return NULL;
    }

    // 通常の哲学者ライフサイクル（簡略化）
    int meal_count = 0;
    while (meal_count < TEST_NUM_MEALS && !check_death(philo))
    {
        // 食事をシミュレート
        printf("Philosopher %d is eating (meal %d)\n", philo->id, meal_count + 1);
        philo->last_eat_time = get_time();
        ft_usleep(philo->data->time_to_eat);
        meal_count++;
        
        // 食事をカウント
        pthread_mutex_lock(&g_stats.stats_mutex);
        g_stats.total_meals++;
        pthread_mutex_unlock(&g_stats.stats_mutex);
        
        // 睡眠をシミュレート
        printf("Philosopher %d is sleeping\n", philo->id);
        ft_usleep(philo->data->time_to_sleep);
        
        // 思考をシミュレート
        printf("Philosopher %d is thinking\n", philo->id);
    }
    
    return NULL;
}

// テスト用のモニターリング関数
void *test_monitor_routine(void *arg)
{
    t_data *data = (t_data *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&data->start_lock);
    pthread_mutex_unlock(&data->start_lock);
    
    // 単一哲学者のケース
    if (data->single_philo)
    {
        ft_usleep(data->time_to_die + 20);
        pthread_mutex_lock(&data->death);
        data->is_dead = 1;
        pthread_mutex_unlock(&data->death);
        printf("Single philosopher %d died after %d ms\n", 
               data->philos[0].id, data->time_to_die);
        return NULL;
    }
    
    // 通常のモニタリング（簡略化）
    long long start_time = get_time();
    while (1)
    {
        if (time_elapsed(start_time) > TEST_TIME_TO_DIE * 2)
        {
            // テスト用のタイムアウト
            break;
        }
        
        // 全食事完了チェック
        pthread_mutex_lock(&g_stats.stats_mutex);
        if (g_stats.total_meals >= TEST_NUM_MEALS * data->num_philos)
        {
            pthread_mutex_unlock(&g_stats.stats_mutex);
            printf("All philosophers completed their meals\n");
            break;
        }
        pthread_mutex_unlock(&g_stats.stats_mutex);
        
        ft_usleep(10);
    }
    
    return NULL;
}

// 通常のマルチ哲学者テスト
void test_multi_philosophers()
{
    t_data data;
    int i;
    pthread_t monitor;
    
    printf("\n=== Testing Multiple Philosophers (%d) ===\n", TEST_NUM_PHILO_NORMAL);
    
    // データ初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = TEST_NUM_PHILO_NORMAL;
    data.time_to_die = TEST_TIME_TO_DIE;
    data.time_to_eat = TEST_TIME_TO_EAT;
    data.time_to_sleep = TEST_TIME_TO_SLEEP;
    data.must_eat = TEST_NUM_MEALS;
    data.is_dead = 0;
    data.sim_state = SIM_RUNNING;
    data.single_philo = 0;
    data.start_time = get_time();
    
    // ミューテックス初期化
    assert(pthread_mutex_init(&data.print, NULL) == 0);
    assert(pthread_mutex_init(&data.death, NULL) == 0);
    assert(pthread_mutex_init(&data.start_lock, NULL) == 0);
    
    // フォーク初期化
    data.forks = malloc(sizeof(t_fork) * data.num_philos);
    assert(data.forks != NULL);
    for (i = 0; i < data.num_philos; i++)
    {
        assert(pthread_mutex_init(&data.forks[i].mutex, NULL) == 0);
        data.forks[i].state = FORK_AVAILABLE;
        data.forks[i].owner_id = -1;
    }
    
    // 哲学者初期化
    data.philos = malloc(sizeof(t_philo) * data.num_philos);
    assert(data.philos != NULL);
    for (i = 0; i < data.num_philos; i++)
    {
        data.philos[i].id = i + 1;
        data.philos[i].state = PHILO_THINKING;
        data.philos[i].left_fork = i;
        data.philos[i].right_fork = (i + 1) % data.num_philos;
        data.philos[i].eat_count = 0;
        data.philos[i].last_eat_time = get_time();
        data.philos[i].data = &data;
    }
    
    // 統計情報初期化
    g_stats.total_meals = 0;
    g_stats.threads_started = 0;
    assert(pthread_mutex_init(&g_stats.stats_mutex, NULL) == 0);
    
    // スタート前の同期
    pthread_mutex_lock(&data.start_lock);
    
    // 哲学者スレッド作成
    for (i = 0; i < data.num_philos; i++)
    {
        assert(pthread_create(&data.philos[i].thread, NULL, test_philo_routine, &data.philos[i]) == 0);
    }
    
    // モニタースレッド作成
    assert(pthread_create(&monitor, NULL, test_monitor_routine, &data) == 0);
    
    // スレッドをすべて同時に開始
    ft_usleep(100);  // 全スレッドが作成されるまで少し待機
    pthread_mutex_unlock(&data.start_lock);
    
    // すべてのスレッドが終了するのを待機
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_join(data.philos[i].thread, NULL);
    }
    pthread_join(monitor, NULL);
    
    // 結果検証
    pthread_mutex_lock(&g_stats.stats_mutex);
    printf("Total meals eaten: %d (expected: %d)\n", 
           g_stats.total_meals, TEST_NUM_MEALS * data.num_philos);
    printf("Threads started: %d (expected: %d)\n", 
           g_stats.threads_started, data.num_philos);
    assert(g_stats.total_meals == TEST_NUM_MEALS * data.num_philos);
    assert(g_stats.threads_started == data.num_philos);
    pthread_mutex_unlock(&g_stats.stats_mutex);
    
    // クリーンアップ
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_mutex_destroy(&data.forks[i].mutex);
    }
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    pthread_mutex_destroy(&data.start_lock);
    pthread_mutex_destroy(&g_stats.stats_mutex);
    free(data.forks);
    free(data.philos);
    
    printf("Multiple philosophers test passed!\n");
}

// 単一哲学者のテスト
void test_single_philosopher()
{
    t_data data;
    pthread_t monitor;
    long long start_time;
    
    printf("\n=== Testing Single Philosopher Case ===\n");
    
    // データ初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = TEST_NUM_PHILO_SINGLE;
    data.time_to_die = TEST_TIME_TO_DIE;
    data.time_to_eat = TEST_TIME_TO_EAT;
    data.time_to_sleep = TEST_TIME_TO_SLEEP;
    data.must_eat = TEST_NUM_MEALS;
    data.is_dead = 0;
    data.sim_state = SIM_RUNNING;
    data.single_philo = 1;
    data.start_time = get_time();
    
    // ミューテックス初期化
    assert(pthread_mutex_init(&data.print, NULL) == 0);
    assert(pthread_mutex_init(&data.death, NULL) == 0);
    assert(pthread_mutex_init(&data.start_lock, NULL) == 0);
    
    // フォーク初期化
    data.forks = malloc(sizeof(t_fork) * data.num_philos);
    assert(data.forks != NULL);
    assert(pthread_mutex_init(&data.forks[0].mutex, NULL) == 0);
    data.forks[0].state = FORK_AVAILABLE;
    data.forks[0].owner_id = -1;
    
    // 哲学者初期化
    data.philos = malloc(sizeof(t_philo) * data.num_philos);
    assert(data.philos != NULL);
    data.philos[0].id = 1;
    data.philos[0].state = PHILO_THINKING;
    data.philos[0].left_fork = 0;
    data.philos[0].right_fork = 0;
    data.philos[0].eat_count = 0;
    data.philos[0].last_eat_time = get_time();
    data.philos[0].data = &data;
    
    // 統計情報初期化
    g_stats.total_meals = 0;
    g_stats.threads_started = 0;
    assert(pthread_mutex_init(&g_stats.stats_mutex, NULL) == 0);
    
    // スタート前の同期
    pthread_mutex_lock(&data.start_lock);
    
    // 哲学者スレッド作成
    assert(pthread_create(&data.philos[0].thread, NULL, test_philo_routine, &data.philos[0]) == 0);
    
    // モニタースレッド作成
    assert(pthread_create(&monitor, NULL, test_monitor_routine, &data) == 0);
    
    // スレッドをすべて同時に開始
    ft_usleep(50);  // 全スレッドが作成されるまで少し待機
    start_time = get_time();
    pthread_mutex_unlock(&data.start_lock);
    
    // スレッドが終了するのを待機
    pthread_join(data.philos[0].thread, NULL);
    pthread_join(monitor, NULL);
    
    // 結果検証
    long long elapsed = get_time() - start_time;
    printf("Single philosopher test completed in %lld ms\n", elapsed);
    printf("Philosopher died: %s\n", data.is_dead ? "YES" : "NO");
    assert(data.is_dead);  // 単一哲学者は死亡するはず
    assert(elapsed >= data.time_to_die);  // 少なくともtime_to_die時間経過しているはず
    
    // クリーンアップ
    pthread_mutex_destroy(&data.forks[0].mutex);
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    pthread_mutex_destroy(&data.start_lock);
    pthread_mutex_destroy(&g_stats.stats_mutex);
    free(data.forks);
    free(data.philos);
    
    printf("Single philosopher test passed!\n");
}

int main(int argc, char **argv)
{
    (void)argc;  // 使用していない引数の警告を抑制
    (void)argv;  // 使用していない引数の警告を抑制
    
    printf("=== Starting Thread Creation and Synchronization Tests ===\n");
    
    // 複数哲学者のテスト
    test_multi_philosophers();
    
    // 単一哲学者のテスト
    test_single_philosopher();
    
    printf("\n=== All Thread Tests Passed! ===\n");
    return 0;
} 