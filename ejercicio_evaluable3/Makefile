CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -fPIC -I/usr/include/tirpc \
         -Wno-unused-variable -Wno-cast-function-type -Wno-unused-parameter
LDFLAGS = -lpthread -ltirpc

RPC_X = claves.x
RPCGEN_SRCS = claves_clnt.c claves_svc.c claves_xdr.c
RPCGEN_HDR = claves.h

CLIENT_SRC = app-cliente.c
PROXY_SRC = proxy-rpc.c
CLAVES_SRC = claves_real.c
SERVER_SRC = servidor-rpc.c

LIBCLAVES = libclaves.so
CLIENTE = cliente
SERVIDOR = servidor

# Objetivo por defecto
all: $(RPCGEN_SRCS) $(RPCGEN_HDR) $(LIBCLAVES) $(CLIENTE) $(SERVIDOR)

# Generar archivos RPC si no existen o si cambia el .x
$(RPCGEN_SRCS) $(RPCGEN_HDR): $(RPC_X)
	rpcgen -C $(RPC_X)

# Librería compartida
$(LIBCLAVES): $(PROXY_SRC) claves_clnt.o claves_xdr.o
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

# Cliente
$(CLIENTE): $(CLIENT_SRC) proxy-rpc.o claves_clnt.o claves_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

# Servidor
$(SERVIDOR): $(SERVER_SRC) claves_svc.o claves_xdr.o $(CLAVES_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regla general para .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Limpiar
clean:
	rm -f *.o *.so $(CLIENTE) $(SERVIDOR) claves_clnt.c claves_svc.c claves_xdr.c claves.h
