# Debugging Session: Integer Overflow

## Overview

This transcript demonstrates how to debug integer overflow bugs where arithmetic operations exceed the maximum value that can be stored in a type.

## The Bug

**File:** `integer_overflow.c`

Various integer overflow scenarios causing incorrect calculations, buffer overflows, and security vulnerabilities.

## Compilation

```bash
# Compile with debug symbols
gcc -g -O0 integer_overflow.c -o integer_overflow

# With UndefinedBehaviorSanitizer (best for detecting overflows)
gcc -g -O0 -fsanitize=undefined integer_overflow.c -o integer_overflow_ubsan
```

## Scenario 1: Signed Integer Overflow

### NDB Debugging Session

```
$ ./target/main_interactive ./integer_overflow
NDB Debugger v0.1
Type 'help' for commands

ndb> run
Starting program: ./integer_overflow

=== Integer Overflow Examples ===

Example 1: Signed integer overflow
----------------------------------
INT_MAX = 2147483647
Adding 1 to INT_MAX:
Result: -2147483648
(Should be negative due to overflow!)
```

### Examination

```
ndb> break signed_overflow
Breakpoint 1 set at 0x401156

ndb> run

Breakpoint 1, signed_overflow () at integer_overflow.c:15
15	int max = INT_MAX;

ndb> step
16	printf("INT_MAX = %d\n", max);

ndb> step
18	    int result = max + 1;  // BUG: Undefined behavior!

ndb> print max
$1 = 2147483647

ndb> print max + 1
$2 = -2147483648

ndb> step
19	printf("Result: %d\n", result);

ndb> print/x result
$3 = 0x80000000

ndb> print/b result
$4 = 10000000000000000000000000000000
```

The result is `0x80000000` in hex, which is:
- In unsigned: 2,147,483,648
- In signed: -2,147,483,648 (INT_MIN)

Signed integer overflow wraps from maximum to minimum!

### UBSan Detection

UndefinedBehaviorSanitizer catches this:

```
$ ./integer_overflow_ubsan
integer_overflow.c:18:14: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior integer_overflow.c:18:14
```

### The Fix

Use larger types or check before overflow:

```c
// Option 1: Use larger type
int64_t result = (int64_t)max + 1;

// Option 2: Check before operation
if (max < INT_MAX - 1) {
    int result = max + 1;
} else {
    printf("Would overflow!\n");
}

// Option 3: Use builtin overflow check
if (__builtin_add_overflow(max, 1, &result)) {
    printf("Overflow detected!\n");
}
```

## Scenario 2: Allocation Size Overflow (Security Issue)

This is a serious security vulnerability!

### NDB Debugging

```
ndb> break vulnerable_allocation
Breakpoint 2 set at 0x4011d9

ndb> continue
Continuing.

Example 3: Allocation size overflow
-----------------------------------
Allocating 268435456 items of 16 bytes each
Total size: 268435456 * 16 = 0

Breakpoint 2, vulnerable_allocation (count=268435456, size=16) at integer_overflow.c:45
45	size_t total = count * size;

ndb> print count
$5 = 268435456

ndb> print size
$6 = 16

ndb> print count * size
$7 = 0
```

The multiplication overflowed! `268435456 * 16` equals `0` due to wraparound.

### Step-by-Step

```
ndb> print/x count
$8 = 0x10000000

ndb> print/x size
$9 = 0x10

ndb> print/x count * size
$10 = 0x0

ndb> step
46	printf("Allocating %zu bytes\n", total);

ndb> step
48	char *buffer = malloc(total);

ndb> step
49	if (buffer) {

ndb> print buffer
$11 = 0x55555555a6a0 ""
```

`malloc(0)` succeeds and returns a tiny buffer, but the code thinks it allocated a huge buffer!

### The Security Implication

```c
char *buffer = malloc(total);  // Allocates 0 bytes (tiny buffer)
if (buffer) {
    // Thinks we have 4GB of buffer
    // Write 4GB of data into tiny buffer -> HEAP OVERFLOW!
    memcpy(buffer, source, count * size);  // Buffer overflow!
}
```

This is a classic heap overflow vulnerability.

### The Fix

```c
// Check for overflow before multiplication
if (count > 0 && size > SIZE_MAX / count) {
    fprintf(stderr, "Allocation size overflow!\n");
    return NULL;
}

size_t total = count * size;
char *buffer = malloc(total);

// Or use calloc (which checks for overflow)
char *buffer = calloc(count, size);
if (buffer && (count > SIZE_MAX / size)) {
    free(buffer);
    return NULL;
}

// Or use checked arithmetic
#include <stddef.h>
size_t total;
if (__builtin_mul_overflow(count, size, &total)) {
    fprintf(stderr, "Overflow detected!\n");
    return NULL;
}
```

## Scenario 3: Loop Counter Overflow

### Symptom

Loop runs too many times.

### NDB Debugging

```
ndb> break loop_counter_overflow
Breakpoint 3 set at 0x40129a

ndb> continue
Continuing.

Example 4: Loop counter overflow
-------------------------------
Starting counter at: 250
Loop condition: counter < 10

Breakpoint 3, loop_counter_overflow () at integer_overflow.c:67
67	unsigned char counter = 250;

ndb> watch counter
Watchpoint 4: counter

ndb> continue
Continuing.
Hardware watchpoint 4: counter

Old value = 250
New value = 251
Hardware watchpoint 4: counter

Old value = 251
New value = 252
...
Hardware watchpoint 4: counter

Old value = 255
New value = 0
```

The counter wrapped from 255 to 0!

```
ndb> print (unsigned char)255 + 1
$12 = 0

ndb> print 255 < 10
$13 = false

ndb> print 0 < 10
$14 = true
```

After wrapping to 0, the condition `0 < 10` is true again, so the loop continues!

### The Fix

```c
// Use larger type
unsigned int counter = 250;  // Not unsigned char

// Or add separate iteration counter
unsigned char counter = 250;
int iterations = 0;
while (counter < 10 && iterations < MAX_ITERATIONS) {
    // ... work ...
    counter++;
    iterations++;
}
```

## Scenario 4: Signed/Unsigned Comparison

### The Bug

```c
int signed_val = -1;
unsigned int unsigned_val = 10;

if (signed_val < unsigned_val) {
    printf("Less\n");
} else {
    printf("Greater\n");  // This prints!
}
```

### Why This Happens

```
ndb> print signed_val
$15 = -1

ndb> print unsigned_val
$16 = 10

ndb> print (unsigned int)signed_val
$17 = 4294967295

ndb> print (unsigned int)signed_val < unsigned_val
$18 = 0  // False!
```

When comparing signed and unsigned, the signed value is converted to unsigned. `-1` becomes `UINT_MAX` (4294967295), which is much larger than 10!

### The Fix

```c
// Option 1: Make both signed
if ((int)signed_val < (int)unsigned_val) { }

// Option 2: Make both unsigned
if ((unsigned)signed_val < (unsigned)unsigned_val) { }

// Option 3: Check signed value first
if (signed_val < 0) {
    printf("Signed value is negative\n");
} else if ((unsigned)signed_val < unsigned_val) {
    printf("Less\n");
}
```

## Scenario 5: Overflow in Average Calculation

### The Bug

```c
int calculate_average(int a, int b) {
    int sum = a + b;  // Can overflow!
    return sum / 2;
}
```

### Example

```
ndb> print calculate_average(INT_MAX, INT_MAX)
$19 = -1  // Wrong! Should be INT_MAX

ndb> print INT_MAX + INT_MAX
$20 = -2

ndb> print (INT_MAX + INT_MAX) / 2
$21 = -1
```

### The Fix

```c
// Option 1: Safer formula
int calculate_average(int a, int b) {
    return (a / 2) + (b / 2) + ((a % 2 + b % 2) / 2);
}

// Option 2: Use larger type
int calculate_average(int a, int b) {
    return (int)(((int64_t)a + (int64_t)b) / 2);
}

// Option 3: Handle overflow
int calculate_average(int a, int b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > INT_MAX || sum < INT_MIN) {
        // Handle overflow
        return INT_MAX;
    }
    return (int)(sum / 2);
}
```

## Detection Tools

### 1. UndefinedBehaviorSanitizer

```bash
gcc -g -O0 -fsanitize=undefined -fno-sanitize-recover=undefined integer_overflow.c -o ubsan
./ubsan
```

Output:
```
integer_overflow.c:18:14: runtime error: signed integer overflow
integer_overflow.c:45:20: runtime error: addition of unsigned offset to 0x5070000000b0 overflowed
```

### 2. AddressSanitizer

```bash
gcc -g -O0 -fsanitize=address integer_overflow.c -o asan
```

Catches the buffer overflow from allocation size overflow.

### 3. Static Analysis

```bash
cppcheck --enable=all integer_overflow.c
```

### 4. Compiler Warnings

```bash
gcc -Wall -Wextra -Woverflow integer_overflow.c
```

## Prevention Strategies

### 1. Use Appropriate Types

```c
// For sizes and counts
size_t size = 100;

// For large numbers
int64_t large = 1234567890123LL;

// For flags
unsigned int flags = 0xFF;
```

### 2. Check Before Operations

```c
// Addition
if (a > INT_MAX - b) {
    // Handle overflow
}

// Multiplication
if (a > 0 && b > INT_MAX / a) {
    // Handle overflow
}
```

### 3. Use Built-in Overflow Checks

```c
int result;
if (__builtin_add_overflow(a, b, &result)) {
    printf("Overflow!\n");
}

if (__builtin_mul_overflow(a, b, &result)) {
    printf("Overflow!\n");
}
```

### 4. Use Safe Functions

```c
// Instead of malloc(size * count)
calloc(count, size);  // Checks for overflow

// Or use safe wrapper
void* safe_calloc(size_t count, size_t size) {
    if (count > 0 && size > SIZE_MAX / count) {
        return NULL;
    }
    return calloc(count, size);
}
```

### 5. Enable All Warnings

```bash
gcc -Wall -Wextra -Woverflow -Wstrict-overflow=5
```

## Common Overflow Patterns

| Pattern | Danger | Fix |
|---------|--------|-----|
| `malloc(a * b)` | Size wrap to 0 | `calloc(a, b)` or check |
| `array[a + b]` | Index overflow | Check `a + b < size` |
| `(a + b) / 2` | Average overflow | `a/2 + b/2 + (a%2+b%2)/2` |
| `for (; i < 10; i++)` | Unsigned `i` never < 0 | Use `int` or add limit |
| `int len = strlen(s) + 1` | Overflow if huge | Check `s` length first |
| `size = count * 4` | Size calculation | Use `count * sizeof(*ptr)` |

## Real-World Vulnerabilities

1. **CVE-2021-34527** (PrintNightmare) - Integer overflow in Windows Print Spooler
2. **CVE-2019-19906** - wget heap overflow from size calculation
3. **CVE-2018-20346** - Linux kernel overflow in filesystem code
4. **Many image parsing bugs** - Width * height overflow

## What We Learned

1. **Signed overflow is undefined behavior** - can assume anything
2. **Unsigned overflow wraps** - defined but often wrong
3. **Allocation size overflow is dangerous** - causes heap overflows
4. **Mixed signed/unsigned is tricky** - implicit conversions
5. **UBSan is excellent** for detecting these issues
6. **Check before operations** - use builtins when possible
7. **Use calloc instead of malloc*size** - safer
8. **Be careful with size calculations** - `sizeof(*ptr) * count`

## Quick Reference

### Detecting Overflow

```c
// Addition
if (__builtin_add_overflow(a, b, &result)) { }

// Multiplication
if (__builtin_mul_overflow(a, b, &result)) { }

// Subtraction
if (__builtin_sub_overflow(a, b, &result)) { }

// Manual check
if (a > INT_MAX - b) { /* overflow */ }
```

### Safe Allocation

```c
// Good
ptr = calloc(count, size);

// Good with check
if (count > 0 && size > SIZE_MAX / count) {
    return NULL;
}
ptr = malloc(count * size);

// Good with builtin
size_t total;
if (__builtin_mul_overflow(count, size, &total)) {
    return NULL;
}
ptr = malloc(total);
```

## Summary

Integer overflows cause:
- Incorrect calculations
- Buffer overflows (via size calculations)
- Security vulnerabilities
- Logic errors

Detection:
- UBSan (`-fsanitize=undefined`)
- Compiler warnings
- Static analysis
- Code review

Prevention:
- Use appropriate types
- Check before operations
- Use built-in overflow checks
- Use safe functions (calloc)
- Enable all warnings
- Test with boundary values (INT_MAX, etc.)
