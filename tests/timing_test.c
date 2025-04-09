#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

#ifndef TEST_ONLY
#include "../philo.h"
#else
// テスト専用のプロトタイプ定義
long long	get_time(void);
void		ft_usleep(long long time);
long long	time_elapsed(long long start_time);
#endif

/**
 * @brief get_time関数のテスト
 * 連続して呼び出すと増加する値を返すことを確認
 * 
 * @return bool テスト成功の場合true
 */
static bool test_get_time()
{
    long long time1, time2;
    
    time1 = get_time();
    if (time1 < 0) {
        printf("❌ get_time: 負の値を返しました: %lld\n", time1);
        return false;
    }
    
    // 少し待機
    usleep(5000); // 5ms
    
    time2 = get_time();
    if (time2 < time1) {
        printf("❌ get_time: 2回目の値が1回目より小さいです: %lld < %lld\n", time2, time1);
        return false;
    }
    
    printf("✅ get_time: テスト成功 (%lld → %lld)\n", time1, time2);
    return true;
}

/**
 * @brief time_elapsed関数のテスト
 * 指定された時間経過後に正しい値を返すことを確認
 * 
 * @return bool テスト成功の場合true
 */
static bool test_time_elapsed()
{
    long long start_time, elapsed;
    int wait_time_ms = 50; // 50ms
    
    start_time = get_time();
    
    // 指定時間待機
    usleep(wait_time_ms * 1000);
    
    elapsed = time_elapsed(start_time);
    
    // 許容誤差範囲（±5ms）
    if (elapsed < wait_time_ms - 5 || elapsed > wait_time_ms + 5) {
        printf("❌ time_elapsed: 予期された時間と実際の経過時間が大きく異なります: 期待値 %dms, 実際 %lldms\n", 
               wait_time_ms, elapsed);
        return false;
    }
    
    printf("✅ time_elapsed: テスト成功（予期値: %dms, 実際: %lldms, 誤差: %.1fms）\n", 
           wait_time_ms, elapsed, (double)(elapsed - wait_time_ms));
    return true;
}

/**
 * @brief ft_usleep関数のテスト
 * 指定された時間だけ待機することを確認
 * 
 * @return bool テスト成功の場合true
 */
static bool test_ft_usleep()
{
    long long start_time, elapsed;
    int tests_passed = 0;
    int total_tests = 3;
    int wait_times[] = {10, 50, 100}; // テストする待機時間（ms）
    
    for (int i = 0; i < total_tests; i++) {
        int wait_time = wait_times[i];
        start_time = get_time();
        
        ft_usleep(wait_time);
        
        elapsed = time_elapsed(start_time);
        
        // 許容誤差範囲（待機時間の10%または5ms、どちらか大きい方）
        double tolerance = wait_time * 0.1 > 5 ? wait_time * 0.1 : 5;
        
        if (elapsed < wait_time - tolerance || elapsed > wait_time + tolerance) {
            printf("❌ ft_usleep(%d): 予期された時間と実際の経過時間が大きく異なります: 期待値 %dms, 実際 %lldms\n", 
                   wait_time, wait_time, elapsed);
        } else {
            printf("✅ ft_usleep(%d): テスト成功（予期値: %dms, 実際: %lldms, 誤差: %.1fms）\n", 
                   wait_time, wait_time, elapsed, (double)(elapsed - wait_time));
            tests_passed++;
        }
    }
    
    return tests_passed == total_tests;
}

/**
 * @brief スリープ精度のベンチマーク
 * 標準のusleep関数とft_usleep関数を比較
 */
static void benchmark_sleep_precision()
{
    long long start_time, elapsed1, elapsed2;
    int wait_time = 100; // 100ms
    
    printf("\n===== スリープ精度ベンチマーク =====\n");
    
    // 標準usleepのテスト
    start_time = get_time();
    usleep(wait_time * 1000);
    elapsed1 = time_elapsed(start_time);
    
    // ft_usleepのテスト
    start_time = get_time();
    ft_usleep(wait_time);
    elapsed2 = time_elapsed(start_time);
    
    printf("標準usleep(%dms): 実際 %.2fms (誤差: %.2fms)\n", 
           wait_time, (double)elapsed1, (double)(elapsed1 - wait_time));
    printf("ft_usleep(%dms): 実際 %.2fms (誤差: %.2fms)\n", 
           wait_time, (double)elapsed2, (double)(elapsed2 - wait_time));
    
    if (llabs(elapsed2 - wait_time) < llabs(elapsed1 - wait_time)) {
        printf("結果: ft_usleepの方が精度が高いです\n");
    } else {
        printf("結果: 標準usleepの方が精度が高いです\n");
    }
}

int run_timing_tests(void)
{
    int tests_passed = 0;
    int total_tests = 3;
    
    printf("===== タイミング機能単体テスト =====\n\n");
    
    if (test_get_time())
        tests_passed++;
    
    if (test_time_elapsed())
        tests_passed++;
    
    if (test_ft_usleep())
        tests_passed++;
    
    // ベンチマーク実行
    benchmark_sleep_precision();
    
    printf("\n===== テスト結果サマリー =====\n");
    printf("%d/%d テスト成功\n", tests_passed, total_tests);
    
    return tests_passed == total_tests ? 0 : 1;
}

#ifndef TEST_MAIN
#define TEST_MAIN
int main(void)
{
    return run_timing_tests();
}
#endif 