#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// タイムスタンプ取得関数
long long get_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

// ミューテックス
pthread_mutex_t print_mutex;

// メッセージ定義
#define MSG_FORK    "has taken a fork"
#define MSG_EAT     "is eating"
#define MSG_SLEEP   "is sleeping"
#define MSG_THINK   "is thinking"
#define MSG_DIED    "died"

// グローバル変数
int is_dead = 0;
long long start_time;

// スレッドセーフなプリント関数
void print_status(int id, char *status)
{
    long long time;

    pthread_mutex_lock(&print_mutex);
    if (!is_dead || !strcmp(status, MSG_DIED))
    {
        time = get_timestamp() - start_time;
        printf("%lld %d %s\n", time, id, status);
    }
    pthread_mutex_unlock(&print_mutex);
}

// テスト用スレッド関数
void *test_thread(void *arg)
{
    int id = *(int*)arg;
    
    // スレッド開始タイミングをずらす
    usleep(id * 10000);
    
    // 各種ステータスを出力
    print_status(id, MSG_FORK);
    usleep(10000);
    print_status(id, MSG_EAT);
    usleep(10000);
    print_status(id, MSG_SLEEP);
    usleep(10000);
    print_status(id, MSG_THINK);
    
    return NULL;
}

// 死亡テスト用スレッド関数
void *death_thread(void *arg)
{
    int id = *(int*)arg;
    
    // 少し待機してから死亡フラグを設定
    usleep(60000);
    is_dead = 1;
    print_status(id, MSG_DIED);
    
    return NULL;
}

int main(void)
{
    pthread_t threads[5];
    pthread_t d_thread;
    int ids[5] = {1, 2, 3, 4, 5};
    
    // 初期化
    if (pthread_mutex_init(&print_mutex, NULL) != 0)
    {
        printf("❌ ミューテックス初期化に失敗しました\n");
        return 1;
    }
    
    start_time = get_timestamp();
    
    printf("===== ステータスロギング機能テスト =====\n\n");
    printf("複数スレッドから同時にメッセージを出力するテスト\n");
    printf("出力形式: [タイムスタンプ(ms)] [ID] [状態メッセージ]\n\n");
    
    // テストスレッド作成
    for (int i = 0; i < 5; i++)
    {
        if (pthread_create(&threads[i], NULL, test_thread, &ids[i]) != 0)
        {
            printf("❌ スレッド作成に失敗しました\n");
            pthread_mutex_destroy(&print_mutex);
            return 1;
        }
    }
    
    // 死亡テストスレッド作成
    if (pthread_create(&d_thread, NULL, death_thread, &ids[0]) != 0)
    {
        printf("❌ 死亡テストスレッド作成に失敗しました\n");
    }
    
    // スレッド終了待ち
    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_join(d_thread, NULL);
    
    // 死亡後のメッセージテスト
    printf("\n死亡後のメッセージ出力テスト\n");
    printf("以下のメッセージは表示されないはずです（MSG_DIEDを除く）\n");
    
    for (int i = 0; i < 5; i++)
    {
        print_status(ids[i], MSG_THINK);
    }
    
    // 死亡メッセージは表示されるはず
    print_status(ids[0], MSG_DIED);
    
    // 後片付け
    pthread_mutex_destroy(&print_mutex);
    
    printf("\n===== テスト完了 =====\n");
    printf("出力結果を確認し、以下を検証してください：\n");
    printf("1. タイムスタンプが正しく増加しているか\n");
    printf("2. 出力が混在していないか（スレッドセーフか）\n");
    printf("3. 死亡状態設定後は死亡メッセージのみ表示されるか\n");
    
    return 0;
} 