all: run

.PHONY: clean

run: debug test
	./debug test

test: test.c 
	gcc -no-pie -g test.c -o test

# TODO fix lib error
debug: debug.c test.c
	gcc -Wall -Wextra -g debug.c lib/linenoise.c -o debug -ldwarf -lunwind -lelf 

clean:
	rm -rf *.o *.out test debug history.txt
