all: run

.PHONY: clean lib

run: debug test lib
	./debug test

test: test.c 
	gcc -g -gdwarf-2 test.c -o test
lib: lib/utilities.h lib/utilities.c
	gcc  -o utilities.o -c lib/utilities.c 
# TODO fix lib error
debug: debug.c test lib
	gcc -Wall -Wextra -g debug.c utilities.o -o debug -ldwarf -lunwind -lunwind-ptrace -lunwind-generic 

clean:
	rm -rf *.o *.out test debug history.txt
