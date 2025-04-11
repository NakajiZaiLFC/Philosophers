NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -Iinclude

SRC_DIR = src
OBJ_DIR = obj

SRC = main.c \
	init.c \
	cleanup.c \
	philo.c \
	utils.c \
	sleep.c \
	think.c \
	handle.c \
	monitor.c \
	condition.c \
	fork_utils.c \
	status.c \
	eat.c
	

SRCS = $(addprefix $(SRC_DIR)/, $(SRC))
OBJS = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(NAME): $(OBJ_DIR) $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re