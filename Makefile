# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmakhmae <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/21 15:19:28 by mmakhmae          #+#    #+#              #
#    Updated: 2026/06/21 15:19:34 by mmakhmae         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = libftcodexion.a

SOURCES = utils.c \
		main.c

OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -Wall -Wextra -Werror -pthread -g

all: $(NAME)

$(NAME): $(OBJECTS)
	gcc $(CFLAGS) $(SOURCES) -o codex
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./codex 5 400 200 200 1 2 1 fifo

clean:
	rm -rf codex
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
