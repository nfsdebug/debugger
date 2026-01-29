# NDB Debugger - Real-World Debugging Examples

This directory contains practical debugging examples demonstrating common C/C++ bugs and how to find them using the NDB debugger.

## Overview

Each example includes:
- **Buggy source code** - Realistic programs with actual bugs
- **Debugging transcript** - Complete NDB session showing how to find the bug
- **Analysis** - Explanation of what went wrong
- **Fix** - How to correct the bug

## Examples

### 1. Segmentation Fault (Null Pointer Dereference)
**Files:** `segfault_example.c`, `segfault_debugging.md`

**Difficulty:** Beginner

**What you'll learn:**
- How to analyze a segfault crash
- Using backtrace to find the crash location
- Examining function parameters and variables
- Identifying NULL pointer dereferences

**Key NDB commands:**
- `backtrace` or `bt` - Show call stack
- `frame <n>` - Switch stack frame
- `print <var>` - Show variable values
- `list` - View source code

---

### 2. Memory Leak
**Files:** `memory_leak_example.c`, `memory_leak_debugging.md`

**Difficulty:** Intermediate

**What you'll learn:**
- How to identify memory leaks with Valgrind
- Understanding heap allocation patterns
- Common leak patterns (forgotten free, early returns, realloc)
- Tracking allocations and frees

**Tools used:**
- Valgrind (primary tool for leak detection)
- NDB (for understanding code flow)
- AddressSanitizer (alternative)

**Key concepts:**
- Every malloc needs a free
- Check error paths for cleanup
- Use tools regularly in development

---

### 3. Use-After-Free
**Files:** `use_after_free.c`, `use_after_free_debugging.md`

**Difficulty:** Intermediate

**What you'll learn:**
- Understanding dangling pointers
- How use-after-free causes crashes and corruption
- Using AddressSanitizer for detection
- Preventing use-after-free with proper cleanup

**Key NDB commands:**
- Setting breakpoints on free()
- Examining pointer values after free
- Watching memory addresses

**Prevention:**
- Set pointers to NULL after free
- Copy data you need to keep
- Use smart pointers (C++) or safe wrappers (C)

---

### 4. Race Condition
**Files:** `race_condition.c`, `race_condition_debugging.md`

**Difficulty:** Advanced

**What you'll learn:**
- How to debug threading issues
- Understanding data races
- Using ThreadSanitizer for detection
- Mutex synchronization patterns

**Tools used:**
- ThreadSanitizer (best for race detection)
- NDB (for examining thread states)
- Helgrind (alternative)

**Key NDB commands:**
- `info threads` - See all threads
- `thread <n>` - Switch to specific thread
- `thread apply all backtrace` - Backtrace all threads
- `watchpoint <var>` - Watch shared variables

**Note:** Race conditions are non-deterministic and hard to reproduce. ThreadSanitizer is much more effective than a debugger.

---

### 5. Infinite Loop
**Files:** `infinite_loop.c`, `infinite_loop_debugging.md`

**Difficulty:** Beginner to Intermediate

**What you'll learn:**
- How to debug programs that appear frozen
- Interrupting execution with Ctrl-C
- Examining loop variables and conditions
- Detecting deadlocks

**Key NDB commands:**
- Ctrl-C to interrupt
- `backtrace` - See where program is stuck
- `info threads` - Check all thread states
- `watch <var>` - Watch loop variables

**Common causes:**
- Wrong loop condition
- Modifying wrong variable
- Waiting for impossible condition
- Thread deadlock
- Infinite recursion

---

### 6. Integer Overflow
**Files:** `integer_overflow.c`, `integer_overflow_debugging.md`

**Difficulty:** Intermediate

**What you'll learn:**
- How integer overflow causes bugs
- Security implications (buffer overflows)
- Using UndefinedBehaviorSanitizer
- Safe arithmetic patterns

**Tools used:**
- UndefinedBehaviorSanitizer (UBSan)
- NDB (for examining values)
- Compiler warnings

**Key concepts:**
- Signed overflow is undefined behavior
- Unsigned overflow wraps around
- Size calculations can overflow (security risk!)
- Mixed signed/unsigned comparisons

---

## How to Use These Examples

### For Each Example:

1. **Read the buggy source code**
   ```bash
   cat segfault_example.c
   ```

2. **Compile with debug symbols**
   ```bash
   gcc -g -O0 segfault_example.c -o segfault_example
   ```

3. **Run under NDB**
   ```bash
   ../../target/main_interactive ./segfault_example
   ```

4. **Follow the debugging transcript**
   - Use the commands shown in the debugging guide
   - Compare your output to the transcript
   - Understand what each command tells you

5. **Read the analysis**
   - Understand why the bug occurred
   - Learn how to fix it
   - Study the prevention strategies

6. **Apply the fix**
   - Modify the source code
   - Recompile and verify the fix works

## Compilation Guide

### Basic Debug Build
```bash
gcc -g -O0 <example>.c -o <example>
```

### With Sanitizers (Recommended!)

```bash
# AddressSanitizer (memory errors, leaks, use-after-free)
gcc -g -O0 -fsanitize=address <example>.c -o <example>

# UndefinedBehaviorSanitizer (integer overflows, etc.)
gcc -g -O0 -fsanitize=undefined <example>.c -o <example>

# ThreadSanitizer (race conditions)
gcc -g -O0 -fsanitize=thread -pthread <example>.c -o <example>
```

### With Pthread (for race_condition.c)
```bash
gcc -g -O0 -pthread race_condition.c -o race_condition
```

## Recommended Learning Path

### Beginner
1. **Segfault Example** - Learn basic debugging workflow
2. **Infinite Loop** - Learn to interrupt and examine state

### Intermediate
3. **Memory Leak** - Learn to use Valgrind and understand heap
4. **Use-After-Free** - Understand memory lifecycle and dangling pointers
5. **Integer Overflow** - Learn about undefined behavior and sanitizers

### Advanced
6. **Race Condition** - Learn threading issues and ThreadSanitizer

## Tools Reference

### NDB Debugger
- **Project:** This debugger (NDB)
- **Use for:** Controlling execution, examining state
- **Best for:** Crashes, hangs, logic errors

### Valgrind
- **Install:** `sudo apt install valgrind`
- **Use for:** Memory leaks, invalid memory access
- **Command:** `valgrind --leak-check=full ./program`

### AddressSanitizer (ASan)
- **Flag:** `-fsanitize=address`
- **Use for:** Memory errors, use-after-free, buffer overflows
- **Best:** Fast and comprehensive

### UndefinedBehaviorSanitizer (UBSan)
- **Flag:** `-fsanitize=undefined`
- **Use for:** Integer overflows, misaligned accesses
- **Best:** Catches undefined behavior

### ThreadSanitizer (TSan)
- **Flag:** `-fsanitize=thread`
- **Use for:** Data races, deadlocks
- **Best:** Threading issues

### Helgrind
- **Tool:** `valgrind --tool=helgrind`
- **Use for:** Race conditions
- **Alternative:** ThreadSanitizer is better

## Common NDB Commands

### Execution Control
- `run` - Start program
- `continue` or `c` - Continue execution
- `step` or `s` - Step into functions
- `next` or `n` - Step over functions
- `finish` - Run until current function returns

### Breakpoints
- `break <function>` - Set breakpoint at function
- `break <file>:<line>` - Set breakpoint at line
- `info breakpoints` - List breakpoints
- `delete <n>` - Remove breakpoint

### Information
- `backtrace` or `bt` - Show call stack
- `frame <n>` - Select stack frame
- `info registers` - Show CPU registers
- `info threads` - Show all threads
- `list` - Show source code

### Data Examination
- `print <expr>` - Evaluate and print expression
- `print/x <expr>` - Print in hexadecimal
- `print/t <expr>` - Print in binary
- `watch <expr>` - Set watchpoint
- `x/10x <address>` - Examine memory

## Debugging Workflow

1. **Reproduce the bug** - Make it crash or fail consistently
2. **Start debugger** - Run under NDB with debug symbols
3. **Breakpoint** - Set breakpoint at suspicious area
4. **Run** - Execute until breakpoint
5. **Examine** - Check variables, registers, memory
6. **Step** - Step through code line by line
7. **Understand** - Identify the root cause
8. **Fix** - Modify code to fix the bug
9. **Verify** - Test that fix works
10. **Learn** - Remember the pattern for next time

## Tips for Effective Debugging

### Before Debugging
- **Compile with -g** - Always include debug symbols
- **Compile with -O0** - Disable optimizations (can confuse debugger)
- **Reproduce locally** - Make bug happen on your machine
- **Minimize example** - Reduce to smallest failing case

### During Debugging
- **Start simple** - Use backtrace, don't overcomplicate
- **Be systematic** - Check one thing at a time
- **Trust nothing** - Verify assumptions with print statements
- **Take notes** - Document what you've tried
- **Use tools** - Let sanitizers find bugs for you

### After Debugging
- **Add tests** - Prevent regression
- **Document** - Write up what you learned
- **Fix root cause** - Don't just patch symptoms
- **Review** - Check for similar issues elsewhere

## Additional Resources

### Documentation
- [Main README](../README.md) - Project overview
- [Command Reference](../docs/commands.md) - All NDB commands
- [Architecture](../docs/architecture.md) - Project structure

### External Resources
- gdb Documentation - Many concepts apply to NDB
- Valgrind Manual - Understanding memory errors
- Sanitizers Wiki - Compiler-based detection tools

## Contributing

Found a bug in the examples? Have a suggestion for improvement?

1. Check the [project issues](../issues)
2. Create a pull request with your improvements
3. Follow the existing style and format

## License

Same as the main NDB project. See [LICENSE](../LICENSE).

---

**Happy Debugging!** Remember: Every bug is a learning opportunity.
