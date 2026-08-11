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
	gcc $(CFLAGS) $(SOURCES) -o codexion
	./codexion 4 1000 200 200 200 4 5 fifo

leak:
	gcc $(CFLAGS) $(SOURCES) -o codexion
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./codexion 4 1000 200 200 200 1 5 fifo

helgrind:
	gcc $(CFLAGS) $(SOURCES) -o codexion
	valgrind --tool=helgrind ./codexion 4 1000 200 200 200 1 5 fifo

# fix:
# 	gcc $(CFLAGS) $(SOURCES) -o codexion
# 	./codexion 4 100 200 200 200 1 5 fifo

clean:
	rm -rf codexion
	rm -f $(OBJECTS) main2.o main.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re leak
