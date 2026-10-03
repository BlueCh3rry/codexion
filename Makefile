# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/14 17:51:22 by mmakhmae          #+#    #+#              #
#    Updated: 2026/09/14 17:51:36 by mmakhmae         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

SOURCES = src/utils.c \
		  src/utils2.c \
		  src/monitor.c \
		  src/compile.c \
		  src/coder_routine.c \
		  src/coder_routine2.c \
		  src/parser.c \
		  src/heap.c \
		  src/main.c

OBJECTS = $(SOURCES:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -g

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

run: $(NAME)
	./$(NAME) 5 3000 200 200 200 5 400 fifo

leak: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 5 3000 200 200 200 5 400 fifo

helgrind: $(NAME)
	valgrind --tool=helgrind -s ./$(NAME) 5 3000 200 200 200 5 400 fifo

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re leak helgrind
