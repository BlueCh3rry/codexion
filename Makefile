NAME = Codex

all:
	gcc -pthread main.c -o $(NAME)
	./$(NAME) 5 20 15 50 100 2 500 fifo

clean:
	rm -rf $(NAME)

fclean:

re: fclean clean all

.PHONY: all clean fclean re