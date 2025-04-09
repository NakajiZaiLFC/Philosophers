#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <sys/time.h>
#include <unistd.h>

/**
 * 現在時刻をミリ秒単位で取得する
 */
static inline long long get_test_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/**
 * 経過時間を計算する
 */
static inline long long test_time_elapsed(long long start_time)
{
	return (get_test_time() - start_time);
}

/**
 * マイクロ秒単位でスリープする
 */
static inline void test_ft_usleep(long long time)
{
	long long start;
	long long elapsed;

	start = get_test_time();
	while (1)
	{
		elapsed = get_test_time() - start;
		if (elapsed >= time)
			break;
		usleep(100);
	}
}

#endif /* TEST_UTILS_H */