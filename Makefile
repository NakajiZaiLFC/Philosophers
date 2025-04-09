NAME = philo

CC = cc
# CFLAGS = -Wall -Wextra -Werror -pthread
CFLAGS = -pthread
# デバッグフラグ
DEBUG_FLAGS = -g -DDEBUG
# スレッドセーフティチェックフラグ
THREAD_FLAGS = -g -fsanitize=thread

SRCS = philo.c init.c utils.c cleanup.c threading.c
OBJS = $(SRCS:.c=.o)

# テスト用ソース（philo.cを除く）
TEST_SRCS = init.c utils.c cleanup.c threading.c
TEST_OBJS = init.o utils.o cleanup.o threading.o

# テストプログラム
TEST_PHILO = test_philo
TEST_PHILO_SRCS = tests/philosopher_test.c

TEST_CLEANUP = test_cleanup
TEST_CLEANUP_SRCS = tests/cleanup_test.c

TEST_THREAD = test_thread
TEST_THREAD_SRCS = tests/thread_safety_test.c

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

thread_check: CFLAGS += $(THREAD_FLAGS)
thread_check: re

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(TEST_PHILO)
	rm -f $(TEST_CLEANUP)
	rm -f $(TEST_THREAD)

fclean: clean
	rm -f $(NAME)

re: fclean all

# テスト用ルール
test_philo: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_PHILO_SRCS) $(TEST_OBJS) -o $(TEST_PHILO)
	./$(TEST_PHILO)

test_cleanup: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_CLEANUP_SRCS) $(TEST_OBJS) -o $(TEST_CLEANUP)
	./$(TEST_CLEANUP)

test_thread: init.o utils.o cleanup.o threading.o
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -DTHREAD_SAFETY_TEST_STANDALONE $(TEST_THREAD_SRCS) utils.o threading.o -o $(TEST_THREAD)
	./$(TEST_THREAD)

test: test_philo test_cleanup test_thread

.PHONY: all clean fclean re debug thread_check test test_philo test_cleanup test_thread 