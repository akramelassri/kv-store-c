CFLAGS = -Wall -Wextra -g -O2

all: server

server: server.c hash.c hash.h
	gcc $(CFLAGS) -o server server.c hash.c

clean:
	rm -f server *.o

.PHONY: all clean
