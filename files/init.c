#include "philo.h"

static int	validate_args(int argc, char **argv)
{
	int	i;
	int	j;

	DEBUG_PRINT("Validating arguments: argc=%d", argc);
	if (argc != 5 && argc != 6)
	{
		DEBUG_PRINT("Error: wrong number of arguments");
		return (1);
	}
	i = 1;
	while (i < argc)
	{
		j = 0;
		while (argv[i][j])
		{
			if (argv[i][j] < '0' || argv[i][j] > '9')
			{
				DEBUG_PRINT("Error: invalid character in argument %d: %c", i, argv[i][j]);
				return (1);
			}
			j++;
		}
		i++;
	}
	return (0);
}

int	init_data(t_data *data, int argc, char **argv)
{
	DEBUG_PRINT("Initializing data structure");
	
	// リソースインベントリを初期化
	if (init_resource_inventory() != 0)
	{
		DEBUG_PRINT("Error: リソースインベントリの初期化に失敗しました");
		return (1);
	}
	
	if (validate_args(argc, argv))
		return (1);
	data->num_philos = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->must_eat = ft_atoi(argv[5]);
	else
		data->must_eat = -1;
	DEBUG_PRINT("Parsed arguments: philosophers=%d, time_to_die=%d, time_to_eat=%d, time_to_sleep=%d, must_eat=%d",
		data->num_philos, data->time_to_die, data->time_to_eat, data->time_to_sleep, data->must_eat);
	if (data->num_philos <= 0 || data->time_to_die <= 0
		|| data->time_to_eat <= 0 || data->time_to_sleep <= 0
		|| (argc == 6 && data->must_eat <= 0))
	{
		DEBUG_PRINT("Error: invalid argument values");
		return (1);
	}

	// シミュレーション状態の初期化
	data->is_dead = 0;
	data->sim_state = SIM_RUNNING;
	data->single_philo = (data->num_philos == 1) ? 1 : 0;
	
	// 時間の初期化
	data->start_time = get_time();
	
	// ミューテックスの初期化
	if (init_mutex(data) != 0)
		return (1);
	
	// フォークの初期化
	if (init_forks(data) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		cleanup_single_mutex(&data->start_lock);
		cleanup_single_mutex(&data->meal_lock);
		return (1);
	}
	
	DEBUG_PRINT("Data structure initialized successfully");
	return (0);
}

int	init_mutex(t_data *data)
{
	DEBUG_PRINT("Initializing mutexes");
	
	if (pthread_mutex_init(&data->print, NULL) != 0)
		return (1);
	register_mutex(&data->print, "print mutex");
	
	if (pthread_mutex_init(&data->death, NULL) != 0)
	{
		cleanup_single_mutex(&data->print);
		return (1);
	}
	register_mutex(&data->death, "death mutex");
	
	if (pthread_mutex_init(&data->start_lock, NULL) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		return (1);
	}
	register_mutex(&data->start_lock, "start_lock mutex");
	
	if (pthread_mutex_init(&data->meal_lock, NULL) != 0)
	{
		cleanup_single_mutex(&data->print);
		cleanup_single_mutex(&data->death);
		cleanup_single_mutex(&data->start_lock);
		return (1);
	}
	register_mutex(&data->meal_lock, "meal_lock mutex");
	
	return (0);
}

int	init_forks(t_data *data)
{
	int	i;
	char fork_desc[32];

	DEBUG_PRINT("Initializing forks");
	data->forks = malloc(sizeof(t_fork) * data->num_philos);
	if (!data->forks)
	{
		DEBUG_PRINT("Error: failed to allocate memory for forks");
		return (1);
	}
	register_memory(data->forks, "forks array");
	
	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_mutex_init(&data->forks[i].mutex, NULL) != 0)
		{
			while (--i >= 0)
				cleanup_single_mutex(&data->forks[i].mutex);
			free(data->forks);
			unregister_resource(data->forks);
			return (1);
		}
		snprintf(fork_desc, sizeof(fork_desc), "fork %d mutex", i);
		register_mutex(&data->forks[i].mutex, fork_desc);
		
		data->forks[i].state = FORK_AVAILABLE;
		data->forks[i].owner_id = -1;
		i++;
	}
	return (0);
}

int	init_philos(t_data *data)
{
	int	i;

	DEBUG_PRINT("Initializing philosophers");
	data->philos = malloc(sizeof(t_philo) * data->num_philos);
	if (!data->philos)
	{
		DEBUG_PRINT("Error: failed to allocate memory for philosophers");
		return (1);
	}
	register_memory(data->philos, "philosophers array");
	
	i = 0;
	while (i < data->num_philos)
	{
		data->philos[i].id = i + 1;
		data->philos[i].state = PHILO_THINKING;
		data->philos[i].left_fork = i;
		data->philos[i].right_fork = (i + 1) % data->num_philos;
		data->philos[i].eat_count = 0;
		data->philos[i].last_eat_time = 0;
		data->philos[i].data = data;
		DEBUG_PRINT("Initialized philosopher %d: left_fork=%d, right_fork=%d",
			data->philos[i].id, data->philos[i].left_fork, data->philos[i].right_fork);
		i++;
	}
	return (0);
}

#ifdef DEBUG
void	debug_init(t_data *data)
{
	int	i;

	printf("=== 初期化デバッグ情報 ===\n");
	printf("哲学者の数: %d\n", data->num_philos);
	printf("死亡時間: %d\n", data->time_to_die);
	printf("食事時間: %d\n", data->time_to_eat);
	printf("睡眠時間: %d\n", data->time_to_sleep);
	printf("必要食事回数: %d\n", data->must_eat);
	printf("開始時間: %lld\n", data->start_time);
	printf("\nフォークの状態:\n");
	i = 0;
	while (i < data->num_philos)
	{
		printf("フォーク %d: 状態=%d, 所有者=%d\n",
			i, data->forks[i].state, data->forks[i].owner_id);
		i++;
	}
	printf("\n哲学者の状態:\n");
	i = 0;
	while (i < data->num_philos)
	{
		printf("哲学者 %d: 状態=%d, 左フォーク=%d, 右フォーク=%d\n",
			data->philos[i].id, data->philos[i].state,
			data->philos[i].left_fork, data->philos[i].right_fork);
		i++;
	}
	
	// リソースインベントリ情報を表示
	print_resource_inventory();
	
	printf("========================\n");
} 
#endif