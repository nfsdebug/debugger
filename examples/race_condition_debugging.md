# Debugging Session: Race Condition

## Overview

This transcript demonstrates how to debug a race condition where multiple threads access shared data without proper synchronization.

**Important Note:** Race conditions are notoriously difficult to debug because they are non-deterministic. The bug may not appear every time you run the program, and using a debugger can actually change the timing enough to make the bug disappear (called a "heisenbug").

## The Bug

**File:** `race_condition.c`

Multiple threads increment a shared counter without synchronization, causing lost updates due to the non-atomic nature of the increment operation.

## Compilation

```bash
# Compile with debug symbols and pthread
gcc -g -O0 -pthread race_condition.c -o race_condition

# With ThreadSanitizer (best for detecting races)
gcc -g -O0 -pthread -fsanitize=thread race_condition.c -o race_condition_tsan
```

## The Problem

The increment operation `counter++` looks like one operation, but it's actually three:
1. Read `counter` from memory into a register
2. Add 1 to the register
3. Write the register back to memory

Without synchronization, multiple threads can interleave these steps:
- Thread A reads counter (value: 100)
- Thread B reads counter (value: 100)
- Thread A adds 1 (register: 101)
- Thread B adds 1 (register: 101)
- Thread A writes 101
- Thread B writes 101
- Result: counter is 101 instead of 102!

## Running the Program (Multiple Times)

Run the program several times to see different results:

```
$ for i in {1..5}; do ./race_condition; echo "---"; done
=== Race Condition Example ===

Example 1: Counter race condition
Expected result: 1000000
Actual result: 875432
Lost increments: 124568

Example 1: Counter race condition
Expected result: 1000000
Actual result: 912345
Lost increments: 87655

Example 1: Counter race condition
Expected result: 1000000
Actual result: 854321
Lost increments: 145679
```

Notice how the "Actual result" varies each time! That's the hallmark of a race condition.

## ThreadSanitizer Output (Best Detection Method)

ThreadSanitizer is specifically designed to find data races:

```
$ ./race_condition_tsan
=== Race Condition Example ===

Example 1: Counter race condition
Expected result: 1000000
==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 4 at 0x555555558010 by thread T7:
    #0 increment_counter race_condition.c:23
    #1 <null> <null>

  Previous write of size 4 at 0x555555558010 by thread T6:
    #0 increment_counter race_condition.c:23
    #1 <null> <null>

Location is global 'shared_counter' of size 4 at 0x555555558010

Thread T7 (tid=12350, running) created by main at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors.cc:913
    #1 main race_condition.c:141

Thread T6 (tid=12349, running) created by main at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors.cc:913
    #1 main race_condition.c:141

SUMMARY: ThreadSanitizer: data race race_condition.c:23 in increment_counter
==================
Actual result: 987654
Lost increments: 12346
```

Perfect! ThreadSanitizer identifies:
- The exact line with the race (line 23)
- The variable involved (`shared_counter`)
- The threads involved (T6 and T7)

## Using NDB to Debug Race Conditions

While race conditions are hard to catch in a debugger (due to timing changes), here's how you can investigate:

### 1. Examine All Threads

```
$ ./target/main_interactive ./race_condition
NDB Debugger v0.1
Type 'help' for commands

ndb> break increment_counter
Breakpoint 1 set at 0x4011a6

ndb> run
Starting program: ./race_condition

=== Race Condition Example ===

Example 1: Counter race condition
Expected result: 1000000

Breakpoint 1, increment_counter (arg=0x7fffffffdc9c) at race_condition.c:20
20	    printf("Thread %d starting\n", thread_id);
```

### 2. Check All Threads

```
ndb> info threads
  1 Thread 12345.12345 "race_condition"  0x4011a6 in increment_counter
  2 Thread 12345.12346 "race_condition"  0x7ffff7bc7cb2 in clone
  3 Thread 12345.12347 "race_condition"  0x7ffff7bc7cb2 in clone
```

### 3. Switch Between Threads

```
ndb> thread 2
[Switching to thread 2]
#0  0x7ffff7bc7cb2 in clone () from /lib/x86_64-linux-gnu/libc.so.6

ndb> thread 3
[Switching to thread 3]
#0  0x7ffff7bc7cb2 in clone () from /lib/x86_64-linux-gnu/libc.so.6
```

### 4. Set Watchpoints on Shared Data

```
ndb> watchpoint shared_counter
Hardware watchpoint 2: shared_counter

ndb> continue
Continuing.

Hardware watchpoint 2: shared_counter

Old value = 0
New value = 1
increment_counter (arg=0x7fffffffdc9c) at race_condition.c:23
23	        shared_counter++;  // RACE CONDITION HERE
```

The watchpoint will trigger every time `shared_counter` changes, showing you which threads are accessing it.

## Root Causes and Fixes

### Problem 1: Unsynchronized Counter

**Buggy Code:**
```c
void* increment_counter(void *arg) {
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        shared_counter++;  // RACE CONDITION
    }
    return NULL;
}
```

**Fix 1: Mutex**
```c
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* increment_counter(void *arg) {
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}
```

**Fix 2: Atomic Operations (Best)**
```c
#include <stdatomic.h>

atomic_int shared_counter = 0;

void* increment_counter(void *arg) {
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        atomic_fetch_add(&shared_counter, 1);
    }
    return NULL;
}
```

### Problem 2: Check-Then-Act Race

**Buggy Code:**
```c
void* withdraw(void *amount_ptr) {
    int amount = *(int*)amount_ptr;

    if (bank_balance >= amount) {  // Check
        usleep(1);  // Delay makes race more likely
        bank_balance -= amount;  // Act
        printf("Withdrew %d\n", amount);
    }
    return NULL;
}
```

Two threads can both pass the check, then both withdraw, causing negative balance.

**Fix:**
```c
pthread_mutex_t balance_mutex = PTHREAD_MUTEX_INITIALIZER;

void* withdraw(void *amount_ptr) {
    int amount = *(int*)amount_ptr;

    pthread_mutex_lock(&balance_mutex);
    if (bank_balance >= amount) {
        bank_balance -= amount;
        printf("Withdrew %d\n", amount);
    } else {
        printf("Insufficient funds\n");
    }
    pthread_mutex_unlock(&balance_mutex);

    return NULL;
}
```

Or use atomic CAS (Compare-And-Swap):
```c
bool withdraw(int amount) {
    int old_balance, new_balance;
    do {
        old_balance = bank_balance;
        if (old_balance < amount) return false;
        new_balance = old_balance - amount;
    } while (!atomic_compare_exchange_weak(&bank_balance, &old_balance, new_balance));
    return true;
}
```

### Problem 3: Producer-Consumer Without Synchronization

**Buggy Code:**
```c
SharedData shared_data = {0, 0};

void* producer(void *arg) {
    shared_data.value = i * 100;
    shared_data.ready = 1;  // No memory barrier!
}

void* consumer(void *arg) {
    while (!shared_data.ready) { }  // Spin
    printf("%d\n", shared_data.value);  // Might see stale data
}
```

**Fix:**
```c
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_cond = PTHREAD_COND_INITIALIZER;

void* producer(void *arg) {
    pthread_mutex_lock(&data_mutex);
    shared_data.value = i * 100;
    shared_data.ready = 1;
    pthread_cond_signal(&data_cond);
    pthread_mutex_unlock(&data_mutex);
}

void* consumer(void *arg) {
    pthread_mutex_lock(&data_mutex);
    while (!shared_data.ready) {
        pthread_cond_wait(&data_cond, &data_mutex);
    }
    printf("%d\n", shared_data.value);
    pthread_mutex_unlock(&data_mutex);
}
```

## Detection Tools

### 1. ThreadSanitizer (Best)

```bash
gcc -g -O1 -fsanitize=thread -fPIE -pie race_condition.c -pthread -o race_condition_tsan
./race_condition_tsan
```

### 2. Helgrind (Valgrind)

```bash
valgrind --tool=helgrind ./race_condition
```

### 3. Static Analysis

```bash
cppcheck --enable=all --inconclusive race_condition.c
```

## Prevention Strategies

### 1. Use Synchronization Primitives

- **Mutexes**: Protect shared data
- **Atomic operations**: For simple counters
- **Condition variables**: For waiting on conditions
- **Semaphores**: For resource counting
- **Barriers**: For phased parallelism

### 2. Minimize Shared State

```c
// Bad: Global shared state
int counter = 0;
void* thread_func(void*) {
    counter++;  // Needs synchronization
}

// Good: Thread-local data
__thread int counter = 0;
void* thread_func(void*) {
    counter++;  // No synchronization needed
}
```

### 3. Use Thread-Safe Data Structures

- Message queues
- Lock-free data structures
- Thread-local storage
- Immutable data

### 4. Follow Best Practices

```c
// Good pattern: Lock hierarchy to avoid deadlocks
void thread1() {
    lock(mutex_a);
    lock(mutex_b);
    // ... critical section ...
    unlock(mutex_b);
    unlock(mutex_a);
}

void thread2() {
    lock(mutex_a);  // Same order!
    lock(mutex_b);
    // ... critical section ...
    unlock(mutex_b);
    unlock(mutex_a);
}
```

## What We Learned

1. **Race conditions are non-deterministic** - results vary between runs
2. **Increment is not atomic** - it's read-modify-write
3. **Debuggers can hide race conditions** - by changing timing
4. **ThreadSanitizer is the best tool** for detecting races
5. **Synchronization is required** for all shared mutable state
6. **Check-then-act is a common pattern** that's often racy
7. **Memory barriers matter** - not just locks

## Common Race Condition Patterns

1. **Lost Update**: Read-modify-write without synchronization
2. **Check-Then-Act**: Check state, then act (state may change)
3. **Lazy Initialization**: Double-checked locking without proper barriers
4. **Publish-Subscribe**: Publishing object before fully initialized
5. **Non-Atomic Initialization**: Writing to object before pointer is visible

## Debugging Tips

1. **Use ThreadSanitizer** - it's designed for this
2. **Run multiple times** - race conditions are intermittent
3. **Add delays strategically** - to make races more frequent
4. **Review all shared state** - if multiple threads touch it, it needs protection
5. **Use assertions** - `assert(thread_owns_lock(&mutex))`
6. **Enable thread debugging** - `-pthread` in GCC

## Summary

Race conditions occur when:
- Multiple threads access shared data
- At least one access is a write
- No synchronization prevents simultaneous access

Detection:
- ThreadSanitizer (best)
- Helgrind
- Code review
- Stress testing

Prevention:
- Use mutexes or atomics
- Minimize shared state
- Use thread-safe data structures
- Follow lock ordering rules
- Test with ThreadSanitizer regularly
