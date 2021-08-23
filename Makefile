CC = gcc
RM = rm
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99
OBJS = main.o parse.o ast.o check.o stack.o run.o eval.o utils.o

all: clean $(OBJS)
	@$(CC) $(OBJS) -o myfind

check: all
	@./tests/exec.sh

test: clean $(OBJS)
	@$(CC) $(OBJS) -o test -g -fsanitize=address

clean:
	@$(RM) -f $(OBJS) myfind test

%.o: src/%.c
	@$(CC) -c $< $(CFLAGS)

.PHONY: clean