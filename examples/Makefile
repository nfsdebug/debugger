# Makefile for NDB Debugger Examples
# Compiles all example programs with various configurations

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -O0
SANITIZERS = -fsanitize=address,undefined

# Pthread library for threading examples
PTHREAD = -pthread

# Targets
EXAMPLES = segfault_example memory_leak_example use_after_free race_condition infinite_loop integer_overflow

# Default target: build all examples
all: $(EXAMPLES)

# Build individual examples
segfault_example: segfault_example.c
	$(CC) $(CFLAGS) $< -o $@

memory_leak_example: memory_leak_example.c
	$(CC) $(CFLAGS) $< -o $@

use_after_free: use_after_free.c
	$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) -fsanitize=address $< -o $@_asan

race_condition: race_condition.c
	$(CC) $(CFLAGS) $(PTHREAD) $< -o $@
	$(CC) $(CFLAGS) $(PTHREAD) -fsanitize=thread $< -o $@_tsan

infinite_loop: infinite_loop.c
	$(CC) $(CFLAGS) $(PTHREAD) $< -o $@

integer_overflow: integer_overflow.c
	$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) -fsanitize=undefined $< -o $@_ubsan

# Build with sanitizers
sanitizers: clean
	$(MAKE) all
	$(MAKE) use_after_free
	$(MAKE) race_condition
	$(MAKE) integer_overflow

# Run examples through NDB (for testing)
test-segfault: segfault_example
	@echo "Running segfault example through NDB..."
	@cd .. && ./target/main_interactive examples/segfault_example <<< "run\nquit\n"

# Run examples with tools
test-valgrind: memory_leak_example
	@echo "Running memory leak example with Valgrind..."
	valgrind --leak-check=full --error-exitcode=1 ./memory_leak_example

test-asan: use_after_free_asan
	@echo "Running use-after-free example with AddressSanitizer..."
	./use_after_free_asan || true

test-tsan: race_condition_tsan
	@echo "Running race condition example with ThreadSanitizer..."
	./race_condition_tsan || true

test-ubsan: integer_overflow_ubsan
	@echo "Running integer overflow example with UBSan..."
	./integer_overflow_ubsan || true

# Clean build artifacts
clean:
	rm -f $(EXAMPLES)
	rm -f *_asan *_tsan *_ubsan
	rm -f *.o
	rm -f vgcore.*

# Help target
help:
	@echo "NDB Debugger Examples - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build all examples"
	@echo "  make sanitizers   - Build examples with sanitizers"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make test-valgrind - Test with Valgrind"
	@echo "  make test-asan    - Test with AddressSanitizer"
	@echo "  make test-tsan    - Test with ThreadSanitizer"
	@echo "  make test-ubsan   - Test with UndefinedBehaviorSanitizer"
	@echo ""
	@echo "Examples:"
	@echo "  segfault_example     - Null pointer dereference"
	@echo "  memory_leak_example  - Memory leak demo"
	@echo "  use_after_free       - Use-after-free bug"
	@echo "  race_condition       - Threading race condition"
	@echo "  infinite_loop        - Various infinite loops"
	@echo "  integer_overflow     - Integer overflow bugs"
	@echo ""
	@echo "See README.md for detailed debugging guides."

.PHONY: all sanitizers clean help test-valgrind test-asan test-tsan test-ubsan
