CC=gcc
CFLAGS= -I. -Wall -Wextra -ggdb
LIBS= -pthread
DEPS= utils.h

CLIENT_OBJS=client.o utils.o
SERVER_OBS=server.o utils.o

all: client server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(CLIENT_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

server: $(SERVER_OBS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

cppcheck:
	cppcheck --enable=all *.c

clean :
	rm -rf client server *o
