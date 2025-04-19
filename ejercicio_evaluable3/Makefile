# ===========================
# Makefile para ejercicio RPC
# ===========================

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -fPIC -I/usr/include/tirpc
LDFLAGS = -lpthread -ltirpc


# Archivos fuente
RPC_X = claves.x
RPC_GEN = claves_clnt.c claves_svc.c claves_xdr.c claves.h

CLIENT_SRC = app-cliente.c
PROXY_SRC = proxy-rpc.c
CLAVES_SRC = claves.c

SERVER_SRC = servidor-rpc.c
REAL_SRC = claves_real.c

# Objetivos
all: cliente servidor

# Generar archivos RPC automáticamente
$(RPC_GEN): $(RPC_X)
	rpcgen -C $(RPC_X)

# Cliente
libclaves.so: $(PROXY_SRC) $(RPC_GEN)
	$(CC) $(CFLAGS) -shared -o $@ $(PROXY_SRC) claves_clnt.c claves_xdr.c $(LDFLAGS)

cliente: $(CLIENT_SRC) libclaves.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath=. $(LDFLAGS)

# Servidor
servidor: $(SERVER_SRC) $(REAL_SRC) claves_svc.c claves_xdr.c
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(REAL_SRC) claves_svc.c claves_xdr.c $(LDFLAGS)

# Limpiar
clean:
	rm -f cliente servidor libclaves.so *.o claves_clnt.c claves_svc.c claves_xdr.c claves.h

.PHONY: all clean
