CC=gcc
CFLAGS=-Werror -Wall
LFLAGS=-lz
EXEC=programme
FILENAME=

all:$(EXEC)

programme: packet_implem_100.o packet_interface.h
	$(CC) packet_implem_100.o -o programme $(CFLAGS) $(LFLAGS)

packet_implem_100.o: packet_implem_100.c packet_interface.h
	$(CC) -c packet_implem_100.c -o packet_implem_100.o $(CFLAGS) $(LFLAGS)
 
osef: $(FILENAME).c
	gcc -c $(FILENAME).c -o $(FILENAME).o -lpthread -lm
	gcc $(FILENAME).o -o $(FILENAME) -lpthread -lm

test: test.o
	$(CC) test.o -o test
	
test.o: test.c
	$(CC) -c test.c -o test.o


