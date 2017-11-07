CFLAGS = -Werror -Wall -Wshadow -g -Wextra
LFLAGS = -lz
CC = gcc
SRCDIR=src
MAINS = receiver.c sender.c
EXEC = $(MAINS:%.c=%)
ALLSRC = $(wildcard $(SRCDIR)/*.c)
SRC = $(filter-out $(addprefix $(SRCDIR)/,$(MAINS)),$(ALLSRC))
OBJ = $(SRC:%.c=%.o)

all: $(EXEC)

$(EXEC): % : $(SRCDIR)/%.o $(OBJ)
	@$(CC) -o $@ $^ $(LFLAGS)


%.o: $(SRCDIR)/%.c
	@$(CC) -o $(SRCDIR)/$@ -c $< $(CFLAGS)

test: tests/test.c
	$(CC) -o tests/$@ -c $< $(LFLAGS)

clean:
	rm -rf $(SRCDIR)/*.o tests/*.o

mrproper: clean
	rm -rf $(EXEC)

list:
	@echo "Sources: $(SRC)\nObjets: $(OBJ)\nMains: $(MAINS)\nEXEC: $(EXEC)"
