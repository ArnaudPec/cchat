CC=gcc
CFLAGS= -Wall -Wextra -ggdb
LIBS= -pthread
all: client server

client:
	$(CC) client.c $(LIBS) -o client

server:
	$(CC) server.c $(LIBS) -o server

cppcheck:
	cppcheck --enable=all *.c

clean :
	rm -rf client server *o
