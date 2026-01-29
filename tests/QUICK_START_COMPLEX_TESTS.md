# Quick Start Guide - Complex Test Programs

This guide helps you quickly start using the new complex test programs for NDB debugger testing.

## Quick Reference

### Build All Test Programs

```bash
cd /home/sbstndbs/debugger

# Build test_complex
gcc -g -O0 -o tests/binaries/test_complex tests/programs/test_complex.c

# Build test_signals
gcc -g -O0 -o tests/binaries/test_signals tests/programs/test_signals.c

# Build test_threads
gcc -g -O0 -pthread -o tests/binaries/test_threads tests/programs/test_threads.c
```

### Run Test Programs (Without Debugger)

```bash
# Test complex features
./tests/binaries/test_complex

# Test signal handling
./tests/binaries/test_signals          # All signals
./tests/binaries/test_signals segv     # Only SIGSEGV
./tests/binaries/test_signals fpe      # Only SIGFPE

# Test threading
./tests/binaries/test_threads          # All tests
./tests/binaries/test_threads basic    # Basic threading
./tests/binaries/test_threads mutex    # Mutex contention
```

### Run Test Suite

```bash
# Run comprehensive test suite
./tests/suites/test_real_programs.sh

# Expected: 17/20 tests pass (85%)
```

### Debug with NDB

```bash
# Start NDB on test program
./target/main_interactive tests/binaries/test_complex

# Example session
(ndb) break factorial          # Set breakpoint
(ndb) continue                 # Run to breakpoint
(ndb) backtrace                # Show call stack (may not work)
(ndb) print n                  # Print variable
(ndb) step                     # Step into function
(ndb) next                     # Step over function
(ndb) continue                 # Continue execution
(ndb) quit                     # Exit debugger
```

## Test Program Features

### test_complex.c

**Key Functions for Breakpoints:**
- `factorial(int n)` - Recursive factorial (depth: n)
- `fibonacci(int n)` - Recursive fibonacci (depth: n)
- `deep_recursion(int depth)` - Deep recursion (depth: 50)
- `swap_ints(int* a, int* b)` - Pointer swap
- `create_point(int x, int y, const char* name)` - Struct creation
- `move_point(Point* p, int dx, int dy)` - Struct pointer manipulation
- `create_point_list(int capacity)` - Memory allocation
- `add_point(PointList* list, Point p)` - Add to dynamic array
- `free_point_list(PointList* list)` - Memory cleanup
- `level1(int x)` - Nested call (level1→level2→level3)

**Global Variables:**
- `global_counter` - Global counter (use for watchpoints)
- `static_counter` - Static counter

**Test Scenarios:**
1. Factorial recursion (factorial(5) = 120)
2. Fibonacci recursion (fibonacci(10) = 55)
3. Deep recursion (depth=50)
4. Pointer swap (swap &a, &b)
5. Struct operations (create and move points)
6. Memory allocation (create list, add points, free)
7. Function pointers (add and multiply operations)
8. Nested calls (level1→level2→level3)
9. Pointer arithmetic (array traversal)
10. Double pointers (pointer to pointer)

**Example Debugging Session:**

```bash
./target/main_interactive tests/binaries/test_complex

# Test recursion
(ndb) break factorial
(ndb) continue
(ndb) print n              # Should show 5
(ndb) step                 # Step into recursive call
(ndb) print n              # Should show 4
(ndb) backtrace            # Should show call stack

# Test structs
(ndb) break create_point
(ndb) continue
(ndb) print p              # Show entire struct
(ndb) print p.x            # Show member
(ndb) print p.name         # Show array member

# Test pointers
(ndb) break swap_ints
(ndb) continue
(ndb) print a              # Show pointer address
(ndb) print *a             # Dereference

# Test watchpoints (if implemented)
(ndb) watch global_counter
(ndb) continue             # Should stop when counter changes
```

### test_signals.c

**Key Functions for Breakpoints:**
- `sigsegv_handler(int sig)` - SIGSEGV handler
- `sigfpe_handler(int sig)` - SIGFPE handler
- `sigusr1_handler(int sig)` - SIGUSR1 handler
- `cause_sigsegv()` - Triggers SIGSEGV
- `cause_sigfpe()` - Triggers SIGFPE (division by zero)
- `cause_sigill()` - Triggers SIGILL (illegal instruction)
- `recursive_signal_test(int depth)` - Signals in deep call stack

**Global Variables:**
- `sigsegv_count` - SIGSEGV counter
- `sigfpe_count` - SIGFPE counter
- `sigill_count` - SIGILL counter
- `sigusr1_count` - SIGUSR1 counter
- `signal_received` - Last signal received

**Test Scenarios:**
1. SIGSEGV (null pointer dereference with recovery)
2. SIGFPE (division by zero with recovery)
3. SIGUSR1 (user-defined signal)
4. Rapid signals (5 signals in quick succession)
5. Nested signals (signals in deep call stacks)
6. Signal ignore/restore
7. Signals during execution

**Example Debugging Session:**

```bash
./target/main_interactive tests/binaries/test_signals

# Test SIGSEGV handler
(ndb) break sigsegv_handler
(ndb) continue
(ndb) backtrace            # See where SIGSEGV occurred
(ndb) print sigsegv_count  # See how many times caught

# Test SIGFPE handler
(ndb) break sigfpe_handler
(ndb) break cause_sigfpe
(ndb) continue
(ndb) print sigfpe_count

# Test rapid signals
(ndb) break sigusr1_handler
(ndb) continue
(ndb) print sigusr1_count  # Should increment rapidly

# Test nested signals
(ndb) break recursive_signal_test
(ndb) continue
(ndb) backtrace            # See deep call stack
```

### test_threads.c

**Key Functions for Breakpoints:**
- `worker_thread(void* arg)` - Basic worker thread
- `test_basic_threads()` - Thread creation test
- `factorial_thread_func(void* arg)` - Factorial in thread
- `deep_stack_thread(void* arg)` - Deep call stack in thread
- `mutex_contention_thread(void* arg)` - Mutex contention
- `static_thread(void* arg)` - Static vs local variables

**Global Variables:**
- `global_counter` - Thread-safe counter (with mutex)
- `thread_created` - Thread creation counter
- `thread_started` - Thread start counter
- `thread_finished` - Thread completion counter

**Test Scenarios:**
1. Basic thread creation (3 threads)
2. Deep call stacks in threads
3. Factorial computation in threads (5 threads)
4. Mutex contention (4 threads competing)
5. Static vs local variables (2 threads)
6. Signals in threads
7. Many threads (10 threads)
8. Waiting threads (different timeouts)

**Example Debugging Session:**

```bash
./target/main_interactive tests/binaries/test_threads basic

# Test thread creation
(ndb) break worker_thread
(ndb) continue
(ndb) backtrace            # May show thread context
(ndb) print data->thread_id
(ndb) print global_counter

# Test mutex contention
./target/main_interactive tests/binaries/test_threads mutex
(ndb) break mutex_contention_thread
(ndb) continue
(ndb) print global_counter  # Should see changes
(ndb) step                 # Step through mutex ops

# Test static vs local
./target/main_interactive tests/binaries/test_threads static
(ndb) break static_thread
(ndb) continue
(ndb) print static_var     # Static (shared)
(ndb) print local_var      # Local (per-thread)
```

## Common Debugging Workflows

### Workflow 1: Find Segmentation Fault

```bash
./target/main_interactive tests/binaries/test_signals segv

(ndb) run                  # Start program
# Program will hit SIGSEGV
(ndb) backtrace            # See where it occurred
(ndb) print null_ptr       # See the null pointer
(ndb) frame 0              # Go to frame 0
(ndb) list                 # See source code
```

### Workflow 2: Debug Recursion

```bash
./target/main_interactive tests/binaries/test_complex

(ndb) break deep_recursion
(ndb) continue
(ndb) print depth          # See current depth
(ndb) step                 # Step into recursive call
(ndb) print depth          # Should be depth-1
(ndb) backtrace            # See call stack depth
```

### Workflow 3: Track Memory Leak

```bash
./target/main_interactive tests/binaries/test_complex

(ndb) break create_point_list
(ndb) commands
> print "Allocating list"
> continue
> end

(ndb) break add_point
(ndb) commands
> print "Adding point"
> print list->count
> continue
> end

(ndb) break free_point_list
(ndb) commands
> print "Freeing list"
> continue
> end

(ndb) continue
# Watch allocation/deallocation pattern
```

### Workflow 4: Debug Race Condition

```bash
./target/main_interactive tests/binaries/test_threads mutex

(ndb) break mutex_contention_thread
(ndb) commands
> print "Thread"
> print data->thread_id
> print global_counter
> continue
> end

(ndb) continue
# Watch thread interleaving
# See when threads acquire/release mutex
```

## Test Suite Summary

| Test Program | Size | Features | Tests |
|-------------|------|----------|-------|
| test_complex | 21KB | Recursion, structs, pointers, malloc | 10 scenarios |
| test_signals | 28KB | Signal handling with recovery | 8 scenarios |
| test_threads | 35KB | Multi-threading, mutex, signals | 8 scenarios |

**Total Test Scenarios:** 26
**Test Suite Pass Rate:** 85% (17/20)

## Known Issues

1. **Backtrace not available** - Requires libunwind-ptrace (not in repo)
2. **Watchpoints not implemented** - `watch` command not recognized
3. **Nested functions not found** - Some static functions missing from symbols
4. **Thread support limited** - Basic breakpoint support only
5. **Print command limited** - Complex expressions may not work

## Tips for Effective Testing

1. **Start Simple:** Begin with `test_complex` to learn basics
2. **Use Breakpoints:** Set breakpoints before running
3. **Check Variables:** Use `print` frequently to verify state
4. **Watch Recursion:** Use recursion tests to verify call stack
5. **Test Signals:** Use signal tests to verify error handling
6. **Thread Carefully:** Thread support is experimental
7. **Report Bugs:** Document any issues you find

## Getting Help

- **Command Help:** Type `help` in NDB
- **Full Guide:** See `REAL_PROGRAM_TESTING_GUIDE.md`
- **Test Results:** See `TESTING_REPORT.md`
- **Bug Reports:** Document issues in TESTING_REPORT.md format

## Next Steps

1. Run all test programs to ensure they work
2. Run the test suite to see current capabilities
3. Try debugging each scenario manually
4. Explore features not covered by tests
5. Report any bugs or limitations found

Happy Debugging!
