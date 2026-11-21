CC=g++
CFLAGS=-std=c++11 -g

all: server client

server: main.cpp reactor.cpp
	$(CC) $(CFLAGS) $^ -o $@

client: test_client.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f server client
