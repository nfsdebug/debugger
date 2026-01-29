# Debugging Session: Infinite Loop

## Overview

This transcript demonstrates how to debug programs that hang or appear frozen due to infinite loops.

**Note:** Infinite loops are particularly tricky because the program doesn't crash - it just stops responding. You need to interrupt it and examine its state.

## The Bug

**File:** `infinite_loop.c`

Various loop conditions that never become false, causing the program to hang.

## Compilation

```bash
gcc -g -O0 infinite_loop.c -o infinite_loop
```

## Scenario 1: Wrong Loop Condition

### Symptom
Program appears to hang, printing the same value repeatedly.

### NDB Debugging Session

```
$ ./target/main_interactive ./infinite_loop
NDB Debugger v0.1
Type 'help' for commands

ndb> run
Starting program: ./infinite_loop

=== Infinite Loop Examples ===

Example 1: Wrong loop condition
Expected: Count from 0 to 9
Actually: Never reaches 10

Count: 0
Count: -1
Count: -2
Count: -3
...
```

The program keeps printing negative numbers! Let's interrupt it.

### Interrupt and Examine

```
[Press Ctrl-C]

Program interrupted.
ndb> backtrace
#0  0x0000000000401168 in wrong_condition_loop () at infinite_loop.c:24
24	        i = i - 1;  // This makes i more negative!

ndb> list
19	    int i = 0;
20	    while (i < 10) {
21	        printf("Count: %d\n", i);
22
23	        // BUG: Should be i++, but we're decrementing
24	        i = i - 1;  // This makes i more negative!
25
26	        if (i < -50) {
27	            printf("Emergency break to prevent actual infinite loop\n");

ndb> print i
$1 = -45

ndb> print i < 10
$2 = true
```

The problem is clear:
- We want `i` to increase toward 10
- But we're doing `i = i - 1`, making it more negative
- The condition `i < 10` is always true for negative numbers!

### The Fix

```c
while (i < 10) {
    printf("Count: %d\n", i);
    i++;  // FIX: Increment, not decrement
}
```

## Scenario 2: Loop Modifying Wrong Variable

### Symptom
Loop counter never reaches exit condition.

### NDB Debugging

```
ndb> break wrong_variable_loop
Breakpoint 1 set at 0x40136a

ndb> continue
Continuing.

Example 7: Modifying wrong variable
-----------------------------------
Count: 0, Max: 10
Count: 0, Max: 11
Count: 0, Max: 12
...
[Press Ctrl-C]

ndb> print count
$1 = 0

ndb> print max
$2 = 17

ndb> print count < max
$3 = true
```

The `count` variable never increases - `max` increases instead!

### Root Cause

```c
int count = 0;
int max = 10;

while (count < max) {
    printf("Count: %d, Max: %d\n", count, max);
    max++;  // BUG: Should be count++
}
```

### The Fix

```c
while (count < max) {
    printf("Count: %d, Max: %d\n", count, max);
    count++;  // FIX: Increment the loop variable
}
```

## Scenario 3: Waiting for Impossible Condition

### Symptom
Program hangs waiting for data that never arrives.

### NDB Debugging

```
ndb> break waiting_forever
Breakpoint 2 set at 0x4011e9

ndb> continue
Continuing.

Example 3: Waiting for impossible condition
Expected: Wait for data to be ready
Actually: Data never becomes ready

Breakpoint 2, waiting_forever () at infinite_loop.c:49
49	    while (!data_ready) {

ndb> step
50	        printf("Waiting for data... (%d)\n", timeout);

ndb> print data_ready
$4 = 0

ndb> watchpoint data_ready
Watchpoint 3: data_ready

ndb> continue
Continuing.
Waiting for data... (0)
Waiting for data... (1)
Waiting for data... (2)
...
```

The watchpoint never triggers because `data_ready` is never set to 1.

### Examination

```
ndb> list 45-60
45	int data_ready = 0;
46	int timeout = 0;
47
48	// Simulate waiting for data
49	while (!data_ready) {
50	    printf("Waiting for data... (%d)\n", timeout);
51
52	    // BUG: No code here ever sets data_ready = 1
53	    // In real code, this might be waiting for a signal,
54	    // callback, or another thread that never happens
55
56	    timeout++;
57	    if (timeout > 5) {
58	        printf("Timeout! Breaking loop\n");
59	        break;
60	    }
```

The code has a timeout escape, but in production code, this might not exist.

### Real-World Patterns

**API Timeout:**
```c
// Good: Add timeout to any waiting loop
time_t start = time(NULL);
while (!data_ready && (time(NULL) - start) < TIMEOUT_SECONDS) {
    usleep(10000);  // Don't busy-wait
}

if (!data_ready) {
    fprintf(stderr, "Timeout waiting for data\n");
    return ERROR_TIMEOUT;
}
```

**Event-Driven Alternative:**
```c
// Better: Use callbacks or condition variables
void on_data_ready() {
    data_ready = 1;
    pthread_cond_signal(&data_cond);
}

void wait_for_data() {
    pthread_mutex_lock(&data_mutex);
    while (!data_ready) {
        pthread_cond_wait(&data_cond, &data_mutex);
    }
    pthread_mutex_unlock(&data_mutex);
}
```

## Scenario 4: Deadlock (Thread Hang)

### Symptom
Program appears frozen, multiple threads stuck.

### NDB Debugging

```
ndb> run
Starting program: ./infinite_loop

[Program appears frozen]

[Press Ctrl-C]

ndb> info threads
  1 Thread 12345.12345 "infinite_loop"  0x7ffff7bc7cb2 in clone
  2 Thread 12345.12346 "infinite_loop"  0x7ffff7a907bc in pthread_mutex_lock
  3 Thread 12345.12347 "infinite_loop"  0x7ffff7a907bc in pthread_mutex_lock

ndb> thread 2
[Switching to thread 2]
#0  0x7ffff7a907bc in pthread_mutex_lock () from /lib/x86_64-linux-gnu/libpthread.so.0

ndb> backtrace
#0  0x7ffff7a907bc in pthread_mutex_lock ()
#1  0x0000000000401234 in thread1_func () at infinite_loop.c:89
#2  0x7ffff7a9e082 in start_thread () from /lib/x86_64-linux-gnu/libpthread.so.0

ndb> thread 3
[Switching to thread 3]
#0  0x7ffff7a907bc in pthread_mutex_lock ()
#1  0x0000000000401278 in thread2_func () at infinite_loop.c:100

ndb> info mutex
Mutex 1: locked by thread 2, waiting for thread 3
Mutex 2: locked by thread 3, waiting for thread 2
```

Classic deadlock! Thread 2 holds mutex1 and wants mutex2, while thread 3 holds mutex2 and wants mutex1.

### The Fix

Always acquire locks in a consistent order:

```c
// Bad: Inconsistent lock order
void thread1() {
    lock(mutex1);
    lock(mutex2);
    // ...
    unlock(mutex2);
    unlock(mutex1);
}

void thread2() {
    lock(mutex2);  // Different order!
    lock(mutex1);
    // ...
    unlock(mutex1);
    unlock(mutex2);
}

// Good: Consistent lock order
void thread1() {
    lock(mutex1);
    lock(mutex2);
    // ...
    unlock(mutex2);
    unlock(mutex1);
}

void thread2() {
    lock(mutex1);  // Same order!
    lock(mutex2);
    // ...
    unlock(mutex2);
    unlock(mutex1);
}
```

## Scenario 5: Infinite Recursion (Stack Overflow)

### Symptom
Program crashes with "Segmentation fault" or "Stack overflow".

### NDB Debugging

```
ndb> run
Starting program: ./infinite_loop

Example 6: Infinite recursion
------------------------------
Recursion depth: 0
Recursion depth: 1
Recursion depth: 2
...
Recursion depth: 98765
Recursion depth: 98766

Program received signal SIGSEGV, Segmentation fault.
0x0000000000401345 in infinite_recursion (n=98767) at infinite_loop.c:130
130	    return infinite_recursion(n + 1) + 1;

ndb> backtrace
#0  0x0000000000401345 in infinite_recursion (n=98767) at infinite_loop.c:130
#1  0x0000000000401345 in infinite_recursion (n=98766) at infinite_loop.c:130
#2  0x0000000000401345 in infinite_recursion (n=98765) at infinite_loop.c:130
...
#98766 0x0000000000401345 in infinite_recursion (n=1) at infinite_loop.c:130
#98767 0x0000000000401445 in main () at infinite_loop.c:148
```

The backtrace shows 98,767 frames! The stack overflowed.

### The Fix

Always have a base case:

```c
int infinite_recursion(int n) {
    // FIX: Base case
    if (n > 100) {
        return n;
    }

    return infinite_recursion(n + 1) + 1;
}
```

Or convert to iteration:

```c
int safe_iteration(int start) {
    int result = 0;
    for (int n = start; n <= 100; n++) {
        result = n + 1;
    }
    return result;
}
```

## Debugging Techniques

### 1. Interrupt with Ctrl-C

When program appears frozen:
```
[Press Ctrl-C]
ndb> backtrace
```

This shows where the program is stuck.

### 2. Set Breakpoints at Loop Conditions

```
ndb> break infinite_loop.c:20
ndb> condition 1 i < 0
ndb> commands
  > print i
  > continue
  > end
```

### 3. Watch Variables

```
ndb> watch i
ndb> continue

Watchpoint 1: i

Old value = 0
New value = -1

Old value = -1
New value = -2
```

### 4. Check All Threads

```
ndb> info threads
ndb> thread apply all backtrace
```

### 5. Examine Loop Variables

```
ndb> print loop_var
ndb> print condition
ndb> print exit_condition
```

## Common Patterns

### Pattern 1: Off-by-One

```c
// Wrong: Skips last element
for (int i = 0; i <= array_size - 1; i++) { }

// Wrong: Goes past array end
for (int i = 0; i <= array_size; i++) { }

// Correct
for (int i = 0; i < array_size; i++) { }
```

### Pattern 2: Unsigned Wrap-Around

```c
// Dangerous: unsigned can't be negative
for (unsigned int i = 10; i >= 0; i--) {
    // When i reaches 0, next iteration wraps to UINT_MAX!
}
```

### Pattern 3: Float Comparison

```c
// Wrong: Float precision issues
for (float f = 0.0; f != 1.0; f += 0.1) {
    // May never exactly equal 1.0
}

// Correct
for (float f = 0.0; f < 1.0; f += 0.1) {
    // Use < instead of !=
}
```

### Pattern 4: Modifying Container During Iteration

```c
// Wrong: Modifying vector while iterating
for (int i = 0; i < vector.size(); i++) {
    if (should_remove(vector[i])) {
        vector.erase(vector.begin() + i);  // Skip next element!
        i--;  // Need to decrement
    }
}

// Better: Iterate backwards
for (int i = vector.size() - 1; i >= 0; i--) {
    if (should_remove(vector[i])) {
        vector.erase(vector.begin() + i);
    }
}
```

## Prevention Strategies

### 1. Use Iteration Limits

```c
int max_iterations = 1000;
int iterations = 0;

while (condition && iterations < max_iterations) {
    // ... work ...
    iterations++;
}

if (iterations >= max_iterations) {
    fprintf(stderr, "Warning: Hit iteration limit\n");
}
```

### 2. Add Progress Logging

```c
for (int i = 0; i < large_number; i++) {
    if (i % 10000 == 0) {
        printf("Progress: %d\n", i);
    }
    // ... work ...
}
```

### 3. Use Timeouts

```c
time_t start = time(NULL);
while (!done && (time(NULL) - start) < TIMEOUT) {
    // ... work ...
}

if (!done) {
    fprintf(stderr, "Timeout after %d seconds\n", TIMEOUT);
}
```

### 4. Static Analysis

```bash
cppcheck --enable=all infinite_loop.c
```

### 5. Compiler Warnings

```bash
gcc -Wall -Wextra -Wunreachable-code infinite_loop.c
```

## What We Learned

1. **Interrupt the program** to see where it's stuck
2. **Check loop conditions** - are they progressing toward exit?
3. **Watch variables** to see how they change
4. **Check all threads** - might be deadlock
5. **Add timeouts** to prevent indefinite waiting
6. **Use iteration limits** as safety measure
7. **Beware of unsigned** - can't go negative
8. **Float != comparisons** are dangerous
9. **Recursion depth** must be limited
10. **Lock ordering** prevents deadlock

## Quick Reference

| Symptom | Likely Cause | Debug Command |
|---------|-------------|---------------|
| Same value prints repeatedly | Wrong loop variable | `print i`, `watch i` |
| Program frozen, CPU idle | Waiting for condition | `backtrace`, `info threads` |
| Program frozen, CPU 100% | Infinite loop | `Ctrl-C`, `backtrace` |
| Crash after many prints | Stack overflow | `backtrace` (look for depth) |
| Multiple threads stuck | Deadlock | `info threads`, `info mutex` |

## Summary

Infinite loops cause hangs because:
- Exit condition never becomes true
- Loop variables change incorrectly
- Waiting for impossible events
- Threads deadlock each other
- Recursion never reaches base case

Detection:
- Interrupt with Ctrl-C
- Check backtrace
- Watch variables
- Examine all threads
- Look for recursion depth

Prevention:
- Add timeouts
- Use iteration limits
- Log progress
- Proper lock ordering
- Always have base case in recursion
- Use static analysis
