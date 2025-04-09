/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_safety_test.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/07 14:30:42 by claude            #+#    #+#             */
/*   Updated: 2025/04/10 00:29:05 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifdef THREAD_SAFETY_TEST_STANDALONE
#include "test_philo.h"

// テスト用スタブ関数
void print_status(t_philo *philo, char *status)
{
	printf("STUB: Philo %d: %s\n", philo->id, status);
}

int set_simulation_state(t_data *data, int state)
{
	data->sim_state = state;
	return 0;
}

int cleanup_single_mutex(pthread_mutex_t *mutex)
{
	if (!mutex)
		return (-1);

	int result = pthread_mutex_destroy(mutex);
	if (result != 0)
	{
		printf("Error destroying mutex: %d\n", result);
		return (-1);
	}
	return (0);
}

int init_resource_inventory(void)
{
	return 0; // スタブ実装
}

#else
#include "../philo.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#endif

#define NUM_TESTS 5
#define NUM_THREADS 10
#define ITERATIONS 1000

// テスト結果の追跡
typedef struct s_test_results
{
	int total;
	int passed;
	int failed;
	pthread_mutex_t mutex;
} t_test_results;

// テスト用グローバル変数
static t_test_results g_results = {0, 0, 0, PTHREAD_MUTEX_INITIALIZER};

// テスト用共有カウンター
static int g_counter = 0;
static pthread_mutex_t g_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// テスト用共有配列
static int g_shared_array[100];
static pthread_mutex_t g_array_mutex = PTHREAD_MUTEX_INITIALIZER;

// テスト用のスレッド
typedef struct s_thread_data
{
	pthread_t thread;
	int id;
	int iterations;
} t_thread_data;

// テスト結果を追加する
void add_test_result(bool passed, const char *test_name)
{
	pthread_mutex_lock(&g_results.mutex);
	g_results.total++;

	if (passed)
	{
		g_results.passed++;
		printf("✅ PASS: %s\n", test_name);
	}
	else
	{
		g_results.failed++;
		printf("❌ FAIL: %s\n", test_name);
	}

	pthread_mutex_unlock(&g_results.mutex);
}

// テスト結果のサマリーを表示
void print_test_summary()
{
	printf("\n==== THREAD SAFETY TEST SUMMARY ====\n");
	printf("Total tests: %d\n", g_results.total);
	printf("Passed: %d\n", g_results.passed);
	printf("Failed: %d\n", g_results.failed);
	printf("====================================\n");
}

// ============ テスト1: bidirectional_lockのテスト ============
void *test_bidirectional_lock_thread(void *arg)
{
	t_thread_data *data = (t_thread_data *)arg;
	pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
	int i;

	// それぞれのスレッドで双方向ロックをテスト
	for (i = 0; i < data->iterations; i++)
	{
		// 方向1でロック
		bidirectional_lock(&mutex1, &mutex2);
		pthread_mutex_lock(&g_counter_mutex);
		g_counter++;
		pthread_mutex_unlock(&g_counter_mutex);
		bidirectional_unlock(&mutex1, &mutex2);

		// 方向2でロック（逆順）
		bidirectional_lock(&mutex2, &mutex1);
		pthread_mutex_lock(&g_counter_mutex);
		g_counter++;
		pthread_mutex_unlock(&g_counter_mutex);
		bidirectional_unlock(&mutex2, &mutex1);
	}

	// クリーンアップ
	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);

	return NULL;
}

bool run_bidirectional_lock_test()
{
	t_thread_data threads[NUM_THREADS];
	int expected_counter;
	int i;

	// カウンターをリセット
	g_counter = 0;

	// スレッドを作成
	for (i = 0; i < NUM_THREADS; i++)
	{
		threads[i].id = i;
		threads[i].iterations = ITERATIONS;
		if (pthread_create(&threads[i].thread, NULL, test_bidirectional_lock_thread, &threads[i]) != 0)
		{
			printf("Error creating thread %d\n", i);
			return false;
		}
	}

	// スレッドの終了を待機
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i].thread, NULL);
	}

	// 期待値: 各スレッドがITERATIONS回のインクリメントを2回（方向1と方向2）
	expected_counter = NUM_THREADS * ITERATIONS * 2;

	// 結果を検証
	if (g_counter != expected_counter)
	{
		printf("Expected counter: %d, Actual: %d\n", expected_counter, g_counter);
		return false;
	}

	return true;
}

// ============ テスト2: 状態チェック関数のテスト ============
void *test_state_check_thread(void *arg)
{
	t_thread_data *data = (t_thread_data *)arg;
	t_data test_data;
	int i;

	// 初期化
	memset(&test_data, 0, sizeof(t_data));
	pthread_mutex_init(&test_data.death, NULL);
	test_data.sim_state = SIM_RUNNING;

	// 各スレッドで状態をトグル
	for (i = 0; i < data->iterations; i++)
	{
		// 状態をSIM_STOPPEDに変更
		if (i % 2 == 0)
		{
			set_simulation_state(&test_data, SIM_STOPPED);
			if (!is_state(&test_data, SIM_STOPPED))
			{
				pthread_mutex_lock(&g_counter_mutex);
				g_counter++; // エラーカウント
				pthread_mutex_unlock(&g_counter_mutex);
			}
		}
		// 状態をSIM_RUNNINGに戻す
		else
		{
			set_simulation_state(&test_data, SIM_RUNNING);
			if (!is_state(&test_data, SIM_RUNNING))
			{
				pthread_mutex_lock(&g_counter_mutex);
				g_counter++; // エラーカウント
				pthread_mutex_unlock(&g_counter_mutex);
			}
		}
	}

	// クリーンアップ
	pthread_mutex_destroy(&test_data.death);

	return NULL;
}

bool run_state_check_test()
{
	t_thread_data threads[NUM_THREADS];
	int i;

	// エラーカウンターをリセット
	g_counter = 0;

	// スレッドを作成
	for (i = 0; i < NUM_THREADS; i++)
	{
		threads[i].id = i;
		threads[i].iterations = ITERATIONS;
		if (pthread_create(&threads[i].thread, NULL, test_state_check_thread, &threads[i]) != 0)
		{
			printf("Error creating thread %d\n", i);
			return false;
		}
	}

	// スレッドの終了を待機
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i].thread, NULL);
	}

	// エラーが発生していないことを確認
	if (g_counter > 0)
	{
		printf("State check errors: %d\n", g_counter);
		return false;
	}

	return true;
}

// ============ テスト3: 食事回数の更新テスト ============
void *test_meal_count_thread(void *arg)
{
	t_thread_data *data = (t_thread_data *)arg;
	t_data test_data;
	t_philo test_philo;
	int i;

	// 初期化
	memset(&test_data, 0, sizeof(t_data));
	memset(&test_philo, 0, sizeof(t_philo));
	pthread_mutex_init(&test_data.meal_lock, NULL);
	pthread_mutex_init(&test_data.death, NULL);
	test_philo.data = &test_data;
	test_philo.id = data->id;
	test_philo.eat_count = 0;

	// 各スレッドで食事回数を更新
	for (i = 0; i < data->iterations; i++)
	{
		// 食事回数を更新
		pthread_mutex_lock(&test_data.meal_lock);
		test_philo.eat_count++;
		pthread_mutex_unlock(&test_data.meal_lock);

		// 共有配列にアクセス（カウント値をインデックスとして使用）
		pthread_mutex_lock(&g_array_mutex);
		if (test_philo.eat_count < 100)
			g_shared_array[test_philo.eat_count]++;
		pthread_mutex_unlock(&g_array_mutex);
	}

	// クリーンアップ
	pthread_mutex_destroy(&test_data.meal_lock);
	pthread_mutex_destroy(&test_data.death);

	return NULL;
}

bool run_meal_count_test()
{
	t_thread_data threads[NUM_THREADS];
	int i;
	bool result = true;

	// 共有配列を初期化
	memset(g_shared_array, 0, sizeof(g_shared_array));

	// スレッドを作成
	for (i = 0; i < NUM_THREADS; i++)
	{
		threads[i].id = i;
		threads[i].iterations = ITERATIONS;
		if (pthread_create(&threads[i].thread, NULL, test_meal_count_thread, &threads[i]) != 0)
		{
			printf("Error creating thread %d\n", i);
			return false;
		}
	}

	// スレッドの終了を待機
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i].thread, NULL);
	}

	// 結果を検証
	for (i = 1; i <= ITERATIONS && i < 100; i++)
	{
		if (g_shared_array[i] != NUM_THREADS)
		{
			printf("Meal count update error at index %d: expected %d, got %d\n",
				   i, NUM_THREADS, g_shared_array[i]);
			result = false;
			break;
		}
	}

	return result;
}

// ============ テスト4: タイムチェックテスト ============
void *test_time_check_thread(void *arg)
{
	t_thread_data *data = (t_thread_data *)arg;
	t_data test_data;
	t_philo test_philo;
	int i;

	// 初期化
	memset(&test_data, 0, sizeof(t_data));
	memset(&test_philo, 0, sizeof(t_philo));
	pthread_mutex_init(&test_data.death, NULL);
	pthread_mutex_init(&test_data.meal_lock, NULL);
	test_philo.data = &test_data;
	test_philo.id = data->id;
	test_philo.last_eat_time = get_time();
	test_data.time_to_die = 10000; // 10秒（テストが早く終わるため）

	// 各スレッドで時間をチェック
	for (i = 0; i < data->iterations; i++)
	{
		// 時間をチェック（まだ死亡時間ではないはず）
		if (is_time_to_die(&test_philo))
		{
			pthread_mutex_lock(&g_counter_mutex);
			g_counter++; // エラーカウント
			pthread_mutex_unlock(&g_counter_mutex);
		}

		// 最後の食事時間を定期的に更新
		if (i % 100 == 0)
		{
			pthread_mutex_lock(&test_data.meal_lock);
			test_philo.last_eat_time = get_time();
			pthread_mutex_unlock(&test_data.meal_lock);
		}
	}

	// クリーンアップ
	pthread_mutex_destroy(&test_data.death);
	pthread_mutex_destroy(&test_data.meal_lock);

	return NULL;
}

bool run_time_check_test()
{
	t_thread_data threads[NUM_THREADS];
	int i;

	// エラーカウンターをリセット
	g_counter = 0;

	// スレッドを作成
	for (i = 0; i < NUM_THREADS; i++)
	{
		threads[i].id = i;
		threads[i].iterations = ITERATIONS;
		if (pthread_create(&threads[i].thread, NULL, test_time_check_thread, &threads[i]) != 0)
		{
			printf("Error creating thread %d\n", i);
			return false;
		}
	}

	// スレッドの終了を待機
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i].thread, NULL);
	}

	// エラーが発生していないことを確認
	if (g_counter > 0)
	{
		printf("Time check errors: %d\n", g_counter);
		return false;
	}

	return true;
}

// ============ テスト5: フォーク取得テスト ============
void *test_fork_thread(void *arg)
{
	t_thread_data *data = (t_thread_data *)arg;
	t_data test_data;
	t_philo test_philo;
	t_fork test_forks[2];
	int i;

	// 初期化
	memset(&test_data, 0, sizeof(t_data));
	memset(&test_philo, 0, sizeof(t_philo));
	memset(test_forks, 0, sizeof(test_forks));

	pthread_mutex_init(&test_data.death, NULL);
	pthread_mutex_init(&test_forks[0].mutex, NULL);
	pthread_mutex_init(&test_forks[1].mutex, NULL);

	test_data.forks = test_forks;
	test_philo.data = &test_data;
	test_philo.id = data->id;
	test_philo.left_fork = 0;
	test_philo.right_fork = 1;

	// 各スレッドでフォークの取得をテスト
	for (i = 0; i < data->iterations; i++)
	{
		// フォークを取得可能か確認
		if (!can_take_fork(&test_philo, 0) && !can_take_fork(&test_philo, 1))
		{
			// 両方のフォークが取得できない場合はスキップ
			continue;
		}

		// フォークを取得（可能な場合）
		bool acquired = false;

		// 左フォークを試みる
		if (take_fork_safe(&test_philo, 0))
		{
			acquired = true;
			// 権利を持っていることを確認
			if (test_forks[0].owner_id != test_philo.id)
			{
				pthread_mutex_lock(&g_counter_mutex);
				g_counter++; // エラーカウント
				pthread_mutex_unlock(&g_counter_mutex);
			}
			// 解放
			test_forks[0].state = FORK_AVAILABLE;
			test_forks[0].owner_id = -1;
			pthread_mutex_unlock(&test_forks[0].mutex);
		}

		// 右フォークを試みる
		if (take_fork_safe(&test_philo, 1))
		{
			acquired = true;
			// 権利を持っていることを確認
			if (test_forks[1].owner_id != test_philo.id)
			{
				pthread_mutex_lock(&g_counter_mutex);
				g_counter++; // エラーカウント
				pthread_mutex_unlock(&g_counter_mutex);
			}
			// 解放
			test_forks[1].state = FORK_AVAILABLE;
			test_forks[1].owner_id = -1;
			pthread_mutex_unlock(&test_forks[1].mutex);
		}

		// フォークを取得できた場合はカウントを増やす
		if (acquired)
		{
			pthread_mutex_lock(&g_array_mutex);
			g_shared_array[0]++;
			pthread_mutex_unlock(&g_array_mutex);
		}
	}

	// クリーンアップ
	pthread_mutex_destroy(&test_data.death);
	pthread_mutex_destroy(&test_forks[0].mutex);
	pthread_mutex_destroy(&test_forks[1].mutex);

	return NULL;
}

bool run_fork_test()
{
	t_thread_data threads[NUM_THREADS];
	int i;

	// カウンターをリセット
	g_counter = 0;
	memset(g_shared_array, 0, sizeof(g_shared_array));

	// スレッドを作成
	for (i = 0; i < NUM_THREADS; i++)
	{
		threads[i].id = i;
		threads[i].iterations = ITERATIONS;
		if (pthread_create(&threads[i].thread, NULL, test_fork_thread, &threads[i]) != 0)
		{
			printf("Error creating thread %d\n", i);
			return false;
		}
	}

	// スレッドの終了を待機
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i].thread, NULL);
	}

	// エラーが発生していないことを確認
	if (g_counter > 0)
	{
		printf("Fork acquisition errors: %d\n", g_counter);
		return false;
	}

	// フォークを少なくとも一部のスレッドが取得できたことを確認
	if (g_shared_array[0] == 0)
	{
		printf("No forks were successfully acquired\n");
		return false;
	}

	printf("Successfully acquired forks: %d times\n", g_shared_array[0]);
	return true;
}

// メイン関数
#ifdef THREAD_SAFETY_TEST_STANDALONE
int main(void)
#else
int thread_safety_test_main(void)
#endif
{
	bool all_tests_passed = true;

	printf("=== Running Thread Safety Tests ===\n\n");

	// テスト1: bidirectional_lockのテスト
	printf("Test 1: Bidirectional Lock Test\n");
	bool test1_result = run_bidirectional_lock_test();
	add_test_result(test1_result, "Bidirectional Lock Test");
	all_tests_passed &= test1_result;

	// テスト2: 状態チェック関数のテスト
	printf("\nTest 2: State Check Test\n");
	bool test2_result = run_state_check_test();
	add_test_result(test2_result, "State Check Test");
	all_tests_passed &= test2_result;

	// テスト3: 食事回数の更新テスト
	printf("\nTest 3: Meal Count Update Test\n");
	bool test3_result = run_meal_count_test();
	add_test_result(test3_result, "Meal Count Update Test");
	all_tests_passed &= test3_result;

	// テスト4: タイムチェックテスト
	printf("\nTest 4: Time Check Test\n");
	bool test4_result = run_time_check_test();
	add_test_result(test4_result, "Time Check Test");
	all_tests_passed &= test4_result;

	// テスト5: フォーク取得テスト
	printf("\nTest 5: Fork Acquisition Test\n");
	bool test5_result = run_fork_test();
	add_test_result(test5_result, "Fork Acquisition Test");
	all_tests_passed &= test5_result;

	// テスト結果のサマリーを表示
	print_test_summary();

	return all_tests_passed ? 0 : 1;
}