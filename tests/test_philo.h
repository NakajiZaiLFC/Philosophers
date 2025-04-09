#ifndef TEST_PHILO_H
#define TEST_PHILO_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdbool.h>

/* 哲学者の状態 */
#define PHILO_THINKING 0
#define PHILO_EATING 1
#define PHILO_SLEEPING 2
#define PHILO_DEAD 3

/* フォークの状態 */
#define FORK_AVAILABLE 0
#define FORK_IN_USE 1

/* シミュレーション状態 */
#define SIM_RUNNING 0
#define SIM_STOPPED 1
#define SIM_ERROR 2
#define SIM_COMPLETED 3

/* ステータスメッセージ */
#define MSG_FORK "has taken a fork"
#define MSG_EAT "is eating"
#define MSG_SLEEP "is sleeping"
#define MSG_THINK "is thinking"
#define MSG_DIED "died"
#define MSG_COMPLETE "All philosophers have eaten enough meals"

typedef struct s_fork
{
	pthread_mutex_t mutex;
	int state;
	int owner_id;
} t_fork;

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
	int sim_state;
	int single_philo;
	long long start_time;
	t_fork *forks;
	pthread_mutex_t print;
	pthread_mutex_t death;
	pthread_mutex_t start_lock;
	pthread_mutex_t meal_lock;
	pthread_t monitor_thread;
	t_philo *philos;
} t_data;

// utils.c
long long get_time(void);
void ft_usleep(long long time);
long long time_elapsed(long long start_time);

// threading.c - スレッドセーフティ最適化用関数
int is_state(t_data *data, int target_state);
int is_dead(t_data *data);
int update_meal_count(t_philo *philo);
int bidirectional_lock(pthread_mutex_t *m1, pthread_mutex_t *m2);
void bidirectional_unlock(pthread_mutex_t *m1, pthread_mutex_t *m2);
int can_take_fork(t_philo *philo, int fork_index);
int take_fork_safe(t_philo *philo, int fork_index);
int is_time_to_die(t_philo *philo);
long long time_since_last_meal(t_philo *philo);

// テスト用スタブ関数
void print_status(t_philo *philo, char *status);
int set_simulation_state(t_data *data, int state);
int cleanup_single_mutex(pthread_mutex_t *mutex);
int init_resource_inventory(void);

#endif