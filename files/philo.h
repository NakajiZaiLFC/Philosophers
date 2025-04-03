/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI                                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by AI               #+#    #+#             */
/*   Updated: 2023/01/01 00:00:00 by AI              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <string.h>
# include <limits.h>

/* Return codes */
# define SUCCESS 0
# define ERROR 1

/* Argument validation error codes */
# define ERROR_NOT_NUMBER 1
# define ERROR_NEGATIVE_OR_ZERO 2
# define ERROR_TOO_LARGE 3
# define ERROR_PHILO_COUNT 4

/* Initialization error codes */
# define ERROR_MALLOC_FORKS 10
# define ERROR_MALLOC_PHILOS 11
# define ERROR_MUTEX_INIT 12
# define ERROR_THREAD_CREATE 13

/* Boolean values */
# define TRUE 1
# define FALSE 0

/* Status messages */
# define FORK_MSG "has taken a fork"
# define EAT_MSG "is eating"
# define SLEEP_MSG "is sleeping"
# define THINK_MSG "is thinking"
# define DIED_MSG "died"

/* Program limits */
# define MIN_PHILOSOPHERS 1
# define MAX_PHILOSOPHERS 200

/* Simulation state */
# define STATE_RUNNING 0
# define STATE_FINISHED 1
# define STATE_ERROR 2

/* New error codes */
# define ERROR_INVALID_ARGS 6
# define ERROR_SYSTEM_CALL 7

typedef struct s_philosopher
{
	int				id;
	int				meals_eaten;
	int				left_fork_id;
	int				right_fork_id;
	long long		last_meal_time;
	pthread_t		thread;
	int				is_full;
	struct s_data	*data;
}				t_philosopher;

typedef struct s_data
{
	int				nbr_of_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				nbr_times_to_eat;
	int				all_ate;
	int				anyone_died;
	int				simulation_state;
	long long		start_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	writing;
	pthread_mutex_t	meal_check;
	pthread_mutex_t	death_check;
	pthread_mutex_t	state_check;
	t_philosopher	*philosophers;
}				t_data;

/* init.c */
int			init_data(t_data *data, int argc, char **argv);
int			init_philosophers(t_data *data);
int			init_mutexes(t_data *data);
void		init_state(t_data *data);

/* utils.c */
int			ft_isdigit(int c);
int			ft_is_number(char *str);
int			ft_atoi(const char *str);
long long	ft_atoll(const char *str);
long long	get_current_time(void);
void		print_status(t_data *data, int id, char *status);
int			validate_arg(char *arg, int arg_idx);
int			check_args(int argc, char **argv);

/* time.c */
long long	get_time_in_ms(void);
long long	get_current_time(void);
long long	time_diff_ms(long long end_time, long long start_time);
int		is_philosopher_dead(t_philosopher *philo);
void		smart_sleep(long long time_to_wait, t_data *data);

/* philosopher.c */
void		*philosopher_routine(void *arg);
void		eating(t_philosopher *philo);
void		thinking(t_philosopher *philo);
void		sleeping(t_philosopher *philo);

/* monitor.c */
int			check_if_anyone_died(t_data *data);
int			check_if_all_ate(t_data *data);
int			check_death(t_data *data, int philo_id);
void		check_all_ate(t_data *data);
void		mark_philosopher_death(t_data *data, int philo_id);
long		calculate_next_check_interval(t_data *data);
void		*monitor_routine(void *arg);
int			get_simulation_state(t_data *data);
void		set_simulation_state(t_data *data, int state);

/* error_handling.c */
void		print_error_message(int error_code, char *arg);
void		print_usage(char *program_name);
void		print_init_error(int error_code);
void		print_system_error(const char *syscall_name);
int			handle_thread_error(t_data *data, int created_count);
int			handle_mutex_error(t_data *data, int mutex_count, const char *mutex_name);

/* cleanup.c */
void		free_resources(t_data *data);
void		cleanup_mutex(pthread_mutex_t *mutex);
void		cleanup_mutexes(t_data *data, int count);

#endif 