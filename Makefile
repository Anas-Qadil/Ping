NAME		= ft_ping
CC			= cc
# FLAGS		= -Werror -Wextra -Wall
FLAGS = 
SRCS		=  src/ft_ping.c

OBJ = $(addprefix obj/, $(SRC:.c=.o))

HEADER = includes/ft_ping.h

$(NAME): $(OBJS) $(HEADER)
	$(CC) ${FLAGS} $(SRCS) -o $(NAME)

%.o : %.c
	$(CC) $(FLAGS) -o $@ -c $<

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all