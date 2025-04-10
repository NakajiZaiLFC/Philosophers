NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
# デバッグフラグ
DEBUG_FLAGS = -g -DDEBUG
# スレッドセーフティチェックフラグ
THREAD_FLAGS = -g -fsanitize=thread

# objディレクトリのパス
OBJ_DIR = obj
# ソースディレクトリのパス
SRC_DIR = src

# ソースファイルを修正：新しいファイルに関数を分割
SRCS = philo.c \
       philo_utils.c \
       init.c \
       utils.c \
       cleanup.c \
       cleanup_utils.c \
       cleanup_extra.c \
       resource_mgmt.c \
       threading.c \
       fork_utils.c \
       fork_extra.c

# ソースファイル（ディレクトリパスを含む）
SRC_FILES = $(addprefix $(SRC_DIR)/, $(SRCS))
# オブジェクトファイル
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# テスト用ソース（philo.cを除く）
TEST_SRCS = init.c utils.c cleanup.c threading.c
TEST_OBJS = $(addprefix $(OBJ_DIR)/, $(TEST_SRCS:.c=.o))

# テストプログラム
TEST_PHILO = test_philo
TEST_PHILO_SRCS = tests/philosopher_test.c

TEST_CLEANUP = test_cleanup
TEST_CLEANUP_SRCS = tests/cleanup_test.c

TEST_THREAD = test_thread
TEST_THREAD_SRCS = tests/thread_safety_test.c

all: $(NAME)

# objディレクトリを作成
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJ_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

thread_check: CFLAGS += $(THREAD_FLAGS)
thread_check: re

# ソースディレクトリからオブジェクトファイルを生成するルール
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TEST_PHILO)
	rm -f $(TEST_CLEANUP)
	rm -f $(TEST_THREAD)

fclean: clean
	rm -f $(NAME)

re: fclean all

# テスト用ルール
test_philo: $(OBJ_DIR) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_PHILO_SRCS) $(TEST_OBJS) -o $(TEST_PHILO)
	./$(TEST_PHILO)

test_cleanup: $(OBJ_DIR) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_CLEANUP_SRCS) $(TEST_OBJS) -o $(TEST_CLEANUP)
	./$(TEST_CLEANUP)

test_thread: $(OBJ_DIR) $(OBJ_DIR)/utils.o $(OBJ_DIR)/threading.o
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -DTHREAD_SAFETY_TEST_STANDALONE $(TEST_THREAD_SRCS) $(OBJ_DIR)/utils.o $(OBJ_DIR)/threading.o -o $(TEST_THREAD)
	./$(TEST_THREAD)

test: test_philo test_cleanup test_thread

.PHONY: all clean fclean re debug thread_check test test_philo test_cleanup test_thread