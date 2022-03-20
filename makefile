CC = gcc
CFLAGS = -Wall -Wextra -g 
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp



all : $(TARGET)/utilities $(TARGET)/interface $(TARGET)/test_process $(TARGET)/mon_programme $(TARGET)/test

$(TARGET)/utilities : $(SRC)/utilities.h $(SRC)/utilities.c
	gcc -o $(TARGET)/utilities.o -c $(SRC)/utilities.c

$(TARGET)/interface :  $(SRC)/interface.c
	gcc  -L./$(VEC) -Wl,-rpath=./$(VEC) $< $(TARGET)/utilities.o -o $@  -lvec -lncursest -lpanelt -lmenut -lformt	-pthread -lpthread -ldwarf -lunwind -lunwind-ptrace -lunwind-generic

$(TARGET)/test_process :  $(EXP)/test_process.c
	gcc -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@ -lvec
	
$(TARGET)/mon_programme:  $(SRC)/mon_programme.c
	gcc $< -o $@ 	

$(TARGET)/test:  $(SRC)/test.c
	gcc $< -gdwarf-2  -o $@ 		

clean:
	rm target/* 


run_interface:
	@LD_LINRARY_PATH=. target/interface

run_exp:
	./target/test_process	
