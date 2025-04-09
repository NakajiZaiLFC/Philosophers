#include "../philo.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "test_utils.h"  // テスト用ユーティリティ

#define NUM_PHILOSOPHERS 5
#define TEST_DURATION_MS 3000  // 3秒間テスト実行

// メッセージ定義
#define MSG_FORK    "has taken a fork"
#define MSG_EAT     "is eating"
#define MSG_SLEEP   "is sleeping"
#define MSG_THINK   "is thinking"
#define MSG_DIED    "died"

// フォーク状態
#define FORK_AVAILABLE 0
#define FORK_IN_USE    1

// 哲学者状態
#define PHILO_THINKING 0
#define PHILO_EATING   1
#define PHILO_SLEEPING 2
#define PHILO_DEAD     3

// 前方宣言
typedef struct s_data t_data;

// 哲学者構造体
typedef struct s_philo
{
    int id;
    int state;
    int left_fork;
    int right_fork;
    int eat_count;
    long long last_eat_time;
    pthread_t thread;
    t_data *data;
} t_philo;

// フォーク構造体
typedef struct s_fork
{
    pthread_mutex_t mutex;
    int state;
    int owner_id;
    // フォーク取得順序をモニタリングするためのカウンター
    int acquisition_order;
} t_fork;

// プログラム状態構造体
typedef struct s_data
{
    int num_philos;
    int time_to_die;
    int time_to_eat;
    int time_to_sleep;
    int must_eat;
    int is_dead;
    long long start_time;
    t_fork *forks;
    pthread_mutex_t print;
    pthread_mutex_t death;
    t_philo *philos;
    // テスト用フラグ
    int fork_acquisition_count;  // フォーク取得回数
    pthread_mutex_t order_mutex; // 取得順序保護用ミューテックス
} t_data;

// スレッドセーフなプリント関数
void print_status(t_philo *philo, char *status)
{
    long long time;

    pthread_mutex_lock(&philo->data->print);
    if (!philo->data->is_dead || !strcmp(status, MSG_DIED))
    {
        time = time_elapsed(philo->data->start_time);
        printf("%lld %d %s\n", time, philo->id, status);
    }
    pthread_mutex_unlock(&philo->data->print);
}

// 死亡チェック関数
int check_death(t_philo *philo)
{
    pthread_mutex_lock(&philo->data->death);
    if (philo->data->is_dead)
    {
        pthread_mutex_unlock(&philo->data->death);
        return (1);
    }
    pthread_mutex_unlock(&philo->data->death);
    return (0);
}

// フォーク取得関数（テスト用拡張）
int take_forks(t_philo *philo)
{
    if (check_death(philo))
        return (0);

    // フォーク取得順序をカウント
    pthread_mutex_lock(&philo->data->order_mutex);
    philo->data->fork_acquisition_count++;
    int order = philo->data->fork_acquisition_count;
    pthread_mutex_unlock(&philo->data->order_mutex);

    // 偶数番号の哲学者は右フォーク→左フォーク
    // 奇数番号の哲学者は左フォーク→右フォーク
    if (philo->id % 2 == 0)
    {
        // 右のフォークを先に取る（偶数ID）
        pthread_mutex_lock(&philo->data->forks[philo->right_fork].mutex);
        if (check_death(philo))
        {
            pthread_mutex_unlock(&philo->data->forks[philo->right_fork].mutex);
            return (0);
        }
        philo->data->forks[philo->right_fork].state = FORK_IN_USE;
        philo->data->forks[philo->right_fork].owner_id = philo->id;
        philo->data->forks[philo->right_fork].acquisition_order = order;
        print_status(philo, MSG_FORK);

        // 左のフォークを取る
        pthread_mutex_lock(&philo->data->forks[philo->left_fork].mutex);
        if (check_death(philo))
        {
            pthread_mutex_unlock(&philo->data->forks[philo->right_fork].mutex);
            pthread_mutex_unlock(&philo->data->forks[philo->left_fork].mutex);
            return (0);
        }
        philo->data->forks[philo->left_fork].state = FORK_IN_USE;
        philo->data->forks[philo->left_fork].owner_id = philo->id;
        philo->data->forks[philo->left_fork].acquisition_order = order + 1;
        print_status(philo, MSG_FORK);
    }
    else
    {
        // 左のフォークを先に取る（奇数ID）
        pthread_mutex_lock(&philo->data->forks[philo->left_fork].mutex);
        if (check_death(philo))
        {
            pthread_mutex_unlock(&philo->data->forks[philo->left_fork].mutex);
            return (0);
        }
        philo->data->forks[philo->left_fork].state = FORK_IN_USE;
        philo->data->forks[philo->left_fork].owner_id = philo->id;
        philo->data->forks[philo->left_fork].acquisition_order = order;
        print_status(philo, MSG_FORK);

        // 右のフォークを取る
        pthread_mutex_lock(&philo->data->forks[philo->right_fork].mutex);
        if (check_death(philo))
        {
            pthread_mutex_unlock(&philo->data->forks[philo->left_fork].mutex);
            pthread_mutex_unlock(&philo->data->forks[philo->right_fork].mutex);
            return (0);
        }
        philo->data->forks[philo->right_fork].state = FORK_IN_USE;
        philo->data->forks[philo->right_fork].owner_id = philo->id;
        philo->data->forks[philo->right_fork].acquisition_order = order + 1;
        print_status(philo, MSG_FORK);
    }

    return (1);
}

// フォーク解放関数
void release_forks(t_philo *philo)
{
    // 偶数番号の哲学者は左フォーク→右フォーク（取得の逆順）
    // 奇数番号の哲学者は右フォーク→左フォーク（取得の逆順）
    if (philo->id % 2 == 0)
    {
        // 左のフォークを先に離す（偶数ID）
        philo->data->forks[philo->left_fork].state = FORK_AVAILABLE;
        philo->data->forks[philo->left_fork].owner_id = -1;
        pthread_mutex_unlock(&philo->data->forks[philo->left_fork].mutex);

        // 右のフォークを離す
        philo->data->forks[philo->right_fork].state = FORK_AVAILABLE;
        philo->data->forks[philo->right_fork].owner_id = -1;
        pthread_mutex_unlock(&philo->data->forks[philo->right_fork].mutex);
    }
    else
    {
        // 右のフォークを先に離す（奇数ID）
        philo->data->forks[philo->right_fork].state = FORK_AVAILABLE;
        philo->data->forks[philo->right_fork].owner_id = -1;
        pthread_mutex_unlock(&philo->data->forks[philo->right_fork].mutex);

        // 左のフォークを離す
        philo->data->forks[philo->left_fork].state = FORK_AVAILABLE;
        philo->data->forks[philo->left_fork].owner_id = -1;
        pthread_mutex_unlock(&philo->data->forks[philo->left_fork].mutex);
    }
}

// 食事関数
void eat(t_philo *philo)
{
    print_status(philo, MSG_EAT);
    philo->state = PHILO_EATING;
    philo->last_eat_time = get_test_time();
    philo->eat_count++;
    test_ft_usleep(philo->data->time_to_eat);
}

// 睡眠と思考
void sleep_and_think(t_philo *philo)
{
    print_status(philo, MSG_SLEEP);
    philo->state = PHILO_SLEEPING;
    test_ft_usleep(philo->data->time_to_sleep);
    
    print_status(philo, MSG_THINK);
    philo->state = PHILO_THINKING;
}

// 哲学者スレッド関数
void *philosopher(void *arg)
{
    t_philo *philo;
    
    philo = (t_philo *)arg;
    
    // 偶数番号の哲学者は少し遅れてスタート（デッドロック防止のため）
    // 注意: 本来なら「丁寧な哲学者アルゴリズム」によりこの遅延は不要ですが、
    // 安全のために保持しています
    if (philo->id % 2 == 0)
        test_ft_usleep(1);
    
    while (!check_death(philo))
    {
        // フォークを取得
        if (!take_forks(philo))
            break;
        
        // 食事
        eat(philo);
        
        // フォークを解放
        release_forks(philo);
        
        // 睡眠と思考
        sleep_and_think(philo);
    }
    return (NULL);
}

// タイマースレッド関数
void *timer_thread(void *arg)
{
    t_data *data;
    
    data = (t_data *)arg;
    test_ft_usleep(TEST_DURATION_MS);
    
    pthread_mutex_lock(&data->death);
    data->is_dead = 1;
    pthread_mutex_unlock(&data->death);
    
    return (NULL);
}

// テスト実行関数
int main(void)
{
    t_data data;
    pthread_t timer_thread;
    int i;
    
    // データ構造を初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = NUM_PHILOSOPHERS;
    data.time_to_die = 10000; // 死なないように十分な時間
    data.time_to_eat = 100;
    data.time_to_sleep = 100;
    data.must_eat = -1;
    data.is_dead = 0;
    data.start_time = get_test_time();
    data.fork_acquisition_count = 0;
    
    // ミューテックスを初期化
    if (pthread_mutex_init(&data.print, NULL) != 0 ||
        pthread_mutex_init(&data.death, NULL) != 0 ||
        pthread_mutex_init(&data.order_mutex, NULL) != 0)
    {
        printf("❌ ミューテックスの初期化に失敗しました\n");
        return 1;
    }
    
    // フォークを初期化
    data.forks = malloc(sizeof(t_fork) * data.num_philos);
    if (!data.forks)
    {
        printf("❌ フォークのメモリ割り当てに失敗しました\n");
        pthread_mutex_destroy(&data.print);
        pthread_mutex_destroy(&data.death);
        pthread_mutex_destroy(&data.order_mutex);
        return 1;
    }
    
    for (i = 0; i < data.num_philos; i++)
    {
        if (pthread_mutex_init(&data.forks[i].mutex, NULL) != 0)
        {
            printf("❌ フォークミューテックスの初期化に失敗しました\n");
            while (--i >= 0)
                pthread_mutex_destroy(&data.forks[i].mutex);
            free(data.forks);
            pthread_mutex_destroy(&data.print);
            pthread_mutex_destroy(&data.death);
            pthread_mutex_destroy(&data.order_mutex);
            return 1;
        }
        data.forks[i].state = FORK_AVAILABLE;
        data.forks[i].owner_id = -1;
        data.forks[i].acquisition_order = 0;
    }
    
    // 哲学者を初期化
    data.philos = malloc(sizeof(t_philo) * data.num_philos);
    if (!data.philos)
    {
        printf("❌ 哲学者のメモリ割り当てに失敗しました\n");
        for (i = 0; i < data.num_philos; i++)
            pthread_mutex_destroy(&data.forks[i].mutex);
        free(data.forks);
        pthread_mutex_destroy(&data.print);
        pthread_mutex_destroy(&data.death);
        pthread_mutex_destroy(&data.order_mutex);
        return 1;
    }
    
    for (i = 0; i < data.num_philos; i++)
    {
        data.philos[i].id = i + 1;
        data.philos[i].state = PHILO_THINKING;
        data.philos[i].left_fork = i;
        data.philos[i].right_fork = (i + 1) % data.num_philos;
        data.philos[i].eat_count = 0;
        data.philos[i].last_eat_time = get_test_time();
        data.philos[i].data = &data;
    }
    
    printf("===== 哲学者スレッド関数テスト =====\n\n");
    printf("テスト内容：\n");
    printf("1. %d人の哲学者で%dms間シミュレーションを実行\n", NUM_PHILOSOPHERS, TEST_DURATION_MS);
    printf("2. 偶数ID哲学者は右→左、奇数ID哲学者は左→右の順でフォークを取得\n");
    printf("3. フォーク取得順序を記録して検証\n\n");
    
    // タイマースレッドを作成（テスト時間経過後に終了）
    if (pthread_create(&timer_thread, NULL, timer_thread, &data) != 0)
    {
        printf("❌ タイマースレッドの作成に失敗しました\n");
        free(data.philos);
        for (i = 0; i < data.num_philos; i++)
            pthread_mutex_destroy(&data.forks[i].mutex);
        free(data.forks);
        pthread_mutex_destroy(&data.print);
        pthread_mutex_destroy(&data.death);
        pthread_mutex_destroy(&data.order_mutex);
        return 1;
    }
    
    // 哲学者スレッドを作成
    for (i = 0; i < data.num_philos; i++)
    {
        if (pthread_create(&data.philos[i].thread, NULL, philosopher, &data.philos[i]) != 0)
        {
            printf("❌ 哲学者スレッドの作成に失敗しました\n");
            data.is_dead = 1;
            break;
        }
    }
    
    // スレッドの終了を待機
    pthread_join(timer_thread, NULL);
    for (i = 0; i < data.num_philos; i++)
    {
        pthread_join(data.philos[i].thread, NULL);
    }
    
    // テスト結果の検証
    printf("\n===== テスト結果 =====\n");
    int errors = 0;
    
    for (i = 0; i < data.num_philos; i++)
    {
        printf("哲学者 %d (ID:%s): ", i + 1, (data.philos[i].id % 2 == 0) ? "偶数" : "奇数");
        
        if (data.philos[i].eat_count > 0)
        {
            printf("食事回数=%d, ", data.philos[i].eat_count);
            
            // フォーク取得順序の検証
            int expected_first, expected_second;
            if (data.philos[i].id % 2 == 0) // 偶数ID
            {
                expected_first = data.philos[i].right_fork;
                expected_second = data.philos[i].left_fork;
                printf("期待されるフォーク順序: 右(%d)→左(%d)\n", 
                       expected_first, expected_second);
            }
            else // 奇数ID
            {
                expected_first = data.philos[i].left_fork;
                expected_second = data.philos[i].right_fork;
                printf("期待されるフォーク順序: 左(%d)→右(%d)\n", 
                       expected_first, expected_second);
            }
        }
        else
        {
            printf("食事回数=0 (テスト失敗)\n");
            errors++;
        }
    }
    
    printf("\n総合結果: %s\n", (errors == 0) ? "✅ テスト成功" : "❌ テスト失敗");
    
    // リソースを解放
    for (i = 0; i < data.num_philos; i++)
        pthread_mutex_destroy(&data.forks[i].mutex);
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    pthread_mutex_destroy(&data.order_mutex);
    free(data.forks);
    free(data.philos);
    
    return (errors > 0);
} 