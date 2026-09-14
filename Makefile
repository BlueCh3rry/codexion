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

SOURCES = utils.c \
          chrono.c \
          compile.c \
          coder_routine.c \
		  parser.c \
          main.c

OBJECTS = $(SOURCES:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -g

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

run: $(NAME)
	./$(NAME) 99 1000 100 100 100 4 5 fifo

leak: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 300 10 20 20 20 1 5 fifo

helgrind: $(NAME)
	valgrind --tool=helgrind ./$(NAME) 300 10 20 20 20 1 5 edf

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re leak helgrind
