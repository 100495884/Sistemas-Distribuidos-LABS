# Variables
CC = gcc
CFLAGS = $(shell pkg-config --cflags libtirpc)
LIBS = $(shell pkg-config --libs libtirpc) -lpthread
RPCGEN = rpcgen -Y /usr/bin

# Archivos generados por rpcgen
RPC_X = log.x
RPC_HDR = log.h
RPC_SVC = log_svc.c
RPC_CLNT = log_clnt.c
RPC_XDR = log_xdr.c

# Archivos fuente
SERVER_SRC = server.c $(RPC_CLNT) $(RPC_XDR)
LOG_SERVER_SRC = log_server.c $(RPC_SVC) $(RPC_XDR)

# Binarios
SERVER_BIN = server
LOG_SERVER_BIN = log_server

# Reglas
all: $(RPC_HDR) $(SERVER_BIN) $(LOG_SERVER_BIN)

$(RPC_HDR): $(RPC_X)
	$(RPCGEN) $(RPC_X)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(LOG_SERVER_BIN): $(LOG_SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(SERVER_BIN) $(LOG_SERVER_BIN) $(RPC_HDR) $(RPC_SVC) $(RPC_CLNT) $(RPC_XDR)

run-server:
	@echo "Ejecutando servidor principal..."
	@export LOG_RPC_IP=127.0.0.1 && ./$(SERVER_BIN) 8080

run-log:
	@echo "Ejecutando servidor RPC de logs..."
	@./$(LOG_SERVER_BIN)

run-timestamp:
	@echo "Ejecutando timestamp server en Python..."
	@python3 timestamp_server.py

run-client:
	@echo "Ejecutando cliente Python..."
	@python3 client.py -s 127.0.0.1 -p 8080
