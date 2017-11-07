CSFLAGS = -Werror -Wall
SRC = src/
TESTS = tests/
LCFLAGS = -lz
CC = gcc

all: receiver sender

receiver: $(SRC)receiver.c $(SRC)socket.h $(SRC)packet.h
	$(CC) -c $(SRC)receiver.c -o $(SRC)receiver.o $(CSFLAGS) $(LCFLAGS)
	$(CC) $(SRC)receiver.o -o receiver $(CSFLAGS) $(LCFLAGS)
	rm $(SRC)receiver.o

sender: $(SRC)sender.c $(SRC)socket.h $(SRC)packet.h
	$(CC) -c $(SRC)sender.c -o $(SRC)sender.o $(CSFLAGS) $(LCFLAGS)
	$(CC) $(SRC)sender.o -o sender $(CSFLAGS) $(LCFLAGS)
	rm $(SRC)sender.o

tests: $(TESTS)test.c $(SRC)packet.h $(SRC)packet_interface.h $(SRC)socket.h
	$(CC) -c $(TESTS)test.c -o $(TESTS)tests_compilation.o $(LCFLAGS) -lcunit
	$(CC) $(TESTS)tests_compilation.o -o tests_compilation $(LCFLAGS) -lcunit
	rm $(TESTS)tests_compilation.o

.PHONY: tests
.PHONY: compile






