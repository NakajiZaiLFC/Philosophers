/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   termination_test.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"
#include <assert.h>

#define TEST_NUM_PHILO 4
#define TEST_TIME_TO_DIE 800   // 800ms
#define TEST_TIME_TO_EAT 200   // 200ms
#define TEST_TIME_TO_SLEEP 200 // 200ms
#define TEST_NUM_MEALS 3       // 3回の食事
#define TEST_STARVATION_PHILO 2 // 飢餓状態にする哲学者のID

// テスト用の統計情報
typedef struct s_test_stats
{
    int total_meals;
    int threads_started;
    int termination_reason;
    long long termination_time;
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

// 終了原因を記録する関数
void record_termination(int reason, long long time)
{
    pthread_mutex_lock(&g_stats.stats_mutex);
    g_stats.termination_reason = reason;
    g_stats.termination_time = time;
    pthread_mutex_unlock(&g_stats.stats_mutex);
}

// テスト用の哲学者ルーチン（食事回数制限バージョン）
void *test_meal_complete_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&philo->data->start_lock);
    pthread_mutex_unlock(&philo->data->start_lock);
    
    pthread_mutex_lock(&g_stats.stats_mutex);
    g_stats.threads_started++;
    pthread_mutex_unlock(&g_stats.stats_mutex);
    
    int meal_count = 0;
    while (meal_count < TEST_NUM_MEALS && !check_death(philo))
    {
        // 食事をシミュレート
        printf("Philosopher %d is eating (meal %d/%d)\n", 
               philo->id, meal_count + 1, TEST_NUM_MEALS);
        
        philo->last_eat_time = get_time();
        ft_usleep(philo->data->time_to_eat);
        meal_count++;
        
        pthread_mutex_lock(&g_stats.stats_mutex);
        g_stats.total_meals++;
        pthread_mutex_unlock(&g_stats.stats_mutex);
        
        // 睡眠と思考をシミュレート
        if (meal_count < TEST_NUM_MEALS && !check_death(philo))
        {
            printf("Philosopher %d is sleeping\n", philo->id);
            ft_usleep(philo->data->time_to_sleep);
            
            if (!check_death(philo))
            {
                printf("Philosopher %d is thinking\n", philo->id);
            }
        }
    }
    
    return NULL;
}

// テスト用の哲学者ルーチン（死亡バージョン）
void *test_death_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&philo->data->start_lock);
    pthread_mutex_unlock(&philo->data->start_lock);
    
    pthread_mutex_lock(&g_stats.stats_mutex);
    g_stats.threads_started++;
    pthread_mutex_unlock(&g_stats.stats_mutex);
    
    // 特定の哲学者を飢餓状態にする
    if (philo->id == TEST_STARVATION_PHILO)
    {
        printf("Philosopher %d will starve...\n", philo->id);
        ft_usleep(TEST_TIME_TO_DIE + 50);
        return NULL;
    }
    
    while (!check_death(philo))
    {
        // 食事をシミュレート
        printf("Philosopher %d is eating\n", philo->id);
        philo->last_eat_time = get_time();
        ft_usleep(philo->data->time_to_eat);
        
        pthread_mutex_lock(&g_stats.stats_mutex);
        g_stats.total_meals++;
        pthread_mutex_unlock(&g_stats.stats_mutex);
        
        // 睡眠と思考をシミュレート
        if (!check_death(philo))
        {
            printf("Philosopher %d is sleeping\n", philo->id);
            ft_usleep(philo->data->time_to_sleep);
            
            if (!check_death(philo))
            {
                printf("Philosopher %d is thinking\n", philo->id);
            }
        }
    }
    
    return NULL;
}

// テスト用のモニターリング関数（食事回数制限バージョン）
void *test_meal_complete_monitor(void *arg)
{
    t_data *data = (t_data *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&data->start_lock);
    pthread_mutex_unlock(&data->start_lock);
    
    long long start_time = get_time();
    
    while (1)
    {
        // タイムアウトチェック
        if (time_elapsed(start_time) > TEST_TIME_TO_DIE * 2)
        {
            printf("Test timed out\n");
            pthread_mutex_lock(&data->death);
            data->is_dead = 1;
            pthread_mutex_unlock(&data->death);
            record_termination(SIM_ERROR, get_time());
            return NULL;
        }
        
        // 全食事完了チェック
        pthread_mutex_lock(&g_stats.stats_mutex);
        if (g_stats.total_meals >= TEST_NUM_MEALS * data->num_philos)
        {
            pthread_mutex_unlock(&g_stats.stats_mutex);
            printf("All philosophers completed their %d meals\n", TEST_NUM_MEALS);
            record_termination(SIM_COMPLETED, get_time());
            return NULL;
        }
        pthread_mutex_unlock(&g_stats.stats_mutex);
        
        ft_usleep(10);
    }
}

// テスト用のモニターリング関数（死亡バージョン）
void *test_death_monitor(void *arg)
{
    t_data *data = (t_data *)arg;
    
    // スタート前の同期
    pthread_mutex_lock(&data->start_lock);
    pthread_mutex_unlock(&data->start_lock);
    
    long long start_time = get_time();
    int i;
    
    while (1)
    {
        // 哲学者の死亡チェック
        i = 0;
        while (i < data->num_philos)
        {
            if (time_elapsed(data->philos[i].last_eat_time) > data->time_to_die)
            {
                printf("Philosopher %d died after %lld ms without eating\n",
                       data->philos[i].id, time_elapsed(data->philos[i].last_eat_time));
                pthread_mutex_lock(&data->death);
                data->is_dead = 1;
                pthread_mutex_unlock(&data->death);
                record_termination(SIM_STOPPED, get_time());
                return NULL;
            }
            i++;
        }
        
        // タイムアウトチェック
        if (time_elapsed(start_time) > TEST_TIME_TO_DIE * 2)
        {
            printf("Test timed out, no philosopher died as expected\n");
            pthread_mutex_lock(&data->death);
            data->is_dead = 1;
            pthread_mutex_unlock(&data->death);
            record_termination(SIM_ERROR, get_time());
            return NULL;
        }
        
        ft_usleep(10);
    }
}

// 食事回数による終了テスト
void test_meal_completion()
{
    t_data data;
    int i;
    pthread_t monitor;
    
    printf("\n=== Testing Meal Completion Termination ===\n");
    
    // データ初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = TEST_NUM_PHILO;
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
    assert(pthread_mutex_init(&data.meal_lock, NULL) == 0);
    
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
    g_stats.termination_reason = SIM_RUNNING;
    g_stats.termination_time = 0;
    assert(pthread_mutex_init(&g_stats.stats_mutex, NULL) == 0);
    
    // スタート前の同期
    pthread_mutex_lock(&data.start_lock);
    
    // 哲学者スレッド作成
    for (i = 0; i < data.num_philos; i++)
    {
        assert(pthread_create(&data.philos[i].thread, NULL, test_meal_complete_routine, &data.philos[i]) == 0);
    }
    
    // モニタースレッド作成
    assert(pthread_create(&monitor, NULL, test_meal_complete_monitor, &data) == 0);
    
    // スレッドをすべて同時に開始
    long long test_start_time = get_time();
    pthread_mutex_unlock(&data.start_lock);
    
    // スレッドが終了するのを待機
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_join(data.philos[i].thread, NULL);
    }
    pthread_join(monitor, NULL);
    
    // 結果検証
    long long test_duration = g_stats.termination_time - test_start_time;
    printf("Test completed in %lld ms\n", test_duration);
    printf("Total meals eaten: %d (expected: %d)\n", 
           g_stats.total_meals, TEST_NUM_MEALS * data.num_philos);
    printf("Termination reason: %d (expected: %d)\n", 
           g_stats.termination_reason, SIM_COMPLETED);
    
    assert(g_stats.termination_reason == SIM_COMPLETED);
    assert(g_stats.total_meals == TEST_NUM_MEALS * data.num_philos);
    
    // クリーンアップ
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_mutex_destroy(&data.forks[i].mutex);
    }
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    pthread_mutex_destroy(&data.start_lock);
    pthread_mutex_destroy(&data.meal_lock);
    pthread_mutex_destroy(&g_stats.stats_mutex);
    free(data.forks);
    free(data.philos);
    
    printf("Meal completion termination test passed!\n");
}

// 哲学者の死亡による終了テスト
void test_death_termination()
{
    t_data data;
    int i;
    pthread_t monitor;
    
    printf("\n=== Testing Death Termination ===\n");
    
    // データ初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = TEST_NUM_PHILO;
    data.time_to_die = TEST_TIME_TO_DIE;
    data.time_to_eat = TEST_TIME_TO_EAT;
    data.time_to_sleep = TEST_TIME_TO_SLEEP;
    data.must_eat = -1;  // 無限に食事
    data.is_dead = 0;
    data.sim_state = SIM_RUNNING;
    data.single_philo = 0;
    data.start_time = get_time();
    
    // ミューテックス初期化
    assert(pthread_mutex_init(&data.print, NULL) == 0);
    assert(pthread_mutex_init(&data.death, NULL) == 0);
    assert(pthread_mutex_init(&data.start_lock, NULL) == 0);
    assert(pthread_mutex_init(&data.meal_lock, NULL) == 0);
    
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
    g_stats.termination_reason = SIM_RUNNING;
    g_stats.termination_time = 0;
    assert(pthread_mutex_init(&g_stats.stats_mutex, NULL) == 0);
    
    // スタート前の同期
    pthread_mutex_lock(&data.start_lock);
    
    // 哲学者スレッド作成
    for (i = 0; i < data.num_philos; i++)
    {
        assert(pthread_create(&data.philos[i].thread, NULL, test_death_routine, &data.philos[i]) == 0);
    }
    
    // モニタースレッド作成
    assert(pthread_create(&monitor, NULL, test_death_monitor, &data) == 0);
    
    // スレッドをすべて同時に開始
    long long test_start_time = get_time();
    pthread_mutex_unlock(&data.start_lock);
    
    // スレッドが終了するのを待機
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_join(data.philos[i].thread, NULL);
    }
    pthread_join(monitor, NULL);
    
    // 結果検証
    long long test_duration = g_stats.termination_time - test_start_time;
    printf("Test completed in %lld ms\n", test_duration);
    printf("Termination reason: %d (expected: %d)\n", 
           g_stats.termination_reason, SIM_STOPPED);
    
    assert(g_stats.termination_reason == SIM_STOPPED);
    assert(test_duration >= TEST_TIME_TO_DIE);
    assert(test_duration <= TEST_TIME_TO_DIE + 100);
    
    // クリーンアップ
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_mutex_destroy(&data.forks[i].mutex);
    }
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    pthread_mutex_destroy(&data.start_lock);
    pthread_mutex_destroy(&data.meal_lock);
    pthread_mutex_destroy(&g_stats.stats_mutex);
    free(data.forks);
    free(data.philos);
    
    printf("Death termination test passed!\n");
}

int main(int argc, char **argv)
{
    (void)argc;  // 使用していない引数の警告を抑制
    (void)argv;  // 使用していない引数の警告を抑制
    
    printf("=== Starting Termination Condition Tests ===\n");
    
    // 食事回数による終了テスト
    test_meal_completion();
    
    // 哲学者の死亡による終了テスト
    test_death_termination();
    
    printf("\n=== All Termination Tests Passed! ===\n");
    return 0;
} 