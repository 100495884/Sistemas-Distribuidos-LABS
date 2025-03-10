# Variables
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -fPIC
LDFLAGS = -lrt -lpthread
CLIENT_SRC = app-cliente.c
CLIENT2_SRC = app-cliente-2.c
SERVER_SRC = servidor-mq.c
PROXY_SRC = proxy-mq.c
CLAVES_SRC = claves.c

all: servidor cliente cliente2 libclaves.so

servidor: $(SERVER_SRC) $(CLAVES_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cliente: $(CLIENT_SRC) libclaves.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves $(LDFLAGS)

cliente2: $(CLIENT2_SRC) libclaves.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves $(LDFLAGS)

libclaves.so: $(PROXY_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f servidor cliente cliente2 *.o *.so

.PHONY: all clean