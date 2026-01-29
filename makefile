CC = gcc
CFLAGS = -Wall -Wextra -g
OFLAGS = -O3
SRC = src
LIB = lib
VEC = ext/vec
TARGET = target
EXP = exp

all : $(TARGET)/utilities $(TARGET)/debug_console $(TARGET)/main_interactive $(TARGET)/test_process $(TARGET)/mon_programme $(TARGET)/test $(TARGET)/multifunction $(TARGET)/new_tty $(TARGET)/write_on_new_tty

$(TARGET)/utilities : $(SRC)/utilities.h $(SRC)/utilities.c
	gcc -o $(TARGET)/utilities.o -c $(SRC)/utilities.c

$(TARGET)/debug_console : $(SRC)/debug_console.c $(SRC)/utilities.c
	gcc -Wall -Wextra -g $(SRC)/debug_console.c $(TARGET)/utilities.o -o $@ -ldwarf -lunwind -lunwind-ptrace -lunwind-generic

$(TARGET)/main_interactive : $(SRC)/main_interactive.c $(SRC)/core/breakpoints.c $(SRC)/core/symbols.c $(SRC)/core/disasm.c $(SRC)/core/watchpoints.c $(SRC)/display/output.c $(SRC)/display/sections.c $(SRC)/display/theme.c $(SRC)/cli/config.c $(SRC)/cli/parser.c $(SRC)/cli/readline.c
	gcc -Wall -Wextra -g -DHAVE_LIBUNWIND -I$(SRC) -I$(SRC)/core -I$(SRC)/display -I$(SRC)/cli $(SRC)/main_interactive.c $(SRC)/core/breakpoints.c $(SRC)/core/symbols.c $(SRC)/core/disasm.c $(SRC)/core/watchpoints.c $(SRC)/display/output.c $(SRC)/display/sections.c $(SRC)/display/theme.c $(SRC)/cli/config.c $(SRC)/cli/parser.c $(SRC)/cli/readline.c -o $@ -lunwind -lunwind-ptrace -lunwind-generic -lreadline

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
	rm -rf tests/binaries

run_debug_console:
	./target/debug_console

run_exp:
	./target/test_process

# Test targets
test: all
	@echo "Running test suite..."
	@./tests/all_tests.sh

test-basic: all
	@echo "Running basic tests..."
	@./tests/all_tests.sh test_basic

test-breakpoints: all
	@echo "Running breakpoint tests..."
	@./tests/all_tests.sh test_breakpoints

test-step: all
	@echo "Running step tests..."
	@./tests/all_tests.sh test_step

test-watchpoints: all
	@echo "Running watchpoint tests..."
	@./tests/all_tests.sh test_watchpoints

test-registers: all
	@echo "Running register/memory tests..."
	@./tests/all_tests.sh test_registers_memory

test-disas: all
	@echo "Running disas/list tests..."
	@./tests/all_tests.sh test_disas_list

test-verbose: all
	@echo "Running all tests with verbose output..."
	@./tests/all_tests.sh

.PHONY: all clean test test-basic test-breakpoints test-step test-watchpoints test-registers test-disas test-verbose
