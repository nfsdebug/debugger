CC = gcc
CFLAGS = -Wall -Wextra -g 
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp



all : $(TARGET)/interface $(TARGET)/test_process $(TARGET)/mon_programme $(TARGET)/test

$(TARGET)/interface :  $(SRC)/interface.c 
	gcc  -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@  -lvec -lncursest -lpanelt -lmenut -lformt	-pthread -lpthread

$(TARGET)/test_process :  $(EXP)/test_process.c
	gcc -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@ -lvec
	
$(TARGET)/mon_programme:  $(SRC)/mon_programme.c
	gcc $< -o $@ 	

$(TARGET)/test:  $(SRC)/test.c
	gcc $< -o $@ 		

clean:
	rm target/* 


run_interface:
	@LD_LINRARY_PATH=. target/interface

run_exp:
	./target/test_process	
