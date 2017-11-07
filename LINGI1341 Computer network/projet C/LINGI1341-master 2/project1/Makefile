# Compiler
CC = gcc
# See gcc/clang manual to understand all flags
#CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -I$(HOME)/local/include

# We have no libraries to link against except libc
LDFLAGS += -lz
# LDFLAGS += -L$(HOME)/local/lib -lcunit

# Directory
DIR = src
DIR2 = tests

# DESPS
DEPS = $(DIR)/packet_interface.h $(DIR)/create_socket.h $(DIR)/read_write_loop.h $(DIR)/real_address.h $(DIR)/wait_for_client.h $(DIR)/sender.h $(DIR)/receiver.h $(DIR)/queue.h $(DIR)/timer.h $(DIR)/utils.h

# Object
OBJ = $(DIR)/packet_implem.o $(DIR)/create_socket.o $(DIR)/read_write_loop.o $(DIR)/real_address.o $(DIR)/wait_for_client.o $(DIR)/queue.o $(DIR)/timer.o $(DIR)/utils.o
OBJ2 = $(DIR2)/test.o

# Default target
all: clean sender receiver

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter
debug: clean sender

# We use an implicit rule: look for the files record.c/database.c,
# compile them and link the resulting *.o's into an executable named database
%.o: $(DIR)/%.c  $(DEPS)
	$(CC) -c $@ $< $(CFLAGS)
%.o : $(DIR2)/%.c $(DEPS)
	$(CC) -c $@ $< $(CFLAGS)
test : $(DIR)/packet_implem.o $(DIR)/real_address.o $(DIR)/create_socket.o $(DIR2)/test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
sender: $(OBJ) src/sender.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
receiver: $(OBJ) src/receiver.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	@rm -f $(DIR)/*.o $(DIR2)/*.o sender receiver test
