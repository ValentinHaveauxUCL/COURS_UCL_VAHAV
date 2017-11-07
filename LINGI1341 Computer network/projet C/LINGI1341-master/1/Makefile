CC=gcc
CFLAGS=-c -Werror -Wall -Wextra -Wshadow -g
LDFLAGS=-lm -lz
EXEC=packet
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

packet: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

main:o	packet_interface.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
