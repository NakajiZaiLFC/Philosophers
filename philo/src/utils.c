#include "philo.h"

/**
 * @brief 現在の時間をミリ秒単位で取得する
 *
 * @return long long 現在の時間（ミリ秒）
 */
long long get_time(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/**
 * @brief 指定された時間（ミリ秒）だけ待機する精密なスリープ関数
 * より正確な待機のために短いインターバルで繰り返しチェックを行う
 *
 * @param time 待機する時間（ミリ秒）
 */
void ft_usleep(long long time)
{
	long long start;
	long long elapsed;
	long long remaining;

	start = get_time();
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= time)
			break;

		// 残り時間を計算
		remaining = time - elapsed;

		// 待機戦略を最適化
		// 「should live」テストケースのための特別対応
		if (remaining > 50)
			usleep(500); // 0.5msスリープ（長い残り時間）
		else if (remaining > 10)
			usleep(200); // 0.2msスリープ（中程度の残り時間）
		else
			usleep(50); // 0.05msスリープ（短い残り時間、より精密に）
	}
}

/**
 * @brief 指定された開始時間からの経過時間をミリ秒単位で計算する
 *
 * @param start_time 開始時間（ミリ秒）
 * @return long long 経過時間（ミリ秒）
 */
long long time_elapsed(long long start_time)
{
	return (get_time() - start_time);
}

int ft_atoi(const char *str)
{
	int i;
	int sign;
	int result;

	i = 0;
	sign = 1;
	result = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result * sign);
}