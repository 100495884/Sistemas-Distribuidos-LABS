# Variables
CC = gcc
CFLAGS = -Wall -pthread
SRC = servidor-mq.c claves.c proxy-mq.c
OBJ = $(SRC:.c=.o)
EXEC = servidor-mq

# Regla para compilar el ejecutable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Regla para compilar los archivos .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -f $(OBJ) $(EXEC)

# Regla para ejecutar el programa
run: $(EXEC)
	./$(EXEC)

# Regla por defecto (ejecutar el programa)
all: $(EXEC) run
