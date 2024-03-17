NAME		= ft_ping
CC			= cc
FLAGS		= -Werror -Wextra -Wall
SRCS		=  src/ft_ping.c
OBJS		= $(SRCS:.c=.o)
HEADER		= includes/ft_ping.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re