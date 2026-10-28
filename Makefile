CC = gcc
CFLAGS = -Wall -Wextra -g -O2

SRC = main.c utils/server.c utils/parser.c utils/hash.c

all: server

server: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o server

clean:
	rm -f server

re: clean all

.PHONY: all clean re