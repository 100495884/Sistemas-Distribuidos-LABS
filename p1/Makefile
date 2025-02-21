BIN_FILES  = p1

CC = gcc

CFLAGS = -Wall

all: $(BIN_FILES)

p1: p1.o
	$(CC) $(CFLAGS) $^  -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o

