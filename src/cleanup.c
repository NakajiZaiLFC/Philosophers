/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/05 10:30:42 by claude            #+#    #+#             */
/*   Updated: 2025/04/10 15:45:14 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

// リソースタイプの定義
#define RESOURCE_MUTEX 1
#define RESOURCE_MEMORY 2
#define RESOURCE_THREAD 3

// 最大追跡リソース数
#define MAX_RESOURCES 100

// リソースエントリ構造体 - 個々のリソースを追跡
typedef struct s_resource_entry
{
	void *ptr;				 // リソースへのポインタ
	int resource_type;		 // リソースの種類
	long long creation_time; // 作成タイムスタンプ
	pthread_t owner_thread;	 // 所有者スレッド
	char description[32];	 // リソースの説明
	int is_active;			 // アクティブなリソースフラグ
} t_resource_entry;

// リソースインベントリの状態を保持する構造体
typedef struct s_resource_inventory
{
	t_resource_entry resources[MAX_RESOURCES]; // リソースエントリの配列
	int resource_count;						   // 登録されたリソースの総数
	pthread_mutex_t inventory_mutex;		   // インベントリアクセス用ミューテックス
	int is_initialized;						   // 初期化フラグ
} t_resource_inventory;

// グローバルリソースインベントリ
static t_resource_inventory g_inventory = {{{0}}, 0, PTHREAD_MUTEX_INITIALIZER, 0};

/**
 * リソースインベントリを初期化する
 *
 * @return 0: 成功, -1: 失敗
 */
int init_resource_inventory(void)
{
	if (g_inventory.is_initialized)
		return (0);

	if (pthread_mutex_init(&g_inventory.inventory_mutex, NULL) != 0)
		return (-1);

	g_inventory.resource_count = 0;
	memset(g_inventory.resources, 0, sizeof(g_inventory.resources));
	g_inventory.is_initialized = 1;
	DEBUG_PRINT("リソースインベントリが初期化されました");
	return (0);
}

/**
 * 新しいリソースをインベントリに追加する
 *
 * @param ptr リソースへのポインタ
 * @param type リソースの種類（RESOURCE_MUTEX, RESOURCE_MEMORY, RESOURCE_THREAD）
 * @param description リソースの説明
 * @return 0: 成功, -1: 失敗
 */
int register_resource(void *ptr, int type, char *description)
{
	int index;

	if (!ptr || !g_inventory.is_initialized)
		return (-1);

	pthread_mutex_lock(&g_inventory.inventory_mutex);

	// 既に登録済みのリソースかチェック
	for (index = 0; index < g_inventory.resource_count; index++)
	{
		if (g_inventory.resources[index].ptr == ptr &&
			g_inventory.resources[index].is_active)
		{
			pthread_mutex_unlock(&g_inventory.inventory_mutex);
			return (-1); // 既に登録済み
		}
	}

	// 空きスロットを探す
	index = 0;
	while (index < MAX_RESOURCES)
	{
		if (!g_inventory.resources[index].is_active)
			break;
		index++;
	}

	// 空きスロットがない場合
	if (index >= MAX_RESOURCES)
	{
		pthread_mutex_unlock(&g_inventory.inventory_mutex);
		DEBUG_PRINT("リソースインベントリがいっぱいです");
		return (-1);
	}

	// 新しいリソースを登録
	g_inventory.resources[index].ptr = ptr;
	g_inventory.resources[index].resource_type = type;
	g_inventory.resources[index].creation_time = get_time();
	g_inventory.resources[index].owner_thread = pthread_self();
	strncpy(g_inventory.resources[index].description, description, 31);
	g_inventory.resources[index].description[31] = '\0';
	g_inventory.resources[index].is_active = 1;

	// アクティブなリソース数を更新
	if (index >= g_inventory.resource_count)
		g_inventory.resource_count = index + 1;

	pthread_mutex_unlock(&g_inventory.inventory_mutex);
	DEBUG_PRINT("リソース登録: %s", description);
	return (0);
}

/**
 * リソースをインベントリから削除する
 *
 * @param ptr 削除するリソースへのポインタ
 * @return 0: 成功, -1: 失敗
 */
int unregister_resource(void *ptr)
{
	int i;
	int result;

	if (!ptr || !g_inventory.is_initialized)
		return (-1);

	result = -1;
	pthread_mutex_lock(&g_inventory.inventory_mutex);

	// リソースを検索
	for (i = 0; i < g_inventory.resource_count; i++)
	{
		if (g_inventory.resources[i].ptr == ptr &&
			g_inventory.resources[i].is_active)
		{
			// リソースを非アクティブに設定
			g_inventory.resources[i].is_active = 0;
			DEBUG_PRINT("リソース解放: %s", g_inventory.resources[i].description);
			result = 0;
			break;
		}
	}

	pthread_mutex_unlock(&g_inventory.inventory_mutex);
	return (result);
}

/**
 * ミューテックスをリソースインベントリに登録する
 *
 * @param mutex 登録するミューテックス
 * @param description ミューテックスの説明
 * @return 0: 成功, -1: 失敗
 */
int register_mutex(pthread_mutex_t *mutex, char *description)
{
	return register_resource((void *)mutex, RESOURCE_MUTEX, description);
}

/**
 * メモリリソースをインベントリに登録する
 *
 * @param ptr 登録するメモリブロックへのポインタ
 * @param description メモリの説明
 * @return 0: 成功, -1: 失敗
 */
int register_memory(void *ptr, char *description)
{
	return register_resource(ptr, RESOURCE_MEMORY, description);
}

/**
 * スレッドをリソースインベントリに登録する
 *
 * @param thread 登録するスレッド
 * @param description スレッドの説明
 * @return 0: 成功, -1: 失敗
 */
int register_thread(pthread_t thread, char *description)
{
	// pthread_tは実際にはポインタではないかもしれないので変換に注意
	return register_resource((void *)(unsigned long)thread, RESOURCE_THREAD, description);
}

/**
 * 単一のミューテックスを安全にクリーンアップする
 *
 * @param mutex クリーンアップするミューテックス
 * @return 0: 成功, -1: 失敗
 */
int cleanup_single_mutex(pthread_mutex_t *mutex)
{
	int result;

	if (!mutex)
		return (-1);

	result = pthread_mutex_destroy(mutex);
	if (result != 0)
	{
		// エラーログを記録（デバッグ時のみ）
		DEBUG_PRINT("Error destroying mutex: %d", result);
		return (-1);
	}

	// リソースインベントリから削除
	unregister_resource((void *)mutex);
	return (0);
}

/**
 * 複数のミューテックスを安全にクリーンアップする
 *
 * @param mutexes ミューテックスの配列
 * @param count ミューテックスの数
 * @return 成功したクリーンアップの数
 */
int cleanup_multiple_mutexes(pthread_mutex_t *mutexes, int count)
{
	int i;
	int success_count;

	if (!mutexes || count <= 0)
		return (0);

	i = 0;
	success_count = 0;
	while (i < count)
	{
		if (cleanup_single_mutex(&mutexes[i]) == 0)
			success_count++;
		i++;
	}
	return (success_count);
}

/**
 * フォーク（ミューテックス）を安全にクリーンアップする
 *
 * @param forks フォークの配列
 * @param count フォークの数
 * @return 成功したクリーンアップの数
 */
int cleanup_forks(t_fork *forks, int count)
{
	int i;
	int success_count;

	if (!forks || count <= 0)
		return (0);

	i = 0;
	success_count = 0;
	while (i < count)
	{
		// ロックが保持されている場合は解放
		// pthread_mutex_unlock(&forks[i].mutex);

		if (cleanup_single_mutex(&forks[i].mutex) == 0)
			success_count++;
		i++;
	}
	return (success_count);
}

/**
 * マスタークリーンアップ関数 - すべてのリソースを適切な順序でクリーンアップする
 *
 * @return 0: 成功, -1: 失敗
 */
int cleanup_all_resources()
{
	if (!g_inventory.is_initialized)
		return (0);

	DEBUG_PRINT("マスタークリーンアップの開始");

	// まず、スレッドをクリーンアップ（既にjoin済みとみなす）
	DEBUG_PRINT("%d個のスレッドリソースをクリーンアップしました",
				cleanup_all_resources_by_type(RESOURCE_THREAD));

	// 次に、ミューテックスをクリーンアップ
	DEBUG_PRINT("%d個のミューテックスリソースをクリーンアップしました",
				cleanup_all_resources_by_type(RESOURCE_MUTEX));

	// 最後に、メモリをクリーンアップ
	DEBUG_PRINT("%d個のメモリリソースをクリーンアップしました",
				cleanup_all_resources_by_type(RESOURCE_MEMORY));

	// リソースインベントリのミューテックスを解放
	pthread_mutex_destroy(&g_inventory.inventory_mutex);
	g_inventory.is_initialized = 0;

	DEBUG_PRINT("マスタークリーンアップが完了しました");
	return (0);
}

/**
 * データ構造体のリソースを安全に解放する
 *
 * @param data クリーンアップするデータ構造体
 * @return 0: 成功, -1: 失敗
 */
int free_resources(t_data *data)
{
	if (!data)
		return (-1);

	// まず、すべてのスレッドが終了していることを確認（handle_terminationですでに実行されている）

	// フォークのミューテックスをクリーンアップ
	if (data->forks)
	{
		cleanup_forks(data->forks, data->num_philos);
		cleanup_memory(data->forks);
		data->forks = NULL;
	}

	// 共有ミューテックスをクリーンアップ
	cleanup_single_mutex(&data->print);
	cleanup_single_mutex(&data->death);
	cleanup_single_mutex(&data->start_lock);
	cleanup_single_mutex(&data->meal_lock);

	// メモリを解放
	if (data->philos)
	{
		cleanup_memory(data->philos);
		data->philos = NULL;
	}

	// リソースインベントリの状態を表示（デバッグモードの場合）
	print_resource_inventory();

	// マスタークリーンアップ関数を呼び出す
	cleanup_all_resources();

	return (0);
}

/**
 * 緊急時のクリーンアップを実行する
 * シグナルハンドラなどから呼び出されることを想定
 *
 * @param data クリーンアップするデータ構造体（NULLの場合はグローバルリソースのみクリーンアップ）
 * @return 0: 成功, -1: 失敗
 */
int emergency_cleanup(t_data *data)
{
	if (data)
	{
		// データ構造体が存在する場合はリソースを解放
		free_resources(data);
	}
	else
	{
		// データ構造体がない場合はグローバルリソースのみ解放
		cleanup_all_resources();
	}

	return (0);
}

/**
 * リソースインベントリの状態を表示する（デバッグ用）
 */
void print_resource_inventory(void)
{
#ifdef DEBUG
	int i;
	int active_count = 0;

	if (!g_inventory.is_initialized)
	{
		DEBUG_PRINT("リソースインベントリは初期化されていません");
		return;
	}

	pthread_mutex_lock(&g_inventory.inventory_mutex);

	DEBUG_PRINT("===== リソースインベントリ状態 =====");
	for (i = 0; i < g_inventory.resource_count; i++)
	{
		if (g_inventory.resources[i].is_active)
		{
			active_count++;
			DEBUG_PRINT("[%d] タイプ: %d, 説明: %s, 作成時間: %lld",
						i,
						g_inventory.resources[i].resource_type,
						g_inventory.resources[i].description,
						g_inventory.resources[i].creation_time);
		}
	}

	DEBUG_PRINT("アクティブなリソース数: %d/%d", active_count, g_inventory.resource_count);
	DEBUG_PRINT("=================================");

	pthread_mutex_unlock(&g_inventory.inventory_mutex);
#endif
}

/**
 * クリーンアップ状態を検証する
 *
 * @param data 検証するデータ構造体
 * @return 0: すべてクリーン, -1: 未解放リソースあり
 */
int verify_cleanup(t_data *data)
{
	int result;
	int i;
	int active_resources = 0;

	result = 0;

	// データ構造体の検証
	if (!data)
		return (-1);

	// フォークの検証
	if (data->forks != NULL)
	{
		DEBUG_PRINT("Forks not properly freed");
		result = -1;
	}

	// 哲学者の検証
	if (data->philos != NULL)
	{
		DEBUG_PRINT("Philosophers not properly freed");
		result = -1;
	}

	// リソースインベントリの検証
	if (g_inventory.is_initialized)
	{
		pthread_mutex_lock(&g_inventory.inventory_mutex);

		// アクティブなリソースをカウント
		for (i = 0; i < g_inventory.resource_count; i++)
		{
			if (g_inventory.resources[i].is_active)
				active_resources++;
		}

		pthread_mutex_unlock(&g_inventory.inventory_mutex);

		if (active_resources > 0)
		{
			DEBUG_PRINT("未解放のリソースが %d 個あります", active_resources);
			result = -1;
		}
	}

	return (result);
}

/**
 * スレッドリソースをクリーンアップする
 *
 * @param thread クリーンアップするスレッド
 * @param should_join スレッドをjoinすべきかどうか
 * @return 0: 成功, -1: 失敗
 */
int cleanup_thread(pthread_t thread, int should_join)
{
	int result = 0;
	void *thread_result;

	// スレッドをjoinする必要がある場合
	if (should_join)
	{
		result = pthread_join(thread, &thread_result);
		if (result != 0)
		{
			DEBUG_PRINT("Error joining thread: %d", result);
			return (-1);
		}
	}

	// リソースインベントリから削除
	unregister_resource((void *)(unsigned long)thread);
	return (0);
}

/**
 * 複数のスレッドリソースをクリーンアップする
 *
 * @param threads クリーンアップするスレッドの配列
 * @param count スレッドの数
 * @param should_join スレッドをjoinすべきかどうか
 * @return 正常にクリーンアップされたスレッドの数
 */
int cleanup_threads(pthread_t *threads, int count, int should_join)
{
	int i;
	int success_count;

	if (!threads || count <= 0)
		return (0);

	i = 0;
	success_count = 0;
	while (i < count)
	{
		if (cleanup_thread(threads[i], should_join) == 0)
			success_count++;
		i++;
	}
	return (success_count);
}

/**
 * メモリリソースをクリーンアップする
 *
 * @param ptr 解放するメモリブロックへのポインタ
 * @return 0: 成功, -1: 失敗
 */
int cleanup_memory(void *ptr)
{
	if (!ptr)
		return (-1);

	// メモリを解放
	free(ptr);

	// リソースインベントリから削除
	unregister_resource(ptr);
	return (0);
}

/**
 * 指定されたリソースタイプのすべてのリソースをクリーンアップする
 *
 * @param type クリーンアップするリソースのタイプ
 * @return クリーンアップされたリソースの数
 */
int cleanup_all_resources_by_type(int type)
{
	int i;
	int cleaned = 0;

	if (!g_inventory.is_initialized)
		return (0);

	pthread_mutex_lock(&g_inventory.inventory_mutex);

	for (i = 0; i < g_inventory.resource_count; i++)
	{
		if (g_inventory.resources[i].is_active &&
			g_inventory.resources[i].resource_type == type)
		{
			switch (type)
			{
			case RESOURCE_MUTEX:
				if (cleanup_single_mutex((pthread_mutex_t *)g_inventory.resources[i].ptr) == 0)
					cleaned++;
				break;
			case RESOURCE_MEMORY:
				if (cleanup_memory(g_inventory.resources[i].ptr) == 0)
					cleaned++;
				break;
			case RESOURCE_THREAD:
				// スレッドの場合はjoinを行わない（別途joinが必要）
				g_inventory.resources[i].is_active = 0;
				cleaned++;
				break;
			default:
				break;
			}
		}
	}

	pthread_mutex_unlock(&g_inventory.inventory_mutex);
	return (cleaned);
}