# Quick Reference - Debugging Examples

This is a quick reference for the debugging examples. See [README.md](README.md) for detailed guides.

## Example Summary

| Example | Difficulty | Bug Type | Tool Used | Key Lesson |
|---------|-----------|----------|-----------|------------|
| Segfault | Beginner | NULL pointer | NDB | Use backtrace to find crash |
| Memory Leak | Intermediate | Unfreed malloc | Valgrind | Every malloc needs a free |
| Use-After-Free | Intermediate | Dangling pointer | ASan | Set pointers to NULL after free |
| Race Condition | Advanced | Data race | TSan | Use mutexes for shared data |
| Infinite Loop | Beginner/Inter | Wrong condition | NDB Ctrl-C | Interrupt to see where stuck |
| Integer Overflow | Intermediate | Arithmetic overflow | UBSan | Check for overflow before ops |

## Quick Start

### Build All Examples
```bash
cd /home/sbstndbs/debugger/examples
make
```

### Build with Sanitizers
```bash
make sanitizers
```

### Debug with NDB
```bash
# From examples directory
../../target/main_interactive ./segfault_example

# From project root
./target/main_interactive examples/segfault_example
```

### Run with Tools
```bash
# Memory leak detection
valgrind --leak-check=full ./memory_leak_example

# Use-after-free detection
./use_after_free_asan

# Race condition detection
./race_condition_tsan

# Integer overflow detection
./integer_overflow_ubsan
```

## Common Commands

### NDB Commands
```
run              - Start program
Ctrl-C           - Interrupt execution
backtrace        - Show call stack
frame <n>        - Select stack frame
print <var>      - Show variable value
list             - Show source code
break <loc>      - Set breakpoint
step             - Step into function
next             - Step over function
continue         - Continue execution
```

### Valgrind Commands
```bash
# Memory leak check
valgrind --leak-check=full ./program

# With more details
valgrind --leak-check=full --show-leak-kinds=all ./program

# Track origins of uninitialized values
valgrind --track-origins=yes ./program
```

### Sanitizer Flags
```bash
# AddressSanitizer (memory errors)
gcc -fsanitize=address program.c

# UndefinedBehaviorSanitizer (integer overflow)
gcc -fsanitize=undefined program.c

# ThreadSanitizer (race conditions)
gcc -fsanitize=thread -pthread program.c

# All sanitizers combined
gcc -fsanitize=address,undefined,thread program.c
```

## Bug Patterns

### Segmentation Fault
**Symptom:** Program crashes with "Segmentation fault"

**Steps:**
1. Run under NDB
2. Type `run` to start
3. At crash, type `backtrace`
4. Examine the top frame
5. Check for NULL pointers

**Common Causes:**
- NULL pointer dereference
- Dangling pointer
- Buffer overflow
- Stack overflow

### Memory Leak
**Symptom:** Memory usage grows over time

**Steps:**
1. Run with Valgrind
2. Look for "definitely lost" blocks
3. Note allocation location
4. Find matching free() or add one

**Common Causes:**
- Forgetting to free()
- Lost pointer references
- Early returns in error paths
- Realloc without freeing

### Use-After-Free
**Symptom:** Crash or corruption after free()

**Steps:**
1. Run with AddressSanitizer
2. Look for "heap-use-after-free"
3. Note where memory was freed
4. Find where it's accessed after free

**Common Causes:**
- Using pointer after free()
- Saving inner pointers
- Double-free
- Returning pointers to local stack

### Race Condition
**Symptom:** Non-deterministic behavior

**Steps:**
1. Run with ThreadSanitizer
2. Look for "data race"
3. Note the variable and threads involved
4. Add mutex synchronization

**Common Causes:**
- Unsynchronized shared data
- Check-then-act without lock
- Non-atomic operations
- Wrong lock ordering

### Infinite Loop
**Symptom:** Program appears frozen

**Steps:**
1. Run under NDB
2. Press Ctrl-C to interrupt
3. Type `backtrace`
4. Examine loop variables
5. Check exit condition

**Common Causes:**
- Wrong loop condition
- Modifying wrong variable
- Waiting for impossible event
- Thread deadlock
- Unsigned wrap-around

### Integer Overflow
**Symptom:** Wrong calculations or crashes

**Steps:**
1. Run with UBSan
2. Look for "signed integer overflow"
3. Check arithmetic operations
4. Add overflow checks

**Common Causes:**
- Adding large numbers
- Multiplication overflow
- Size calculations
- Signed/unsigned comparison

## Compilation

### Basic (for debugging)
```bash
gcc -g -O0 example.c -o example
```

### With warnings
```bash
gcc -Wall -Wextra -g -O0 example.c -o example
```

### With pthread (threading)
```bash
gcc -Wall -Wextra -g -O0 -pthread example.c -o example
```

### With sanitizers
```bash
# AddressSanitizer
gcc -g -O0 -fsanitize=address example.c -o example

# UndefinedBehaviorSanitizer
gcc -g -O0 -fsanitize=undefined example.c -o example

# ThreadSanitizer
gcc -g -O0 -fsanitize=thread -pthread example.c -o example
```

## Testing Your Understanding

After studying an example, try these exercises:

1. **Segfault**: Add another NULL pointer bug and find it
2. **Memory Leak**: Fix the leaks and verify with Valgrind
3. **Use-After-Free**: Add NULL assignment after free
4. **Race Condition**: Add mutex to fix the race
5. **Infinite Loop**: Fix the loop conditions
6. **Integer Overflow**: Add overflow checks

## Troubleshooting

### "No such file or directory"
Make sure you're in the examples directory:
```bash
cd /home/sbstndbs/debugger/examples
```

### "Permission denied"
Make sure executables have proper permissions:
```bash
chmod +x segfault_example
```

### NDB not found
Build the project first:
```bash
cd /home/sbstndbs/debugger
make
```

### Sanitizer not working
Check GCC version (4.9+ required):
```bash
gcc --version
```

## Further Reading

- [Main README](../README.md) - Project overview
- [Example README](README.md) - Detailed guides
- [NDB Commands](../docs/commands.md) - All commands

## Tips

1. **Start simple** - Use basic examples first
2. **Follow the transcript** - Reproduce exactly
3. **Experiment** - Try variations and see what happens
4. **Use tools** - Let sanitizers find bugs for you
5. **Take notes** - Document what you learn
6. **Be patient** - Debugging is a skill that develops with practice
