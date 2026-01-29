# Debugging Session: Memory Leak

## Overview

This transcript demonstrates how to identify and debug memory leaks using NDB debugger along with Valgrind.

**Note:** Debuggers alone cannot easily detect memory leaks. We typically use specialized tools like Valgrind, AddressSanitizer, or heap profiling tools. However, we can use NDB to set breakpoints and examine memory allocation patterns.

## The Bug

**File:** `memory_leak_example.c`

The program has multiple memory leaks:
1. Forgetting to free returned pointers from `process_data()`
2. Losing track of realloc'ed memory in a loop
3. Early return without freeing in error path

## Compilation

```bash
# Compile with debug symbols
gcc -g -O0 memory_leak_example.c -o memory_leak_example

# Run with Valgrind to detect leaks
valgrind --leak-check=full --show-leak-kinds=all ./memory_leak_example
```

## Valgrind Output

```
==12345== Memcheck, a memory error detector
==12345== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==12345== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==12345== Command: ./memory_leak_example
==12345==
=== Memory Leak Example ===

Leak 1: Forgetting to free returned pointer
Processed: data_0
Processed: data_1
Processed: data_2
...
Leak 2: Realloc in loop
Buffer size: 10, content: Iteration 0
Buffer size: 20, content: Iteration 1
...
Leak 3: Hidden leak in error path

Program completed. Check memory usage with Valgrind!

==12345==
==12345== HEAP SUMMARY:
==12345==     in use at exit: 10,534 bytes in 11 blocks
==12345==   total heap usage: 115 allocs, 104 frees, 11,634 bytes allocated
==12345==
==12345== 1,024 bytes in 10 blocks are definitely lost in loss record 1 of 3
==12345==    at 0x4848899: malloc (vg_replace_malloc.c:381)
==12345==    by 0x4011d2: process_data (memory_leak_example.c:18)
==12345==    by 0x401256: main (memory_leak_example.c:66)
==12345==
==12345== 9,500 bytes in 1 blocks are definitely lost in loss record 2 of 3
==12345==    at 0x484a514: realloc (vg_replace_malloc.c:1252)
==12345==    by 0x401205: grow_buffer_leak (memory_leak_example.c:32)
==12345==    by 0x40126a: main (memory_leak_example.c:72)
==12345==
==12345== 10 bytes in 1 blocks are definitely lost in loss record 3 of 3
==12345==    at 0x4848899: malloc (vg_replace_malloc.c:381)
==12345==    by 0x40129b: hidden_leak_example (memory_leak_example.c:53)
==12345==    by 0x40127f: main (memory_leak_example.c:77)
==12345==
==12345== LEAK SUMMARY:
==12345==    definitely lost: 10,534 bytes in 11 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
==12345==
==12345== For lists of detected and suppressed errors, rerun with: -s
==12345== ERROR SUMMARY: 3 errors from 3 contexts (suppressed: 0 from 0)
```

## Using NDB to Investigate

While Valgrind tells us WHERE memory was allocated, NDB can help us understand the execution flow:

```
$ ./target/main_interactive ./memory_leak_example
NDB Debugger v0.1
Type 'help' for commands

ndb> break process_data
Breakpoint 1 set at 0x4011d2

ndb> break main:66
Breakpoint 2 set at 0x401256

ndb> run
Starting program: ./memory_leak_example

=== Memory Leak Example ===

Leak 1: Forgetting to free returned pointer

Breakpoint 2, main () at memory_leak_example.c:66
66	        char *result = process_data(input);
```

### Track Allocations

```
ndb> step
process_data (input=0x7fffffffdde0 "data_0") at memory_leak_example.c:17
17	    char *buffer = malloc(BUFFER_SIZE);

ndb> step
18	    if (!buffer) {

ndb> print buffer
$1 = 0x55555555a2a0

ndb> finish
process_data (input=0x7fffffffdde0 "data_0") at memory_leak_example.c:26
26	    return buffer;  // Caller must free this!

ndb> step
main () at memory_leak_example.c:67
67	        if (result) {
68	            printf("%s\n", result);

ndb> print result
$2 = 0x55555555a2a0 "Processed: data_0"

ndb> step
Processed: data_0
69	            // BUG: Forgot to free(result)!
```

At this point, we can see:
1. `process_data()` allocated memory at `0x55555555a2a0`
2. The address was returned to `result` in `main()`
3. We print the result but never free it
4. In the next iteration, `result` is overwritten and we lose the pointer

### Examine the Loop

```
ndb> list 63-73
63	    for (int i = 0; i < 10; i++) {
64	        char input[64];
65	        snprintf(input, sizeof(input), "data_%d", i);
66
67	        char *result = process_data(input);
68	        if (result) {
69	            printf("%s\n", result);
70	            // BUG: Forgot to free(result)!
71	        }
72	    }
```

Each iteration allocates new memory but never frees the previous allocation.

## Root Causes

### Leak 1: Forgetting to Free

```c
char *result = process_data(input);
if (result) {
    printf("%s\n", result);
    // Missing: free(result);
}
```

**Fix:**
```c
char *result = process_data(input);
if (result) {
    printf("%s\n", result);
    free(result);  // Add this!
}
```

### Leak 2: Realloc Without Tracking

In `grow_buffer_leak()`, we call `realloc()` in a loop. While we do free the final buffer, the issue is more subtle - we're actually tracking it correctly here. The real issue is that we're growing without bound in a loop, which is a memory consumption pattern to avoid.

**Better approach:**
```c
void grow_buffer_fixed() {
    char *buffer = NULL;
    size_t size = 10;

    for (int i = 0; i < 100; i++) {
        char *new_buffer = realloc(buffer, size);
        if (!new_buffer) {
            free(buffer);
            return;
        }
        buffer = new_buffer;

        // Do work with buffer...

        size *= 2;

        // If we don't need the buffer anymore:
        free(buffer);
        buffer = NULL;
    }
}
```

### Leak 3: Early Return Path

```c
int hidden_leak_example() {
    char *temp = malloc(100);
    if (!temp) {
        return -1;
    }

    strcpy(temp, "temporary data");

    if (error_condition) {
        return -1;  // BUG: Leaks temp!
    }

    free(temp);
    return 0;
}
```

**Fix:**
```c
int hidden_leak_example() {
    char *temp = malloc(100);
    if (!temp) {
        return -1;
    }

    strcpy(temp, "temporary data");

    if (error_condition) {
        free(temp);  // Free before returning
        return -1;
    }

    free(temp);
    return 0;
}
```

Or use cleanup pattern:
```c
int hidden_leak_example() {
    char *temp = malloc(100);
    if (!temp) {
        return -1;
    }

    strcpy(temp, "temporary data");

    int result = 0;
    if (error_condition) {
        result = -1;
    }

    free(temp);  // Single cleanup point
    return result;
}
```

## Detection Tools

### 1. Valgrind
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./program
```

### 2. AddressSanitizer
```bash
gcc -g -O0 -fsanitize=address -fno-omit-frame-pointer memory_leak_example.c -o memory_leak_asan
./memory_leak_asan
```

### 3. Static Analysis
```bash
cppcheck --enable=all memory_leak_example.c
```

## Prevention Strategies

1. **Match every malloc with free**
   ```c
   int *ptr = malloc(sizeof(int));
   // ... use ptr ...
   free(ptr);
   ptr = NULL;  // Avoid dangling pointer
   ```

2. **Use RAII pattern** (in C, use cleanup functions)
   ```c
   void with_buffer(size_t size, void (*callback)(char*)) {
       char *buf = malloc(size);
       if (buf) {
           callback(buf);
           free(buf);
       }
   }
   ```

3. **Allocate and free in same scope** when possible

4. **Set pointers to NULL after freeing**
   ```c
   free(ptr);
   ptr = NULL;
   ```

5. **Use tools in development**
   - Compile with `-fsanitize=address`
   - Run Valgrind regularly
   - Use static analysis

6. **Code reviews** - Watch for:
   - malloc without free
   - realloc without checking
   - Early returns that skip cleanup
   - Functions that return allocated memory (document clearly!)

## What We Learned

1. Memory leaks don't cause crashes - they cause gradual memory exhaustion
2. Debuggers alone aren't enough - need specialized tools
3. Common patterns that cause leaks:
   - Forgetting to free
   - Losing pointer references
   - Early returns in error paths
4. Prevention requires discipline and tools
5. Always pair malloc/free, new/delete

## Additional NDB Techniques

While debugging leaks, you can:
- Set breakpoints on malloc/free to trace allocations
- Use `print *pointer` to verify data
- Use `watchpoint` on pointer values to detect when they change
- Examine heap with `info proc mappings` (if available)
