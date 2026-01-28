CC = gcc
CFLAGS = -Wall -Wextra -g
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp

all : $(TARGET)/utilities $(TARGET)/interface $(TARGET)/debug_console $(TARGET)/test_process $(TARGET)/mon_programme $(TARGET)/test $(TARGET)/multifunction $(TARGET)/new_tty $(TARGET)/write_on_new_tty

$(TARGET)/utilities : $(SRC)/utilities.h $(SRC)/utilities.c
	gcc -o $(TARGET)/utilities.o -c $(SRC)/utilities.c

$(TARGET)/interface :  $(SRC)/interface.c
	gcc  -g -gdwarf-2 -L./$(VEC) -Wl,-rpath=./$(VEC) $< $(TARGET)/utilities.o -o $@  -lvec -lncursesw -lpanelw -lmenuw -lformw	-pthread -lpthread -ldwarf -lunwind -lunwind-ptrace -lunwind-generic

$(TARGET)/debug_console : $(SRC)/debug_console.c $(SRC)/utilities.c
	gcc -Wall -Wextra -g $(SRC)/debug_console.c $(TARGET)/utilities.o -o $@ -ldwarf -lunwind -lunwind-ptrace -lunwind-generic

$(TARGET)/test_process :  $(EXP)/test_process.c
	gcc -L./$(VEC) -Wl,-rpath=./$(VEC) $< -o $@ -lvec

$(TARGET)/mon_programme:  $(SRC)/mon_programme.c
	gcc $< -g -gdwarf-2 -o $@

$(TARGET)/test:  $(SRC)/test.c
	gcc $< -g -gdwarf-2 -mavx2  -o $@

$(TARGET)/multifunction:  $(SRC)/multifunction.c
	gcc $< -g -gdwarf-2 -mavx2  -o $@

$(TARGET)/new_tty:  $(SRC)/new_tty.c
	gcc $< -g -gdwarf-2  -o $@ 	 -lutil

$(TARGET)/write_on_new_tty:  $(SRC)/write_on_new_tty.c
	gcc $< -g -gdwarf-2 -o $@ 	 -lutil

clean:
	rm target/*

run_interface:
	@LD_LIBRARY_PATH=. target/interface

run_debug_console:
	./target/debug_console

run_exp:
	./target/test_process
