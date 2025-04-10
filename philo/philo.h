/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/11 02:34:42 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <signal.h>
# include <string.h>

// #ifdef DEBUG
// void debug_print(const char *fmt, ...);
// #define DEBUG_PRINT debug_print
// #define DEBUG_INIT(data) debug_init(data)
// #else
// #define DEBUG_PRINT(fmt, ...)
// #define DEBUG_INIT(data)
// #endif

/* 哲学者の状態 */
# define PHILO_THINKING 0
# define PHILO_EATING 1
# define PHILO_SLEEPING 2
# define PHILO_DEAD 3

/* フォークの状態 */
# define FORK_AVAILABLE 0
# define FORK_IN_USE 1

/* シミュレーション状態 */
# define SIM_RUNNING 0
# define SIM_STOPPED 1
# define SIM_ERROR 2
# define SIM_COMPLETED 3

/* ステータスメッセージ */
# define MSG_FORK "has taken a fork"
# define MSG_EAT "is eating"
# define MSG_SLEEP "is sleeping"
# define MSG_THINK "is thinking"
# define MSG_DIED "died"
# define MSG_COMPLETE "All philosophers have eaten enough meals"

/* リソースタイプ定義 */
# define RESOURCE_MUTEX 1
# define RESOURCE_MEMORY 2
# define RESOURCE_THREAD 3

typedef struct s_fork
{
	pthread_mutex_t	mutex;
	int				state;
	int				owner_id;
}	t_fork;

typedef struct s_philo
{
	int				id;
	int				state;
	int				left_fork;
	int				right_fork;
	int				eat_count;
	long long		last_eat_time;
	pthread_t		thread;
	struct s_data	*data;
}	t_philo;

typedef struct s_data
{
	int				num_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				must_eat;
	int				is_dead;
	int				sim_state;
	int				single_philo;
	long long		start_time;
	t_fork			*forks;
	pthread_mutex_t	print;
	pthread_mutex_t	death;
	pthread_mutex_t	start_lock;
	pthread_mutex_t	meal_lock;
	pthread_t		monitor_thread;
	t_philo			*philos;
}	t_data;

/* utils.c */
long long	get_time(void);
void		ft_usleep(long long time);
long long	time_elapsed(long long start_time);
int			ft_atoi(const char *str);

/* init.c */
int			init_data(t_data *data, int argc, char **argv);
int			init_philos(t_data *data);
int			init_forks(t_data *data);
int			init_mutex(t_data *data);

// #ifdef DEBUG
// void debug_init(t_data *data);
// #endif

/* philo.c */
int			main(int argc, char **argv);
void		*philo_routine(void *arg);
void		*monitor_routine(void *arg);
void		monitor_philos(t_data *data);
void		print_status(t_philo *philo, char *status);

/* philo_utils.c */
int			set_simulation_state(t_data *data, int state);
int			get_simulation_state(t_data *data);
int			check_all_ate(t_data *data);
void		handle_termination(t_data *data);
void		handle_meal_completion(t_data *data);

/* cleanup.c */
int			init_resource_inventory(void);
int			register_resource(void *ptr, int type, char *description);
int			unregister_resource(void *ptr);
int			register_mutex(pthread_mutex_t *mutex, char *description);
int			register_memory(void *ptr, char *description);

/* cleanup_utils.c */
int			register_thread(pthread_t thread, char *description);
void		print_resource_inventory(void);
int			cleanup_single_mutex(pthread_mutex_t *mutex);
int			cleanup_multiple_mutexes(pthread_mutex_t *mutexes, int count);
int			cleanup_forks(t_fork *forks, int count);

/* cleanup_extra.c */
int			cleanup_thread(pthread_t thread, int should_join);
int			cleanup_threads(pthread_t *threads, int count, int should_join);
int			cleanup_memory(void *ptr);
int			cleanup_all_resources_by_type(int type);
int			cleanup_all_resources(void);

/* resource_mgmt.c */
int			emergency_cleanup(t_data *data);
int			free_resources(t_data *data);
int			verify_cleanup(t_data *data);
void		safe_exit(t_data *data, int exit_code);

/* threading.c */
int			is_state(t_data *data, int target_state);
int			is_dead(t_data *data);
int			update_meal_count(t_philo *philo);
int			bidirectional_lock(pthread_mutex_t *m1, pthread_mutex_t *m2);
void		bidirectional_unlock(pthread_mutex_t *m1, pthread_mutex_t *m2);

/* fork_utils.c */
int			can_take_fork(t_philo *philo, int fork_index);
int			take_fork_safe(t_philo *philo, int fork_index);
int			is_time_to_die(t_philo *philo);
long long	time_since_last_meal(t_philo *philo);
int			check_and_take_both_forks(t_philo *philo);

/* fork_extra.c */
int			check_and_take_both_forks_safe(t_philo *philo);
void		release_both_forks(t_philo *philo);

#endif