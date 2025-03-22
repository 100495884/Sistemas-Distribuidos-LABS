# Variables
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -fPIC
LDFLAGS = -lpthread
CLIENT_SRC = app-cliente.c
SERVER_SRC = servidor-sock.c
PROXY_SRC = proxy-sock.c
CLAVES_SRC = claves.c

all: servidor cliente libclaves.so

servidor: $(SERVER_SRC) $(CLAVES_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cliente: $(CLIENT_SRC) libclaves.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,. $(LDFLAGS)

libclaves.so: $(PROXY_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f servidor cliente *.o *.so

.PHONY: all clean