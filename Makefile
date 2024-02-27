CC=gcc
CFLAGS=-g -lpthread
BINS=server client

all: $(BINS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client

server: server.c
	$(CC) $(CFLAGS) server.c -o server
