CC = gcc
CFLAGS = -Wall -Wextra -g 
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp



all : $(TARGET)/utilities $(TARGET)/interface $(TARGET)/interface_other_tty $(TARGET)/test_process $(TARGET)/mon_programme $(TARGET)/test $(TARGET)/new_tty $(TARGET)/write_on_new_tty

$(TARGET)/utilities : $(SRC)/utilities.h $(SRC)/utilities.c
	gcc -o $(TARGET)/utilities.o -c $(SRC)/utilities.c

$(TARGET)/interface :  $(SRC)/interface.c
	gcc  -L./$(VEC) -Wl,-rpath=./$(VEC) $< $(TARGET)/utilities.o -o $@  -lvec -lncursest -lpanelt -lmenut -lformt	-pthread -lpthread -ldwarf -lunwind -lunwind-ptrace -lunwind-generic

$(TARGET)/interface_other_tty :  $(SRC)/interface_other_tty.c
	gcc  -L./$(VEC) -Wl,-rpath=./$(VEC) $< $(TARGET)/utilities.o -o $@  -lvec -lncursest -lpanelt -lmenut -lformt	-pthread -lpthread -ldwarf -lunwind -lunwind-ptrace -lunwind-generic



$(TARGET)/test_process :  $(EXP)/test_process.c
	gcc -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@ -lvec
	
$(TARGET)/mon_programme:  $(SRC)/mon_programme.c
	gcc $< -o $@ 	

$(TARGET)/test:  $(SRC)/test.c
	gcc $< -g -gdwarf-2  -o $@ 	

$(TARGET)/new_tty:  $(SRC)/new_tty.c
	gcc $< -g -gdwarf-2  -o $@ 	 -lutil

$(TARGET)/write_on_new_tty:  $(SRC)/write_on_new_tty.c
	gcc $< -g -gdwarf-2  -o $@ 	 -lutil

clean:
	rm target/* 


run_interface:
	@LD_LINRARY_PATH=. target/interface

run_exp:
	./target/test_process	
