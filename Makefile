NAME = Codex

all:
	gcc -pthread main.c -o $(NAME)
	./$(NAME)

clean:
	rm -rf $(NAME)

fclean:

re: fclean clean all

.PHONY: all clean fclean re