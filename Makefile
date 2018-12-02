CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: client server

client: client.c csapp.c csapp.h 
server: server.c server.h csapp.c csapp.h gofish.c gofish.h player.c player.h deck.c deck.h

clean:
	rm -rf *~ client server *.o
