NAME = Codex

all:
	gcc -pthread main.c -o $(NAME)
	./$(NAME) 5 400 200 200 1 5 1 fifo

clean:
	rm -rf $(NAME)

fclean:

re: fclean clean all

.PHONY: all clean fclean re