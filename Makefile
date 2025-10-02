CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -Iinclude

SRCS = src/main.c src/lexer.c src/prompt.c src/expansion.c src/path_search.c src/execute.c src/redirection.c src/piping.c src/background.c src/builtins.c src/parser.c

OBJS = $(SRCS:src/%.c=obj/%.o)

BIN = bin/shell

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f obj/*.o $(BIN)
	rm -rf obj bin

.PHONY: all clean
