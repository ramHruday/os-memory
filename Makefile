CFLAGS=-Wall -Wextra -g --std=c99 --pedantic-errors -ggdb -gstrict-dwarf
CC=gcc
DEPS=process.h queue.h memory.h
OBJ=main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


clean:
	rm -f main *.o core.*

.PHONY: clean uncrustify
