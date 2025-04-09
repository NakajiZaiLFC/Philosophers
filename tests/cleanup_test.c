/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup_test.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/07 15:30:42 by claude            #+#    #+#             */
/*   Updated: 2025/04/09 22:43:17 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo.h"
#include <assert.h>

#define TEST_RESOURCES 10
#define TEST_THREADS 5

// テスト用スタブ関数
void print_status(t_philo *philo, char *status)
{
    printf("STUB: Philo %d: %s\n", philo->id, status);
}

// テスト結果構造体
typedef struct s_test_result
{
    int success;
    int failure;
    char message[256];
} t_test_result;

// テスト用のスレッド関数
void *test_thread_func(void *arg)
{
    int *value = (int *)arg;
    (*value)++;
    return NULL;
}

// テスト関数: リソースインベントリの初期化と解放
t_test_result test_resource_inventory_init()
{
    t_test_result result = {0, 0, ""};

    // 初期化テスト
    if (init_resource_inventory() == 0)
    {
        result.success++;
        strcpy(result.message, "リソースインベントリの初期化が成功しました");
    }
    else
    {
        result.failure++;
        strcpy(result.message, "リソースインベントリの初期化に失敗しました");
        return result;
    }

    // 二重初期化テスト
    if (init_resource_inventory() == 0)
    {
        result.success++;
        strcat(result.message, "\n二重初期化の処理が正しく行われました");
    }
    else
    {
        result.failure++;
        strcat(result.message, "\n二重初期化の処理に失敗しました");
    }

    return result;
}

// テスト関数: ミューテックスリソースの登録と解放
t_test_result test_mutex_resource()
{
    t_test_result result = {0, 0, ""};
    pthread_mutex_t mutexes[TEST_RESOURCES];
    int i;
    char desc[32];

    // ミューテックスの初期化と登録
    for (i = 0; i < TEST_RESOURCES; i++)
    {
        if (pthread_mutex_init(&mutexes[i], NULL) != 0)
        {
            result.failure++;
            sprintf(result.message, "ミューテックス %d の初期化に失敗しました", i);
            return result;
        }

        sprintf(desc, "test mutex %d", i);
        if (register_mutex(&mutexes[i], desc) == 0)
        {
            result.success++;
        }
        else
        {
            result.failure++;
            sprintf(result.message, "ミューテックス %d の登録に失敗しました", i);
            // クリーンアップして終了
            while (i >= 0)
            {
                pthread_mutex_destroy(&mutexes[i]);
                i--;
            }
            return result;
        }
    }

    // ミューテックスのクリーンアップ
    for (i = 0; i < TEST_RESOURCES; i++)
    {
        if (cleanup_single_mutex(&mutexes[i]) == 0)
        {
            result.success++;
        }
        else
        {
            result.failure++;
            sprintf(result.message, "%s\nミューテックス %d のクリーンアップに失敗しました",
                    result.message, i);
        }
    }

    if (result.failure == 0)
    {
        strcpy(result.message, "すべてのミューテックスリソースの登録と解放が成功しました");
    }

    return result;
}

// テスト関数: メモリリソースの登録と解放
t_test_result test_memory_resource()
{
    t_test_result result = {0, 0, ""};
    void *memory_blocks[TEST_RESOURCES];
    int i;
    char desc[32];

    // メモリ割り当てと登録
    for (i = 0; i < TEST_RESOURCES; i++)
    {
        memory_blocks[i] = malloc(1024);
        if (memory_blocks[i] == NULL)
        {
            result.failure++;
            sprintf(result.message, "メモリブロック %d の割り当てに失敗しました", i);
            // クリーンアップして終了
            while (--i >= 0)
            {
                free(memory_blocks[i]);
            }
            return result;
        }

        sprintf(desc, "test memory %d", i);
        if (register_memory(memory_blocks[i], desc) == 0)
        {
            result.success++;
        }
        else
        {
            result.failure++;
            sprintf(result.message, "メモリブロック %d の登録に失敗しました", i);
            free(memory_blocks[i]);
            // クリーンアップして終了
            while (--i >= 0)
            {
                free(memory_blocks[i]);
            }
            return result;
        }
    }

    // インベントリ状態の表示（デバッグ用）
    print_resource_inventory();

    // メモリの解放と登録解除
    for (i = 0; i < TEST_RESOURCES; i++)
    {
        if (unregister_resource(memory_blocks[i]) == 0)
        {
            result.success++;
            free(memory_blocks[i]);
        }
        else
        {
            result.failure++;
            sprintf(result.message, "%s\nメモリブロック %d の登録解除に失敗しました",
                    result.message, i);
            free(memory_blocks[i]);
        }
    }

    if (result.failure == 0)
    {
        strcpy(result.message, "すべてのメモリリソースの登録と解放が成功しました");
    }

    return result;
}

// テスト関数: スレッドリソースの登録と解放
t_test_result test_thread_resource()
{
    t_test_result result = {0, 0, ""};
    pthread_t threads[TEST_THREADS];
    int thread_values[TEST_THREADS] = {0};
    int i;
    char desc[32];

    // スレッド作成と登録
    for (i = 0; i < TEST_THREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, test_thread_func, &thread_values[i]) != 0)
        {
            result.failure++;
            sprintf(result.message, "スレッド %d の作成に失敗しました", i);
            return result;
        }

        sprintf(desc, "test thread %d", i);
        if (register_thread(threads[i], desc) == 0)
        {
            result.success++;
        }
        else
        {
            result.failure++;
            sprintf(result.message, "スレッド %d の登録に失敗しました", i);
            return result;
        }
    }

    // スレッドの終了を待機と登録解除
    for (i = 0; i < TEST_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        if (unregister_resource((void *)(unsigned long)threads[i]) == 0)
        {
            result.success++;

            // スレッド関数が実行されたことを確認
            if (thread_values[i] == 1)
            {
                result.success++;
            }
            else
            {
                result.failure++;
                sprintf(result.message, "%s\nスレッド %d の実行が確認できませんでした",
                        result.message, i);
            }
        }
        else
        {
            result.failure++;
            sprintf(result.message, "%s\nスレッド %d の登録解除に失敗しました",
                    result.message, i);
        }
    }

    if (result.failure == 0)
    {
        strcpy(result.message, "すべてのスレッドリソースの登録と解放が成功しました");
    }

    return result;
}

// テスト実行関数
void run_test(char *test_name, t_test_result (*test_func)())
{
    t_test_result result;

    printf("\n==== %s の実行 ====\n", test_name);
    result = test_func();

    printf("成功: %d, 失敗: %d\n", result.success, result.failure);
    printf("メッセージ: %s\n", result.message);

    if (result.failure > 0)
    {
        printf("テスト結果: ❌ 失敗\n");
    }
    else
    {
        printf("テスト結果: ✅ 成功\n");
    }

    printf("=====================\n");

    // テスト失敗時はプログラムを終了
    assert(result.failure == 0);
}

int main(int argc, char **argv)
{
    (void)argc; // 未使用の引数を回避
    (void)argv; // 未使用の引数を回避

    printf("=== リソースクリーンアップテストの開始 ===\n");

    // 初期化テスト
    run_test("リソースインベントリ初期化テスト", test_resource_inventory_init);

    // ミューテックスリソーステスト
    run_test("ミューテックスリソーステスト", test_mutex_resource);

    // メモリリソーステスト
    run_test("メモリリソーステスト", test_memory_resource);

    // スレッドリソーステスト
    run_test("スレッドリソーステスト", test_thread_resource);

    printf("\n=== すべてのテストが正常に完了しました ===\n");

    return 0;
}