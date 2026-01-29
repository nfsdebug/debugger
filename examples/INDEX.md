# NDB Debugger Examples - Complete Index

This is a complete index of all debugging examples available for the NDB debugger.

## Quick Navigation

- [Example Programs](#example-programs) - Source code with bugs
- [Debugging Guides](#debugging-guides) - Step-by-step debugging sessions
- [Documentation](#documentation) - README and reference materials
- [By Difficulty](#by-difficulty) - Examples sorted by skill level
- [By Bug Type](#by-bug-type) - Examples sorted by category

---

## Example Programs

### 1. Segmentation Fault Example
**File:** `segfault_example.c` (75 lines)

**Bug:** NULL pointer dereference

**Description:** A program that crashes when trying to access a structure through a NULL pointer. The `print_student()` function doesn't validate that the pointer is valid before dereferencing it.

**Symptoms:**
- Immediate crash with "Segmentation fault"
- Program exits with code 139

**Key Functions:**
- `create_student()` - Returns NULL on allocation failure
- `print_student()` - Crashes when given NULL pointer

**Lines of Code:** 75
**Difficulty:** Beginner

**Debugging Guide:** [segfault_debugging.md](segfault_debugging.md)

---

### 2. Memory Leak Example
**File:** `memory_leak_example.c` (113 lines)

**Bug:** Multiple memory leak patterns

**Description:** Demonstrates three types of memory leaks:
1. Forgetting to free returned pointers
2. Realloc in loop without proper cleanup
3. Early return without freeing in error path

**Symptoms:**
- Gradual increase in memory usage
- No crash (program completes successfully)
- Detected by Valgrind

**Key Functions:**
- `process_data()` - Allocates buffer, caller must free
- `grow_buffer_leak()` - Reallocates in loop
- `hidden_leak_example()` - Leak in error path

**Lines of Code:** 113
**Difficulty:** Intermediate

**Debugging Guide:** [memory_leak_debugging.md](memory_leak_debugging.md)

---

### 3. Use-After-Free Example
**File:** `use_after_free.c` (138 lines)

**Bug:** Accessing memory after it has been freed

**Description:** Shows various use-after-free scenarios:
1. Basic dangling pointer usage
2. Array of freed pointers
3. Inner pointer to freed memory
4. Double-free potential

**Symptoms:**
- Segmentation fault (if memory unmapped)
- Data corruption (if memory reused)
- Security vulnerability

**Key Functions:**
- `create_block()` - Allocates data structure
- `destroy_block()` - Frees structure
- `process_block_with_bug()` - Uses freed pointer

**Lines of Code:** 138
**Difficulty:** Intermediate

**Debugging Guide:** [use_after_free_debugging.md](use_after_free_debugging.md)

---

### 4. Race Condition Example
**File:** `race_condition.c` (201 lines)

**Bug:** Unsynchronized access to shared data

**Description:** Demonstrates race conditions in multithreaded code:
1. Counter increment without mutex
2. Check-then-act in bank withdrawal
3. Producer-consumer without synchronization

**Symptoms:**
- Non-deterministic results
- Lost increments
- Inconsistent state
- May not reproduce every time

**Key Functions:**
- `increment_counter()` - Race in increment
- `withdraw()` - Check-then-act race
- `producer()` / `consumer()` - Data race

**Lines of Code:** 201
**Difficulty:** Advanced
**Requires:** pthread library

**Debugging Guide:** [race_condition_debugging.md](race_condition_debugging.md)

---

### 5. Infinite Loop Example
**File:** `infinite_loop.c` (266 lines)

**Bug:** Various loop conditions that never become false

**Description:** Shows multiple infinite loop patterns:
1. Wrong loop condition (decrement instead of increment)
2. Off-by-one errors
3. Waiting for impossible condition
4. Thread deadlock (simplified)
5. Infinite recursion

**Symptoms:**
- Program appears frozen
- CPU usage at 100% (for tight loops)
- No response to input

**Key Functions:**
- `wrong_condition_loop()` - Decrementing counter
- `off_by_one_loop()` - Wrong loop bounds
- `waiting_forever()` - Waiting for flag that never sets
- `infinite_recursion()` - Stack overflow

**Lines of Code:** 266
**Difficulty:** Beginner to Intermediate
**Requires:** pthread library

**Debugging Guide:** [infinite_loop_debugging.md](infinite_loop_debugging.md)

---

### 6. Integer Overflow Example
**File:** `integer_overflow.c` (257 lines)

**Bug:** Arithmetic operations exceeding type limits

**Description:** Demonstrates various integer overflow scenarios:
1. Signed integer overflow
2. Unsigned wraparound
3. Allocation size overflow (security!)
4. Loop counter overflow
5. Signed/unsigned comparison
6. Overflow in average calculation

**Symptoms:**
- Incorrect calculations
- Buffer overflows (via size calculations)
- Logic errors
- Security vulnerabilities

**Key Functions:**
- `signed_overflow()` - Adding to INT_MAX
- `vulnerable_allocation()` - Size calculation overflow
- `calculate_average()` - Sum overflow

**Lines of Code:** 257
**Difficulty:** Intermediate

**Debugging Guide:** [integer_overflow_debugging.md](integer_overflow_debugging.md)

---

## Debugging Guides

Each example has a comprehensive debugging guide that includes:

### 1. Segfault Debugging Guide
**File:** `segfault_debugging.md` (173 lines)

**Contents:**
- Overview of NULL pointer dereference
- Compilation instructions
- Complete NDB debugging session
- Step-by-step analysis
- Root cause explanation
- Fix for the bug
- Prevention strategies

**Key Commands Demonstrated:**
- `backtrace` - Finding crash location
- `frame` - Examining call stack
- `print` - Checking variable values
- `list` - Viewing source code

---

### 2. Memory Leak Debugging Guide
**File:** `memory_leak_debugging.md` (347 lines)

**Contents:**
- Using Valgrind for leak detection
- Interpreting Valgrind output
- NDB techniques for understanding code flow
- Three types of leaks explained
- Prevention strategies
- Tool recommendations

**Key Tools:**
- Valgrind (primary)
- AddressSanitizer
- NDB (for understanding)

---

### 3. Use-After-Free Debugging Guide
**File:** `use_after_free_debugging.md` (420 lines)

**Contents:**
- Understanding dangling pointers
- AddressSanitizer output
- NDB breakpoint techniques
- Inner pointer dangers
- Double-free patterns
- Prevention with NULL assignment
- Real-world security impact

**Key Tools:**
- AddressSanitizer (best)
- NDB (for understanding)
- Valgrind (alternative)

---

### 4. Race Condition Debugging Guide
**File:** `race_condition_debugging.md` (427 lines)

**Contents:**
- Understanding data races
- ThreadSanitizer usage
- NDB thread examination
- Mutex synchronization
- Check-then-act patterns
- Producer-consumer issues
- Prevention strategies

**Key Tools:**
- ThreadSanitizer (best)
- NDB (limited value)
- Helgrind (alternative)

---

### 5. Infinite Loop Debugging Guide
**File:** `infinite_loop_debugging.md` (597 lines)

**Contents:**
- Interrupting execution (Ctrl-C)
- Examining loop variables
- Deadlock detection
- Stack overflow analysis
- Common patterns
- Prevention strategies
- Timeout mechanisms

**Key Techniques:**
- Ctrl-C to interrupt
- `backtrace` to see where stuck
- `watch` for variables
- `info threads` for all threads

---

### 6. Integer Overflow Debugging Guide
**File:** `integer_overflow_debugging.md` (573 lines)

**Contents:**
- Signed vs unsigned overflow
- Security implications
- UndefinedBehaviorSanitizer
- Safe arithmetic patterns
- Allocation size overflows
- Prevention strategies
- Real-world vulnerabilities

**Key Tools:**
- UndefinedBehaviorSanitizer
- Compiler warnings
- Static analysis

---

## Documentation

### Main README
**File:** `README.md` (349 lines)

**Contents:**
- Overview of all examples
- Difficulty ratings
- Learning objectives for each
- Compilation guide
- Recommended learning path
- Tool reference
- Common NDB commands
- Debugging workflow

---

### Quick Reference
**File:** `QUICK_REFERENCE.md` (271 lines)

**Contents:**
- Example summary table
- Quick start guide
- Common commands cheat sheet
- Bug pattern reference
- Compilation recipes
- Troubleshooting tips

---

### Summary Document
**File:** `EXAMPLES_SUMMARY.md`

**Contents:**
- Statistics on created content
- File organization
- Key highlights
- Testing results

---

## By Difficulty

### Beginner
1. **Segfault Example** - Learn basic debugging workflow
2. **Infinite Loop** - Learn to interrupt and examine state

**Skills Learned:**
- Using backtrace
- Setting breakpoints
- Examining variables
- Interrupting execution

---

### Intermediate
3. **Memory Leak** - Understanding heap and allocations
4. **Use-After-Free** - Memory lifecycle management
5. **Integer Overflow** - Undefined behavior detection

**Skills Learned:**
- Using Valgrind
- Using sanitizers (ASan, UBSan)
- Understanding memory management
- Detecting undefined behavior

---

### Advanced
6. **Race Condition** - Threading and synchronization

**Skills Learned:**
- Using ThreadSanitizer
- Understanding data races
- Mutex synchronization
- Debugging non-deterministic bugs

---

## By Bug Type

### Crash Bugs
- **Segfault Example** - Immediate crash
- **Use-After-Free** - Crash after free
- **Infinite Loop** - Stack overflow from recursion

**Primary Tool:** NDB

---

### Memory Errors
- **Memory Leak** - Unfreed allocations
- **Use-After-Free** - Dangling pointers

**Primary Tools:** Valgrind, AddressSanitizer

---

### Concurrency Issues
- **Race Condition** - Data races between threads

**Primary Tool:** ThreadSanitizer

---

### Logic Errors
- **Infinite Loop** - Wrong conditions
- **Integer Overflow** - Arithmetic overflow

**Primary Tools:** NDB, UndefinedBehaviorSanitizer

---

## Quick Start

### Build All Examples
```bash
cd /home/sbstndbs/debugger/examples
make
```

### Debug an Example
```bash
# From examples directory
../../target/main_interactive ./segfault_example

# Or from project root
./target/main_interactive examples/segfault_example
```

### Run with Sanitizers
```bash
# AddressSanitizer (use-after-free)
./use_after_free_asan

# ThreadSanitizer (race condition)
./race_condition_tsan

# UndefinedBehaviorSanitizer (integer overflow)
./integer_overflow_ubsan
```

### Run with Valgrind
```bash
valgrind --leak-check=full ./memory_leak_example
```

---

## File Statistics

**Source Code:**
- 6 example programs
- 1,070 total lines of C code
- Average: 178 lines per example

**Documentation:**
- 6 debugging guides (2,589 lines)
- 3 documentation files (887 lines)
- Total: 3,476 lines

**Combined:**
- 15 files
- 4,546 total lines
- Average: 303 lines per file

---

## Integration

The examples are fully integrated with the NDB project:

- **Main README** updated with link to examples
- **Makefile** for easy compilation
- **Debug symbols** included (-g flag)
- **Optimizations disabled** (-O0 flag)
- **Sanitizers** built for applicable examples

---

## Next Steps

1. **Start Learning** - Begin with segfault_example
2. **Follow Guides** - Read the debugging transcript
3. **Practice** - Try the examples yourself
4. **Experiment** - Make modifications and test
5. **Apply** - Use these techniques in real code

---

## Support

For issues or questions about the examples:
- Check the debugging guide for the specific example
- Review the main README
- Examine the QUICK_REFERENCE.md

---

**Happy Debugging!**

Remember: Every bug is a learning opportunity, and these examples provide practical, hands-on experience with real debugging scenarios using NDB.
