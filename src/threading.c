/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threading.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/07 11:30:42 by claude            #+#    #+#             */
/*   Updated: 2025/04/10 20:55:21 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * スレッドセーフな状態チェック関数
 * thread-safe state checker
 *
 * @param data データ構造体
 * @param target_state 確認したい状態
 * @return 1: 状態一致, 0: 状態不一致
 */
int is_state(t_data *data, int target_state)
{
    int result;

    pthread_mutex_lock(&data->death);
    result = (data->sim_state == target_state);
    pthread_mutex_unlock(&data->death);

    return result;
}

/**
 * スレッドセーフな死亡フラグチェック関数
 * thread-safe death flag checker
 *
 * @param data データ構造体
 * @return 1: 死亡, 0: 生存中
 */
int is_dead(t_data *data)
{
    int result;

    pthread_mutex_lock(&data->death);
    result = data->is_dead;
    pthread_mutex_unlock(&data->death);

    return result;
}

/**
 * スレッドセーフな食事回数更新関数
 * thread-safe meal count updater
 *
 * @param philo 哲学者構造体
 * @return 1: 更新成功, 0: 更新失敗
 */
int update_meal_count(t_philo *philo)
{
    int result = 1;

    pthread_mutex_lock(&philo->data->meal_lock);
    if (!is_dead(philo->data))
    {
        philo->eat_count++;
        philo->last_eat_time = get_time();
    }
    else
        result = 0;
    pthread_mutex_unlock(&philo->data->meal_lock);

    return result;
}

/**
 * 双方向ロック関数 - 低番号のロックを先に取得することで順序を一貫させる
 * bidirectional lock function - always acquires locks in consistent order
 *
 * @param m1 最初のミューテックス
 * @param m2 2番目のミューテックス
 * @return 1: ロック成功, 0: ロック失敗（キャンセルされた場合）
 */
int bidirectional_lock(pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    // ポインタアドレスの比較で一貫したロック順序を保証
    // guarantee consistent lock ordering by comparing pointer addresses
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    if (m1 < m2)
    {
        first = m1;
        second = m2;
    }
    else
    {
        first = m2;
        second = m1;
    }

    pthread_mutex_lock(first);
    pthread_mutex_lock(second);

    return 1;
}

/**
 * 双方向アンロック関数 - 高番号のロックを先に解放
 * bidirectional unlock function - always releases locks in reverse order
 *
 * @param m1 最初のミューテックス
 * @param m2 2番目のミューテックス
 */
void bidirectional_unlock(pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    // ロックの解放は獲得の逆順
    // release locks in reverse order of acquisition
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    if (m1 < m2)
    {
        first = m1;
        second = m2;
    }
    else
    {
        first = m2;
        second = m1;
    }

    // 逆順で解放
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
}

/**
 * スレッドセーフなフォーク取得確認関数
 * thread-safe fork acquisition checker
 *
 * @param philo 哲学者構造体
 * @param fork_index フォークのインデックス
 * @return 1: 取得可能, 0: 取得不可
 */
int can_take_fork(t_philo *philo, int fork_index)
{
    t_fork *fork;
    int result;

    fork = &philo->data->forks[fork_index];
    pthread_mutex_lock(&fork->mutex);
    result = (fork->state == FORK_AVAILABLE);
    pthread_mutex_unlock(&fork->mutex);

    return result;
}

/**
 * スレッドセーフなフォーク取得関数
 * thread-safe fork acquisition function
 *
 * @param philo 哲学者構造体
 * @param fork_index フォークのインデックス
 * @return 1: 取得成功, 0: 取得失敗
 */
int take_fork_safe(t_philo *philo, int fork_index)
{
    t_fork *fork;
    int result = 0;

    fork = &philo->data->forks[fork_index];
    pthread_mutex_lock(&fork->mutex);
    if (fork->state == FORK_AVAILABLE && !is_dead(philo->data))
    {
        fork->state = FORK_IN_USE;
        fork->owner_id = philo->id;
        print_status(philo, MSG_FORK);
        result = 1;
    }
    // 失敗した場合はミューテックスを即座に解放、成功した場合は呼び出し元で解放すること
    if (!result)
        pthread_mutex_unlock(&fork->mutex);

    return result;
}

/**
 * スレッドセーフな時間チェック関数
 * thread-safe time check function
 *
 * @param philo 哲学者構造体
 * @return 1: 時間切れ, 0: まだ時間内
 */
int is_time_to_die(t_philo *philo)
{
    int result;

    pthread_mutex_lock(&philo->data->death);
    result = (time_elapsed(philo->last_eat_time) > philo->data->time_to_die);
    pthread_mutex_unlock(&philo->data->death);

    return result;
}

/**
 * スレッドセーフな最後の食事時間確認関数
 * thread-safe last meal time checker
 *
 * @param philo 哲学者構造体
 * @return 最後の食事からの経過時間（ミリ秒）
 */
long long time_since_last_meal(t_philo *philo)
{
    long long result;

    pthread_mutex_lock(&philo->data->meal_lock);
    result = time_elapsed(philo->last_eat_time);
    pthread_mutex_unlock(&philo->data->meal_lock);

    return result;
}

/**
 * 両方のフォークを原子的に確認して取得する関数（データレース修正版）
 * Check and take both forks atomically, with race condition fixes
 *
 * @param philo 哲学者構造体
 * @return 1: 両方のフォーク取得成功, 0: 取得失敗
 */
int check_and_take_both_forks_safe(t_philo *philo)
{
    int left_fork = philo->left_fork;
    int right_fork = philo->right_fork;
    t_fork *left = &philo->data->forks[left_fork];
    t_fork *right = &philo->data->forks[right_fork];
    int success = 0;

    // まず死亡状態をチェック（最初のロックを取得する前に）
    if (is_dead(philo->data))
        return 0;

    // 優先順位処理（思考状態の哲学者が優先）
    int is_thinking_priority = (philo->state == PHILO_THINKING &&
                                philo->data->num_philos % 2 == 1);

    // 低番号のフォークから取得（デッドロック防止）
    pthread_mutex_t *first_mutex = (left_fork < right_fork) ? &left->mutex : &right->mutex;
    pthread_mutex_t *second_mutex = (left_fork < right_fork) ? &right->mutex : &left->mutex;
    t_fork *first_fork = (left_fork < right_fork) ? left : right;
    t_fork *second_fork = (left_fork < right_fork) ? right : left;

    // 最初のミューテックスをロック
    pthread_mutex_lock(first_mutex);

    // 最初のフォークが利用可能かチェック
    if (first_fork->state == FORK_AVAILABLE)
    {
        // ロック取得前に死亡状態を再チェック
        if (is_dead(philo->data))
        {
            pthread_mutex_unlock(first_mutex);
            return 0;
        }

        // 最初のフォークを取得（まだprint_statusは呼ばない）
        first_fork->state = FORK_IN_USE;
        first_fork->owner_id = philo->id;

        // 2番目のミューテックスをロック
        pthread_mutex_lock(second_mutex);

        // 2番目のフォークが利用可能かチェック
        if (second_fork->state == FORK_AVAILABLE)
        {
            // 死亡状態の最終チェック
            if (is_dead(philo->data))
            {
                // 死亡状態なら取得したフォークを解放
                first_fork->state = FORK_AVAILABLE;
                first_fork->owner_id = -1;
                pthread_mutex_unlock(second_mutex);
                pthread_mutex_unlock(first_mutex);
                return 0;
            }

            // 両方のフォークを取得完了
            second_fork->state = FORK_IN_USE;
            second_fork->owner_id = philo->id;

            // 安全にフォークを取得した後でprintする
            print_status(philo, MSG_FORK); // 最初のフォーク
            print_status(philo, MSG_FORK); // 2番目のフォーク

            success = 1;
        }
        else if (is_thinking_priority && second_fork->state == FORK_IN_USE)
        {
            // 思考状態の場合の優先処理：一時的に解放して再試行
            first_fork->state = FORK_AVAILABLE;
            first_fork->owner_id = -1;
            pthread_mutex_unlock(second_mutex);
            pthread_mutex_unlock(first_mutex);

            // 短い待機
            ft_usleep(1);

            // 再度試行（再帰呼び出し）
            return check_and_take_both_forks_safe(philo);
        }
        else
        {
            // 取得失敗：最初のフォークを解放
            first_fork->state = FORK_AVAILABLE;
            first_fork->owner_id = -1;
            pthread_mutex_unlock(second_mutex);
            pthread_mutex_unlock(first_mutex);
            return 0;
        }
    }
    else
    {
        // 最初のフォークが利用できない
        pthread_mutex_unlock(first_mutex);
        return 0;
    }

    return success;
}

/**
 * 両方のフォークを解放する関数
 * Release both forks
 *
 * @param philo 哲学者構造体
 */
void release_both_forks(t_philo *philo)
{
    int left_fork = philo->left_fork;
    int right_fork = philo->right_fork;
    t_fork *left = &philo->data->forks[left_fork];
    t_fork *right = &philo->data->forks[right_fork];

    // 低番号のフォークから取得した場合は高番号から解放
    pthread_mutex_t *first_mutex = (left_fork < right_fork) ? &right->mutex : &left->mutex;
    pthread_mutex_t *second_mutex = (left_fork < right_fork) ? &left->mutex : &right->mutex;
    t_fork *first_fork = (left_fork < right_fork) ? right : left;
    t_fork *second_fork = (left_fork < right_fork) ? left : right;

    // 最初のフォークを解放
    first_fork->state = FORK_AVAILABLE;
    first_fork->owner_id = -1;
    pthread_mutex_unlock(first_mutex);

    // 2番目のフォークを解放
    second_fork->state = FORK_AVAILABLE;
    second_fork->owner_id = -1;
    pthread_mutex_unlock(second_mutex);
}