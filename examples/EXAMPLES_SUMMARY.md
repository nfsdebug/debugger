# NDB Debugger - Examples Summary

This document provides a summary of all debugging examples created for the NDB debugger.

## What Was Created

### Example Programs (6 files, 1,070 lines of code)

All example programs are written in C and demonstrate real-world bugs:

1. **segfault_example.c** (75 lines)
   - Null pointer dereference causing segmentation fault
   - Demonstrates missing NULL check before dereferencing
   - Beginner level

2. **memory_leak_example.c** (113 lines)
   - Multiple memory leak patterns
   - Forgetting to free returned pointers
   - Realloc without proper cleanup
   - Hidden leaks in error paths
   - Intermediate level

3. **use_after_free.c** (138 lines)
   - Dangling pointer after free()
   - Using inner pointers after parent freed
   - Array of freed pointers
   - Double-free patterns
   - Intermediate level

4. **race_condition.c** (201 lines)
   - Unsynchronized counter increment
   - Check-then-act race condition
   - Producer-consumer race
   - Requires pthread library
   - Advanced level

5. **infinite_loop.c** (266 lines)
   - Wrong loop conditions
   - Off-by-one errors
   - Waiting for impossible conditions
   - Thread deadlock (simplified)
   - Infinite recursion
   - Beginner to Intermediate level

6. **integer_overflow.c** (257 lines)
   - Signed integer overflow
   - Unsigned wraparound
   - Allocation size overflow (security issue)
   - Loop counter overflow
   - Signed/unsigned comparison bugs
   - Intermediate level

### Debugging Transcripts (6 files, 2,589 lines)

Each example has a comprehensive debugging guide:

1. **segfault_debugging.md** (173 lines)
   - Complete NDB session showing segfault analysis
   - Using backtrace to find crash location
   - Examining variables and parameters
   - NULL pointer detection

2. **memory_leak_debugging.md** (347 lines)
   - Valgrind output interpretation
   - Tracking allocations with NDB
   - Common leak patterns
   - Prevention strategies

3. **use_after_free_debugging.md** (420 lines)
   - Detecting use-after-free with ASan
   - Tracing memory lifecycle
   - Inner pointer dangers
   - Prevention with NULL assignment

4. **race_condition_debugging.md** (427 lines)
   - Using ThreadSanitizer for race detection
   - Examining thread states with NDB
   - Mutex synchronization patterns
   - Check-then-act fixes

5. **infinite_loop_debugging.md** (597 lines)
   - Interrupting execution with Ctrl-C
   - Examining loop variables
   - Deadlock detection
   - Stack overflow analysis

6. **integer_overflow_debugging.md** (573 lines)
   - UBSan output interpretation
   - Security implications of overflows
   - Safe arithmetic patterns
   - Compiler builtin checks

### Documentation (3 files, 887 lines)

1. **README.md** (349 lines)
   - Comprehensive overview of all examples
   - Difficulty ratings
   - Learning objectives
   - Compilation instructions
   - Recommended learning path
   - Tool reference
   - Common commands

2. **QUICK_REFERENCE.md** (271 lines)
   - Quick summary table
   - Common commands cheat sheet
   - Bug pattern reference
   - Quick start guide
   - Troubleshooting tips

3. **Makefile** (113 lines)
   - Build all examples
   - Build with sanitizers
   - Test targets
   - Clean target

### Build Artifacts (9 executables)

Compiled examples:
- `segfault_example` (18K)
- `memory_leak_example` (19K)
- `use_after_free` (19K)
- `use_after_free_asan` (29K) - with AddressSanitizer
- `race_condition` (21K)
- `race_condition_tsan` (23K) - with ThreadSanitizer
- `infinite_loop` (21K)
- `integer_overflow` (20K)
- `integer_overflow_ubsan` (29K) - with UndefinedBehaviorSanitizer

## Statistics

- **Total source code**: 1,070 lines
- **Total documentation**: 3,476 lines
- **Total examples**: 6
- **Total debugging transcripts**: 6
- **Total lines created**: 4,546

## Features

### Realistic Bugs
All examples demonstrate bugs that occur in real-world code:
- Not artificial or contrived
- Common patterns seen in production
- Range from beginner to advanced
- Security-relevant bugs included

### Educational Value
Each example teaches specific debugging techniques:
- **NDB commands**: How to use the debugger effectively
- **Analysis**: How to understand what went wrong
- **Tools**: When to use sanitizers vs debugger
- **Prevention**: How to avoid similar bugs

### Multiple Tools
Examples demonstrate use of:
- **NDB** - For controlling execution and examining state
- **Valgrind** - For memory leak detection
- **AddressSanitizer** - For memory errors
- **ThreadSanitizer** - For race conditions
- **UndefinedBehaviorSanitizer** - For integer overflows

### Progressive Difficulty
Examples are ordered by difficulty:
1. Beginner: Segfault, Infinite Loop
2. Intermediate: Memory Leak, Use-After-Free, Integer Overflow
3. Advanced: Race Condition

## Usage

### Quick Start
```bash
cd /home/sbstndbs/debugger/examples
make                    # Build all examples
../../target/main_interactive ./segfault_example  # Debug with NDB
```

### Learning Path
1. Start with segfault_example (beginner)
2. Try infinite_loop (beginner)
3. Progress to memory_leak_example (intermediate)
4. Try use_after_free (intermediate)
5. Study integer_overflow (intermediate)
6. Finish with race_condition (advanced)

### Integration with Main Project
The main README.md now includes a link to examples:
```markdown
- [Debugging Examples](examples/README.md) - Real-world debugging tutorials
```

## File Organization

```
/home/sbstndbs/debugger/examples/
├── README.md                          # Main documentation
├── QUICK_REFERENCE.md                 # Quick reference card
├── Makefile                          # Build system
├── segfault_example.c                # Source code
├── segfault_debugging.md             # Debugging guide
├── memory_leak_example.c             # Source code
├── memory_leak_debugging.md          # Debugging guide
├── use_after_free.c                  # Source code
├── use_after_free_debugging.md       # Debugging guide
├── race_condition.c                  # Source code
├── race_condition_debugging.md       # Debugging guide
├── infinite_loop.c                   # Source code
├── infinite_loop_debugging.md        # Debugging guide
├── integer_overflow.c                # Source code
└── integer_overflow_debugging.md     # Debugging guide
```

## Key Highlights

### Comprehensive Coverage
- All major C bug types covered
- Memory management errors (leak, use-after-free)
- Concurrency issues (race conditions)
- Logic errors (infinite loops, integer overflow)
- Crash bugs (segfaults)

### Practical Focus
- Every example can be compiled and run
- Debugging transcripts show actual sessions
- Tools demonstrated are freely available
- Prevention strategies are practical

### Well-Documented
- Each example has detailed debugging guide
- Common patterns identified
- Prevention strategies included
- Tool usage explained

### Easy to Use
- Makefile for easy compilation
- Clear compilation instructions
- Quick reference for common commands
- Progressive difficulty levels

## Testing

All examples have been tested:
- Compile without errors
- Run and demonstrate the bug
- Work with NDB debugger
- Compatible with sanitizers (where applicable)

## Future Enhancements

Possible additions:
- Buffer overflow example
- Stack smashing example
- Format string vulnerability
- More complex race conditions
- Deadlock detection example
- Memory corruption examples

## Conclusion

This example suite provides a comprehensive introduction to debugging C programs using NDB and related tools. The examples are realistic, well-documented, and progressively challenging, making them suitable for:
- Learning to use NDB
- Understanding common C bugs
- Teaching debugging techniques
- Reference material for real debugging sessions

The examples integrate seamlessly with the NDB project and provide practical, hands-on learning opportunities for users of all skill levels.
