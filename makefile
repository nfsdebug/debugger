CC = gcc
CFLAGS = -Wall -Wextra -g 
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp


all : $(TARGET)/interface $(TARGET)/test_process

$(TARGET)/interface :  $(SRC)/interface.c 
	gcc  -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@  -lvec -lcurses -lpanel -lmenu -lform	

$(TARGET)/test_process :  $(EXP)/test_process.c
	gcc -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@ -lvec

clean:
	rm target/*


run_interface:
	./target/interface

run_exp:
	./target/test_process	