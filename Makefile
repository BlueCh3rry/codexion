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

CFLAGS = -Wall -Wextra -Werror -Ilibft 

all: $(NAME)

$(NAME): $(OBJECTS)
	make -C $(LIBFT)
	ar rcs $(NAME) $^ $(LIBFT_A)
	gcc -pthread main.c -o codex
	./codex 5 400 200 200 1 5 1 fifo

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	make clean -C $(LIBFT)
	rm -rf push_swap
	rm -f $(OBJECTS)

fclean: clean
	make fclean -C $(LIBFT)
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
