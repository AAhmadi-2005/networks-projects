# compiler
CC = gcc
CFLAGS = -Wall -g

PORT = 8080
IP = 127.0.0.1

# defualt
all: server client parserver

# server
server: server.c
	$(CC) $(CFLAGS) server.c -o server

# client
client: client.c
	$(CC) $(CFLAGS) client.c -o client

parserver: parserver.c
	$(CC) $(CFLAGS)  parserver.c -o parserver


# run server
runs: server
	./server $(PORT)

# run client
runc: client
	./client $(IP) $(PORT)

# run parserver
runp: parserver
	./parserver $(PORT)
	
# cleaning
clean:
	rm -f server client parserver
