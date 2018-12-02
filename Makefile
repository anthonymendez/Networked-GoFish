CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: client server

client: client.c csapp.c csapp.h gofish.c gofish.h player.c player.h deck.c deck.h
server: server.c csapp.c csapp.h

clean:
	rm -rf *~ client server *.o
