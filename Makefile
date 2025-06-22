CC = gcc
CFLAGS = -Wall -pthread
SRC_SERVER = server.c server_fifo.c server_threaded.c server_prethreaded.c server_forked.c server_preforked.c utils.c
OBJ_SERVER = $(SRC_SERVER:.c=.o)
CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

.PHONY: all clean

all: server client

server: $(SRC_SERVER)
	$(CC) $(CFLAGS) -o server $(SRC_SERVER)

client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o client $(CLIENT_SRC)

clean:
	rm -f *.o server client
