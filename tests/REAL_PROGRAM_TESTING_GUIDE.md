# Real-World Program Testing Guide for NDB Debugger

This guide provides comprehensive instructions for testing the NDB debugger on complex, real-world programs.

## Table of Contents

1. [Building Test Programs](#building-test-programs)
2. [Testing Complex Scenarios](#testing-complex-scenarios)
3. [Compiling Real Open-Source Programs](#compiling-real-open-source-programs)
4. [Debugging Real Programs with NDB](#debugging-real-programs-with-ndb)
5. [Example Debugging Sessions](#example-debugging-sessions)
6. [Known Limitations](#known-limitations)
7. [Bug Reporting](#bug-reporting)

---

## Building Test Programs

### New Complex Test Programs

The test suite now includes several complex test programs:

#### 1. test_complex.c
A comprehensive program featuring:
- **Recursive functions**: factorial, fibonacci, deep_recursion
- **Global and static variables**: global_counter, static_counter
- **Structs and pointers**: Point struct, PointList with dynamic allocation
- **Memory allocation**: malloc/free operations
- **Pointer arithmetic**: Array traversal, double pointers
- **Function pointers**: OperationFunc usage
- **Nested calls**: Deep call stack (level1 → level2 → level3)

**Build:**
```bash
cd /home/sbstndbs/debugger
gcc -g -O0 -o tests/binaries/test_complex tests/programs/test_complex.c
```

**Test scenarios:**
```bash
# Test recursion debugging
./target/main_interactive tests/binaries/test_complex
(ndb) break factorial
(ndb) continue
(ndb) backtrace
(ndb) print n

# Test struct inspection
(ndb) break create_point
(ndb) continue
(ndb) print p
(ndb) print p.x
(ndb) print p.y
(ndb) print p.name

# Test pointer inspection
(ndb) break swap_ints
(ndb) continue
(ndb) print a
(ndb) print *a

# Test watchpoints on globals
(ndb) watch global_counter
(ndb) continue
```

#### 2. test_signals.c
A program for testing signal handling:
- **SIGSEGV handling**: Null pointer dereference with recovery
- **SIGFPE handling**: Division by zero with recovery
- **SIGILL handling**: Illegal instruction (optional)
- **SIGUSR1 handling**: User-defined signals
- **Nested signal calls**: Signals in deep call stacks
- **Rapid signals**: Multiple signals in quick succession

**Build:**
```bash
gcc -g -O0 -o tests/binaries/test_signals tests/programs/test_signals.c
```

**Test scenarios:**
```bash
# Test all signals
./target/main_interactive tests/binaries/test_signals

# Test specific signal
./target/main_interactive tests/binaries/test_signals segv

# Debug signal handlers
(ndb) break sigsegv_handler
(ndb) break sigusr1_handler
(ndb) continue
(ndb) backtrace

# Test signal during execution
(ndb) break recursive_signal_test
(ndb) continue
(ndb) step
```

#### 3. test_threads.c
A multi-threaded program featuring:
- **Thread creation**: Multiple worker threads
- **Mutex synchronization**: Counter with mutex protection
- **Deep call stacks in threads**: Nested function calls
- **Recursive functions in threads**: Factorial computation
- **Static vs local variables**: Thread-local storage behavior
- **Signal handling in threads**: SIGUSR1 in thread context
- **Thread joining**: Waiting for completion

**Build:**
```bash
gcc -g -O0 -pthread -o tests/binaries/test_threads tests/programs/test_threads.c
```

**Test scenarios:**
```bash
# Test basic threading
./target/main_interactive tests/binaries/test_threads

# Test specific scenario
./target/main_interactive tests/binaries/test_threads mutex

# Debug thread functions
(ndb) break worker_thread
(ndb) break test_basic_threads
(ndb) continue
(ndb) backtrace
(ndb) info threads  # If supported

# Debug mutex contention
(ndb) break mutex_contention_thread
(ndb) print global_counter
(ndb) continue
```

---

## Testing Complex Scenarios

### Running the Complex Test Suite

```bash
cd /home/sbstndbs/debugger

# Run the comprehensive test suite
./tests/suites/test_real_programs.sh

# Expected output:
# - Tests for nested breakpoints
# - Tests for watchpoints on globals
# - Tests for complex backtraces
# - Tests for recursive stepping
# - Tests for struct/pointer inspection
# - Tests for malloc operations
# - Tests for function pointers
# - Tests for static variables
# - Tests for signal handlers
# - Tests for threads (if supported)
```

### Manual Test Scenarios

#### Scenario 1: Debugging Recursion
```bash
./target/main_interactive tests/binaries/test_complex
(ndb) break factorial
(ndb) break deep_recursion
(ndb) continue          # Hit first breakpoint in factorial
(ndb) backtrace         # See call stack
(ndb) step              # Step into recursive call
(ndb) backtrace         # See deeper stack
(ndb) print n           # Check parameter value
(ndb) continue          # Continue to next breakpoint
```

#### Scenario 2: Debugging Structs and Pointers
```bash
./target/main_interactive tests/binaries/test_complex
(ndb) break create_point
(ndb) continue
(ndb) print p           # Show entire struct
(ndb) print p.x         # Show member
(ndb) print p.name      # Show array member
(ndb) step
(ndb) break move_point
(ndb) continue
(ndb) print p->x        # Pointer to struct
(ndb) print p->y
(ndb) step
(ndb) print p->x        # Check modification
```

#### Scenario 3: Memory Allocation Tracking
```bash
./target/main_interactive tests/binaries/test_complex
(ndb) break create_point_list
(ndb) continue
(ndb) print list        # Should show address
(ndb) print list->count # Check initial value
(ndb) break add_point
(ndb) continue
(ndb) step
(ndb) print list->count # Should increment
(ndb) break free_point_list
(ndb) continue
(ndb) step              # Watch cleanup
```

#### Scenario 4: Signal Handler Debugging
```bash
./target/main_interactive tests/binaries/test_signals
(ndb) break sigsegv_handler
(ndb) continue
(ndb) backtrace         # See signal origin
(ndb) print sigsegv_count
(ndb) continue

# Test SIGUSR1
(ndb) break sigusr1_handler
(ndb) break test_sigusr1
(ndb) continue
(ndb) backtrace         # See call stack when signal arrives
```

#### Scenario 5: Multi-threaded Debugging
```bash
./target/main_interactive tests/binaries/test_threads
(ndb) break worker_thread
(ndb) break test_basic_threads
(ndb) continue
(ndb) backtrace         # May show thread context
(ndb) print global_counter
(ndb) continue

# Debug mutex contention
(ndb) break mutex_contention_thread
(ndb) continue
(ndb) print global_counter
(ndb) step              # Step through mutex operations
```

---

## Compiling Real Open-Source Programs

### General Guidelines

When compiling real programs for debugging with NDB:

1. **Required flags:**
   ```bash
   -g          # Include debug symbols
   -O0         # Disable optimization (for accurate debugging)
   ```

2. **Recommended flags:**
   ```bash
   -Wall       # Enable all warnings
   -Wextra     # Extra warnings
   -fno-omit-frame-pointer  # Preserve frame pointers
   ```

3. **Avoid:**
   ```bash
   -O1, -O2, -O3  # Optimizations can confuse debugger
   -s              # Strip symbols
   -fomit-frame-pointer  # Makes stack unwinding harder
   ```

### Example Compilations

#### 1. Redis (Key-Value Store)
```bash
# Download Redis
wget http://download.redis.io/redis-stable.tar.gz
tar xzf redis-stable.tar.gz
cd redis-stable

# Compile with debug symbols
make OPTIMIZATION=-O0 DEBUG=-g MALLOC=libc

# Test with NDB
./path/to/ndb ./src/redis-server
```

#### 2. SQLite (Database)
```bash
# Download SQLite
wget https://www.sqlite.org/2024/sqlite-autoconf-3450000.tar.gz
tar xzf sqlite-autoconf-3450000.tar.gz
cd sqlite-autoconf-3450000

# Configure and build
./configure CFLAGS="-g -O0 -Wall"
make

# Test with NDB
./path/to/ndb ./sqlite3 test.db
```

#### 3. tmux (Terminal Multiplexer)
```bash
# Download tmux
git clone https://github.com/tmux/tmux.git
cd tmux

# Build dependencies
# (On Ubuntu: sudo apt install libevent-dev libncurses-dev)

# Compile with debug symbols
./autogen.sh
./configure CFLAGS="-g -O0"
make

# Test with NDB
./path/to/ndb ./tmux new-session -s test
```

#### 4. coreutils (Standard Unix Tools)
```bash
# Download coreutils
wget https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz
tar xf coreutils-9.4.tar.xz
cd coreutils-9.4

# Configure and build
./configure CFLAGS="-g -O0"
make

# Test specific tools with NDB
./path/to/ndb ./src/ls -l
./path/to/ndb ./src/cat file.txt
./path/to/ndb ./src/wc -l file.txt
```

#### 5. lua (Lightweight Scripting Language)
```bash
# Download Lua
wget https://www.lua.org/ftp/lua-5.4.6.tar.gz
tar xzf lua-5.4.6.tar.gz
cd lua-5.4.6

# Modify Makefile for debug build
# Change: CFLAGS= -O2 +Wall -Wextra
# To:     CFLAGS= -O0 -g -Wall -Wextra

make linux
make test

# Test with NDB
./path/to/ndb ./src/lua test.lua
```

---

## Debugging Real Programs with NDB

### Starting a Debugging Session

```bash
# Basic usage
./target/main_interactive /path/to/program [args...]

# With program arguments
./target/main_interactive ./myprogram --input file.txt --verbose

# Attach to running process (if supported)
./target/main_interactive -p <PID>
```

### Common Debugging Workflow

#### 1. Initial Exploration
```
(ndb) help                    # List available commands
(ndb) symbols                 # List all symbols
(ndb) list main               # Show main function source
(ndb) disas main              # Disassemble main function
```

#### 2. Setting Breakpoints
```
(ndb) break main              # Break at function entry
(ndb) break my_function       # Break at specific function
(ndb) break 0x400500          # Break at address
(ndb) break file.c:42         # Break at line (if supported)
```

#### 3. Controlling Execution
```
(ndb) run                     # Start execution
(ndb) continue                # Continue after breakpoint
(ndb) step                    # Step into function
(ndb) next                    # Step over function
(ndb) finish                  # Continue until function returns
```

#### 4. Inspecting State
```
(ndb) backtrace               # Show call stack
(ndb) print variable          # Print variable value
(ndb) print *pointer          # Dereference pointer
(ndb) print struct.member     # Access struct member
(ndb) print array[0]          # Access array element
```

#### 5. Memory and Registers
```
(ndb) registers               # Show all registers
(ndb) register rip            # Show specific register
(ndb) x/10x 0x7fffffffd000   # Examine memory
(ndb) watch global_var        # Set watchpoint
```

---

## Example Debugging Sessions

### Example 1: Finding a Segmentation Fault

**Scenario:** A program crashes with SIGSEGV.

```bash
$ ./target/main_interactive ./crashing_program
(ndb) run
Program starting...
...
Signal SIGSEGV received
(ndb) backtrace
#0  0x0000000000401136 in process_data (ptr=0x0) at data.c:45
#1  0x0000000000401098 in main (argc=1, argv=0x7fffffffdda8) at main.c:23
#2  0x00007ffff7a2e0b3 in __libc_start_main ...
#3  0x000000000040102e in _start ...

(ndb) frame 0
(ndb) print ptr
$1 = (int *) 0x0

(ndb) list 40,50
40      int process_data(int* ptr) {
41          if (ptr == NULL) {
42              return -1;
43          }
44          // Bug: Missing NULL check before dereference
45          return *ptr + 10;  <-- Crash here
46      }
```

**Finding:** The function receives a NULL pointer and dereferences it without checking.

### Example 2: Debugging Memory Leak

**Scenario:** A program leaks memory over time.

```bash
$ ./target/main_interactive ./leaky_program
(ndb) break allocate_memory
(ndb) commands
> print size
> print total_allocated
> continue
> end

(ndb) break free_memory
(ndb) commands
> print ptr
> print total_allocated
> continue
> end

(ndb) continue

# Watch allocation patterns:
allocate_memory called, size=1024, total_allocated=1024
allocate_memory called, size=2048, total_allocated=3072
free_memory called, ptr=0x55555555a2a0, total_allocated=2048
allocate_memory called, size=4096, total_allocated=6144
allocate_memory called, size=8192, total_allocated=14336
# Notice: Not all allocations are freed!
```

### Example 3: Tracing Recursive Function

**Scenario:** Understanding recursion in a complex algorithm.

```bash
$ ./target/main_interactive ./quicksort_program
(ndb) break quicksort
(ndb) commands
> print array
> print low
> print high
> backtrace 5
> continue
> end

(ndb) continue

# Output shows recursive calls:
quicksort: array=0x7fffffffdc20, low=0, high=9
  Call stack:
  #0  quicksort (low=0, high=9)
  #1  main ...

quicksort: array=0x7fffffffdc20, low=0, high=4
  Call stack:
  #0  quicksort (low=0, high=4)
  #1  quicksort (low=0, high=9)
  #2  main ...

quicksort: array=0x7fffffffdc20, low=0, high=1
  Call stack:
  #0  quicksort (low=0, high=1)
  #1  quicksort (low=0, high=4)
  #2  quicksort (low=0, high=9)
  #3  main ...
```

### Example 4: Debugging Multi-threaded Race Condition

**Scenario:** Two threads accessing shared data incorrectly.

```bash
$ ./target/main_interactive ./race_condition_program
(ndb) break increment_counter
(ndb) commands
> thread info
> print counter
> print thread_id
> continue
> end

(ndb) continue

# Observe thread interleaving:
Thread 1: counter=100
Thread 2: counter=101
Thread 1: counter=102  <-- May see lost updates!
Thread 2: counter=103

# Set watchpoint to catch concurrent access:
(ndb) watch counter
(ndb) continue
# Hit when counter is modified by any thread
```

### Example 5: Signal Handler Investigation

**Scenario:** Investigating unexpected signal.

```bash
$ ./target/main_interactive ./signal_program
(ndb) break signal_handler
(ndb) break main
(ndb) continue

# Program runs and hits signal_handler
(ndb) backtrace
#0  signal_handler (sig=11) at signals.c:123
#1  <signal handler called>
#2  0x0000000000401123 in buggy_function at main.c:67
#3  0x00000000004010a0 in process_data at main.c:45
#4  0x0000000000401056 in main at main.c:23

(ndb) frame 2
(ndb) print x
$1 = 0
(ndb) print y
$2 = 0
(ndb) list 65,70
65      int buggy_function() {
66          int x = 10;
67          int y = x / 0;  <-- Division by zero!
68          return y;
69      }
```

---

## Known Limitations

Based on testing with complex programs, NDB currently has these limitations:

### 1. Threading Support
- **Status**: Experimental/Limited
- **Limitations**:
  - May not show all threads
  - `info threads` command may not be available
  - Thread-specific breakpoints may not work reliably
  - Cannot switch between thread contexts

### 2. Signal Handling
- **Status**: Partial
- **Limitations**:
  - Cannot catch SIGKILL (cannot be caught)
  - Signal handler breakpoints may be unreliable
  - Async signal safety not guaranteed

### 3. C++ Support
- **Status**: Not tested/Unknown
- **Limitations**:
  - Name demangling may not work
  - Template debugging may fail
  - Virtual function call inspection limited

### 4. Optimized Code
- **Status**: Not recommended
- **Limitations**:
  - Variables may be optimized out
  - Call stack may be inaccurate
  - Instruction stepping may not match source

### 5. Large Programs
- **Status**: Unknown limits
- **Potential Issues**:
  - Symbol loading may be slow for large binaries
  - Memory usage may be high
  - Performance may degrade

### 6. Advanced Features
- **Not Implemented**:
  - Conditional breakpoints
  - Catchpoint on exceptions
  - Reverse debugging
  - Record/replay
  - Remote debugging

---

## Bug Reporting

When you find bugs while testing real programs, please report them with:

### Required Information

1. **Program Information:**
   - Name and version of the program being debugged
   - How to compile it with debug symbols
   - Minimal reproduction case

2. **NDB Version:**
   ```bash
   cd /home/sbstndbs/debugger
   git log -1 --oneline
   git describe --always
   ```

3. **System Information:**
   ```bash
   uname -a
   lsb_release -a  # or cat /etc/os-release
   ```

4. **Steps to Reproduce:**
   - Exact commands used
   - Program input if any
   - Expected behavior
   - Actual behavior

5. **Debug Output:**
   - Complete NDB session output
   - Any error messages
   - Backtrace if NDB crashes

6. **Test Case:**
   - Minimal code that reproduces the issue
   - Compilation command
   - NDB commands that trigger the bug

### Bug Report Template

```markdown
## Bug Description

Brief description of the bug.

## Reproduction Steps

1. Compile program: `gcc -g -O0 -o test test.c`
2. Run NDB: `./target/main_interactive ./test`
3. Commands: `break main`, `continue`, `print variable`
4. Error: ...

## Expected Behavior

What should happen.

## Actual Behavior

What actually happens (include error messages).

## Environment

- NDB version: [commit hash]
- OS: [distribution and version]
- Compiler: [version]
- Program being debugged: [name and version]

## Minimal Test Case

\`\`\`c
// Minimal code that reproduces the bug
#include <stdio.h>

int main() {
    // ...
}
\`\`\`

## Additional Context

Logs, screenshots, or other relevant information.
```

### Where to Report

- Create issue in the project repository
- Tag with `bug`, `real-program-testing`
- Include `test-complex` or `test-signals` output if relevant

---

## Test Execution Checklist

Use this checklist when testing NDB on real programs:

- [ ] Program compiles with `-g -O0`
- [ ] NDB can load the program
- [ ] Symbols are loaded correctly (`symbols` command)
- [ ] Can set breakpoint on `main`
- [ ] Can run the program (`run` or `continue`)
- [ ] Can inspect local variables (`print` command)
- [ ] Can inspect global variables
- [ ] Can inspect pointers and dereference
- [ ] Can inspect structs and struct members
- [ ] Can inspect arrays
- [ ] Backtrace shows correct call stack
- [ ] Step/next commands work correctly
- [ ] Can set watchpoints
- [ ] Watchpoints trigger correctly
- [ ] Disassembly shows correct instructions
- [ ] Can handle signals (SIGSEGV, SIGFPE, etc.)
- [ ] Can debug multiple threads (if applicable)
- [ ] Can debug recursive functions
- [ ] Can debug dynamic memory allocation
- [ ] Program runs to completion without NDB crashes
- [ ] NDB quits cleanly

---

## Summary

This guide provides:

1. **New test programs** for complex scenarios:
   - `test_complex.c`: Recursion, structs, pointers, malloc
   - `test_signals.c`: Signal handling with recovery
   - `test_threads.c`: Multi-threaded programs

2. **Comprehensive test suite** (`test_real_programs.sh`):
   - 19 test scenarios for complex programs
   - Tests for nested calls, recursion, structs, pointers
   - Tests for signals and threading

3. **Compilation guidelines** for real programs:
   - Proper compiler flags
   - Examples for Redis, SQLite, tmux, etc.

4. **Debugging workflows** and examples:
   - Finding segmentation faults
   - Debugging memory leaks
   - Tracing recursion
   - Race conditions
   - Signal handlers

5. **Bug reporting guidelines**:
   - Required information
   - Report template
   - Test checklist

Use these resources to thoroughly test NDB on increasingly complex programs and identify any limitations or bugs.
