/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nassy <nassy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 12:00:00 by AI Assistan       #+#    #+#             */
/*   Updated: 2025/04/14 10:08:09 by nassy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

# define PHILO_THINKING 0
# define PHILO_EATING 1
# define PHILO_SLEEPING 2
# define PHILO_DEAD 3

# define FORK_AVAILABLE 0
# define FORK_IN_USE 1

# define SIM_RUNNING 0
# define SIM_STOPPED 1
# define SIM_ERROR 2
# define SIM_COMPLETED 3

# define MSG_FORK "has taken a fork"
# define MSG_EAT "is eating"
# define MSG_SLEEP "is sleeping"
# define MSG_THINK "is thinking"
# define MSG_DIED "died"
# define MSG_COMPLETE "All philosophers have eaten enough meals"

# define RESOURCE_MUTEX 1
# define RESOURCE_MEMORY 2
# define RESOURCE_THREAD 3

# define ERR_CREATE_MONITOR "Error: failed to create monitor thread"
# define ERR_CREATE_THREAD "Error: failed to create thread"
# define ERR_WRONG_ARG "Error: wrong number of arguments"
# define ERR_DATA_INIT "Error: initialization failed"
# define ERR_PHILO_INIT "Error: philosopher initialization failed"

typedef struct s_fork
{
	pthread_mutex_t	mutex;
	int				state;
	int				owner_id;
}					t_fork;

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
}					t_philo;

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
}					t_data;

int					free_resources(t_data *data);
int					cleanup_single_mutex(pthread_mutex_t *mutex);
long long			get_time(void);
int					ft_atoi(const char *str);
long long			time_elapsed(long long start_time);
void				ft_usleep(long long time);
int					is_state(t_data *data, int target_state);
void				release_both_forks(t_philo *philo);
int					is_dead(t_data *data);
int					eat(t_philo *philo);
int					check_and_take_both_forks_safe(t_philo *philo);
int					check_all_ate(t_data *data);
void				handle_meal_completion(t_data *data);
void				philo_think(t_philo *philo);
void				adjust_think_time(t_philo *philo);
int					can_take_fork(t_philo *philo, int fork_index);
int					take_fork_safe(t_philo *philo, int fork_index);
int					is_time_to_die(t_philo *philo);
long long			time_since_last_meal(t_philo *philo);
int					check_and_take_both_forks(t_philo *philo);
void				handle_termination(t_data *data);
int					init_data(t_data *data, int argc, char **argv);
int					init_mutex(t_data *data);
int					init_forks(t_data *data);
int					init_philos(t_data *data);
void				*monitor_routine(void *arg);
int					handle_thread_creation_error(t_data *data, char *err_msg);
int					get_simulation_state(t_data *data);
void				*philo_routine(void *arg);
void				sleep_and_think(t_philo *philo);
int					try_get_forks(t_philo *philo);
void				print_status(t_philo *philo, char *status);
int					check_death(t_philo *philo);
int					set_simulation_state(t_data *data, int state);
void				philo_sleep(t_philo *philo);
int					cleanup_forks(t_fork *forks, int count);
int					validate_args(int argc, char **argv);
int					parse_args(t_data *data, int argc, char **argv);
#endif