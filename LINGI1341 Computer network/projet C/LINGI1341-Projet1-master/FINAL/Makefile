CSFLAGS = -Werror -Wall
LCFLAGS = -lz
CC = gcc

all: receiver sender

receiver: receiver.o
	$(CC) receiver.o -o receiver $(CSFLAGS) $(LCFLAGS)

receiver.o: receiver.c socket.h packet.h packet_interface.h
	$(CC) -c receiver.c -o receiver.o

sender: sender.o
	$(CC) sender.o -o sender $(CSFLAGS) $(LCFLAGS)

sender.o: sender.c socket.h packet.h packet_interface.h	
	$(CC) -c sender.c -o sender.o

test: test.o
	$(CC) test.o -o test $(CSFLAGS) $(LCFLAGS) -lcunit

test.o: test.c
	$(CC) -c test.c -o test.o 



