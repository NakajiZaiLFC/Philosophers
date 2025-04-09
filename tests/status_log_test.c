#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define TEST_STATUS
#ifdef TEST_STATUS
// philo.hをインクルードするとmain関数の宣言と競合するので、必要な定義だけを行う
#define MSG_FORK    "has taken a fork"
#define MSG_EAT     "is eating"
#define MSG_SLEEP   "is sleeping"
#define MSG_THINK   "is thinking"
#define MSG_DIED    "died"

#define PHILO_THINKING 0
#define PHILO_EATING   1
#define PHILO_SLEEPING 2
#define PHILO_DEAD     3

typedef struct s_data t_data;

typedef struct s_philo
{
    int id;
    int state;
    int left_fork;
    int right_fork;
    int eat_count;
    long long last_eat_time;
    pthread_t thread;
    struct s_data *data;
} t_philo;

typedef struct s_data
{
    int num_philos;
    int time_to_die;
    int time_to_eat;
    int time_to_sleep;
    int must_eat;
    int is_dead;
    long long start_time;
    void *forks;
    pthread_mutex_t print;
    pthread_mutex_t death;
    t_philo *philos;
} t_data;

// 外部関数のプロトタイプ
long long get_time(void);
long long time_elapsed(long long start_time);
void print_status(t_philo *philo, char *status);

#else
#include "../philo.h"
#endif

#define NUM_PHILOSOPHERS 5
#define NUM_MESSAGES 3
#define TEST_DURATION_MS 100

/**
 * @brief テスト用のスレッド関数
 * 各哲学者が異なる状態メッセージを出力
 */
void *philosopher_thread(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    int id = philo->id;

    // 少しずらしてスレッド開始
    usleep(id * 10 * 1000);

    // 各状態を出力
    print_status(philo, MSG_FORK);
    usleep(10 * 1000);
    
    print_status(philo, MSG_EAT);
    usleep(10 * 1000);
    
    print_status(philo, MSG_SLEEP);
    usleep(10 * 1000);
    
    print_status(philo, MSG_THINK);
    
    return NULL;
}

/**
 * @brief 死亡状態をテストするスレッド関数
 */
void *death_test_thread(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    // 少し待機してから死亡状態を設定
    usleep(80 * 1000);
    
    pthread_mutex_lock(&philo->data->death);
    philo->data->is_dead = 1;
    pthread_mutex_unlock(&philo->data->death);
    
    // 死亡メッセージを出力
    print_status(philo, MSG_DIED);
    
    return NULL;
}

#ifdef TEST_STATUS
// テスト用のmain関数
int main(void)
{
    t_data data;
    pthread_t threads[NUM_PHILOSOPHERS];
    pthread_t death_thread;
    int i;
    
    // データ構造を初期化
    memset(&data, 0, sizeof(t_data));
    data.num_philos = NUM_PHILOSOPHERS;
    data.time_to_die = 1000;
    data.time_to_eat = 100;
    data.time_to_sleep = 100;
    data.must_eat = -1;
    data.is_dead = 0;
    data.start_time = get_time();
    
    // ミューテックスを初期化
    if (pthread_mutex_init(&data.print, NULL) != 0 ||
        pthread_mutex_init(&data.death, NULL) != 0)
    {
        printf("❌ ミューテックスの初期化に失敗しました\n");
        return 1;
    }
    
    // 哲学者データを初期化
    data.philos = malloc(sizeof(t_philo) * NUM_PHILOSOPHERS);
    if (!data.philos)
    {
        printf("❌ メモリ割り当てに失敗しました\n");
        pthread_mutex_destroy(&data.print);
        pthread_mutex_destroy(&data.death);
        return 1;
    }
    
    // 各哲学者を初期化
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        data.philos[i].id = i + 1;
        data.philos[i].state = PHILO_THINKING;
        data.philos[i].eat_count = 0;
        data.philos[i].last_eat_time = get_time();
        data.philos[i].data = &data;
    }
    
    printf("===== ステータスロギング機能テスト =====\n\n");
    printf("複数スレッドから同時にメッセージを出力するテスト\n");
    printf("出力形式: [タイムスタンプ(ms)] [哲学者ID] [状態メッセージ]\n\n");
    
    // 哲学者スレッドを作成
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        if (pthread_create(&threads[i], NULL, philosopher_thread, &data.philos[i]) != 0)
        {
            printf("❌ スレッド作成に失敗しました\n");
            free(data.philos);
            pthread_mutex_destroy(&data.print);
            pthread_mutex_destroy(&data.death);
            return 1;
        }
    }
    
    // 死亡テストスレッドを作成
    if (pthread_create(&death_thread, NULL, death_test_thread, &data.philos[0]) != 0)
    {
        printf("❌ 死亡テストスレッドの作成に失敗しました\n");
    }
    
    // スレッドの終了を待機
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_join(death_thread, NULL);
    
    // メッセージ出力後のテスト
    printf("\n死亡後のメッセージ出力テスト\n");
    printf("以下のメッセージは表示されないはずです（MSG_DIEDを除く）\n");
    
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        print_status(&data.philos[i], MSG_THINK);
    }
    
    // 死亡メッセージは表示されるはず
    print_status(&data.philos[0], MSG_DIED);
    
    // リソースを解放
    free(data.philos);
    pthread_mutex_destroy(&data.print);
    pthread_mutex_destroy(&data.death);
    
    printf("\n===== テスト完了 =====\n");
    printf("出力結果を確認し、以下を検証してください：\n");
    printf("1. タイムスタンプが正しく増加しているか\n");
    printf("2. 出力が混在していないか（スレッドセーフか）\n");
    printf("3. 死亡状態設定後は死亡メッセージのみ表示されるか\n");
    
    return 0;
}
#endif 