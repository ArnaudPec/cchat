cc=gcc -Wall -Wextra
all: client server

client: client.c
	${cc} -c client.c

server: server.c
	${cc} -c server.c

cppcheck:
	cppcheck --enable=all *.c

clean :
	rm -rf client server
