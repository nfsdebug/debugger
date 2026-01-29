# Debugging Session: Use-After-Free

## Overview

This transcript demonstrates how to debug a use-after-free vulnerability where memory is accessed after being freed.

## The Bug

**File:** `use_after_free.c`

The program continues to use pointers after the memory they point to has been freed. This can cause:
- Segmentation faults (if memory is unmapped)
- Data corruption (if memory is reused)
- Security vulnerabilities (exploitable in some cases)

## Compilation

```bash
# Compile with debug symbols
gcc -g -O0 use_after_free.c -o use_after_free

# Or with AddressSanitizer for better detection
gcc -g -O0 -fsanitize=address use_after_free.c -o use_after_free_asan
```

## NDB Debugging Session

```
$ ./target/main_interactive ./use_after_free
NDB Debugger v0.1
Type 'help' for commands

ndb> run
Starting program: ./use_after_free

=== Use-After-Free Example ===

Example 1: Basic use-after-free
--------------------------------
Block created: id=1, data=Important data
Block value set to: 42
Trying to access saved data: Important data
Trying to access block: id=1

Program received signal SIGSEGV, Segmentation fault.
0x00000000004012e9 in process_block_with_bug () at use_after_free.c:54
54	    printf("Trying to access block: id=%d\n", block->id);
```

### Analysis

#### 1. Examine the Crash

```
ndb> backtrace
#0  0x00000000004012e9 in process_block_with_bug () at use_after_free.c:54
#1  0x00000000004013a4 in main () at use_after_free.c:92

ndb> list
49	    // Free the block
50	    destroy_block(block);
51
52	    // BUG: Try to use the saved data pointer
53	    // This is use-after-free because the data was freed with destroy_block
54	    printf("Trying to access saved data: %s\n", saved_data);  // DANGEROUS!
55
56	    // Even worse - use the freed block pointer
57	    printf("Trying to access block: id=%d\n", block->id);  // CRASH HERE!
```

#### 2. Check Pointer Values

```
ndb> print block
$1 = (DataBlock *) 0x55555555a2a0

ndb> print *block
Cannot access memory at address 0x55555555a2a0

ndb> print saved_data
$2 = 0x55555555a6b0 "Important data"
```

Interesting! `saved_data` still shows the string, but `block` is inaccessible. This is because:
- `saved_data` points to freed memory that hasn't been overwritten yet
- `block` points to freed memory that has been overwritten or unmapped

#### 3. Trace the Execution

Let's restart and see what happens at the free:

```
ndb> break destroy_block
Breakpoint 1 set at 0x401189

ndb> run
Starting program: ./use_after_free

=== Use-After-Free Example ===

Example 1: Basic use-after-free
--------------------------------
Block created: id=1, data=Important data
Block value set to: 42

Breakpoint 1, destroy_block (block=0x55555555a2a0) at use_after_free.c:37
37	void destroy_block(DataBlock *block) {
```

```
ndb> step
38	    if (block) {

ndb> step
39	        free(block->data);

ndb> print block
$1 = (DataBlock *) 0x55555555a2a0
ndb> print block->data
$2 = 0x55555555a6b0 "Important data"

ndb> step
40	        free(block);

ndb> print block
$3 = (DataBlock *) 0x55555555a2a0
ndb> print *block
Cannot access memory at address 0x55555555a2a0
```

After `free(block)`, the memory is immediately inaccessible. The pointer still holds the address, but accessing it causes a segfault.

#### 4. Examine the Second Example

Let's look at the dangling pointer array example:

```
ndb> break dangling_pointer_array
Breakpoint 2 set at 0x40131a

ndb> continue
Continuing.

Trying to access saved data: Important data

Breakpoint 2, dangling_pointer_array () at use_after_free.c:68
68	void dangling_pointer_array() {
```

```
ndb> step
70	    for (int i = 0; i < 5; i++) {
71	        array[i] = malloc(sizeof(int));

ndb> print i
$4 = 0

ndb> step
72	        if (array[i]) {
73	        *(array[i]) = i * 10;

ndb> print *array[0]
$5 = 0
```

Continue to the problematic part:

```
ndb> break use_after_free.c:86
Breakpoint 3 set at 0x40136f

ndb> continue
Continuing.
...
Breakpoint 3, dangling_pointer_array () at use_after_free.c:86
86	    printf("Array[1] value: %d\n", *array[1]);  // DANGEROUS!

ndb> print array[1]
$6 = (int *) 0x0
```

Wait, array[1] is NULL? Let's check the loop again:

```
ndb> list 79-86
79	    // Free some elements
80	    for (int i = 0; i < 3; i++) {
81	        free(array[i]);
82	        array[i] = NULL;  // Good practice, but often forgotten
83	    }
84
85	    // BUG: Try to use freed element
86	    printf("Array[1] value: %d\n", *array[1]);  // DANGEROUS!
```

Actually, setting it to NULL after free prevents the use-after-free from crashing (dereferencing NULL gives segfault, but at least it's predictable). The real issue would be if line 82 was missing.

## AddressSanitizer Output

AddressSanitizer is much better at detecting these issues:

```
$ ./use_after_free_asan
=== Use-After-Free Example ===

Example 1: Basic use-after-free
--------------------------------
Block created: id=1, data=Important data
Block value set to: 42
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x5070000000b0
READ of size 1 at 0x5070000000b0 thread T0
    #0 0x4012e9 in process_block_with_bug use_after_free.c:54
    #1 0x4013a4 in main use_after_free.c:92
    #2 0x7ffff7a7e082 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40113d in _start use_after_free.c:134

0x5070000000b0 is located 0 bytes inside of 5-byte region [0x5070000000b0,0x5070000000b5)
freed by thread T0 here:
    #0 0x7ffff7d4a7b2 in free ../../../../src/libsanitizer/asan/asan_malloc_linux.cc:123
    #1 0x40119a in destroy_block use_after_free.c:39
    #2 0x4012d0 in process_block_with_bug use_after_free.c:50
    #3 0x4013a4 in main use_after_free.c:92

previously allocated by thread T0 here:
    #0 0x7ffff7d4aab2 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cc:145
    #1 0x401167 in create_block use_after_free.c:20
    #2 0x4012b0 in process_block_with_bug use_after_free.c:44
    #3 0x4013a4 in main use_after_free.c:92

SUMMARY: AddressSanitizer: heap-use-after-free use_after_free.c:54 in process_block_with_bug
==12345==ABORTING
```

Perfect! ASan tells us:
- Exact line where use-after-free occurred (line 54)
- Where the memory was freed (line 39)
- Where it was originally allocated (line 20)

## Root Causes

### Problem 1: Using Pointer After Free

```c
DataBlock *block = create_block(1, "Important data");
// ... use block ...
destroy_block(block);

// BUG: block is now a dangling pointer!
printf("%d\n", block->id);  // Use-after-free
```

**Fix:**
```c
DataBlock *block = create_block(1, "Important data");
// ... use block ...
destroy_block(block);
block = NULL;  // Prevent accidental use

if (block) {
    printf("%d\n", block->id);
}
```

### Problem 2: Saving Inner Pointer

```c
char *saved_data = block->data;
destroy_block(block);

// BUG: saved_data points to freed memory!
printf("%s\n", saved_data);
```

**Fix:**
```c
// If you need to keep the data, copy it first
char *saved_data = strdup(block->data);
destroy_block(block);

printf("%s\n", saved_data);
free(saved_data);
saved_data = NULL;
```

### Problem 3: Double Free

```c
free(ptr);
free(ptr);  // Double-free - undefined behavior!
```

**Fix:**
```c
free(ptr);
ptr = NULL;  // Now second free is safe
free(ptr);  // This is OK (freeing NULL is safe)
```

## Prevention Strategies

### 1. Always NULL After Free

```c
free(ptr);
ptr = NULL;
```

This makes use-after-free immediately crash with a clear NULL dereference.

### 2. Use Smart Pointers (in C++)

Or in C, use allocation wrappers:

```c
typedef struct {
    void *ptr;
    size_t size;
} SafePtr;

SafePtr* safe_alloc(size_t size) {
    SafePtr *sp = malloc(sizeof(SafePtr));
    if (sp) {
        sp->ptr = malloc(size);
        sp->size = sp->ptr ? size : 0;
    }
    return sp;
}

void safe_free(SafePtr *sp) {
    if (sp) {
        free(sp->ptr);
        sp->ptr = NULL;
        sp->size = 0;
    }
}
```

### 3. Copy Data You Need to Keep

```c
// Instead of keeping pointer to internal data
char *data = obj->data;
free_object(obj);

// Copy the data first
char *data = strdup(obj->data);
free_object(obj);
// ... use data ...
free(data);
```

### 4. Use Tools

```bash
# AddressSanitizer
gcc -fsanitize=address -g program.c

# Valgrind
valgrind --free-fill=FF --track-origins=yes ./program

# Static analysis
cppcheck --enable=all program.c
```

### 5. Code Review Checklist

- [ ] Every malloc has a corresponding free
- [ ] Pointers are set to NULL after free
- [ ] No returns to freed memory
- [ ] Inner pointers aren't saved past object lifetime
- [ ] No double-free paths

## What We Learned

1. **Use-after-free is undefined behavior** - may crash, may silently corrupt data
2. **Dangling pointers** are pointers to freed memory
3. **ASan is excellent** at detecting these issues at runtime
4. **Setting to NULL after free** turns silent corruption into obvious crashes
5. **Copy data** if you need it past the object's lifetime
6. **Inner pointers** (pointers to struct members) are especially dangerous

## Real-World Impact

Use-after-free vulnerabilities are serious security issues:
- CVE-2019-13720 (Chrome)
- CVE-2019-5786 (Windows)
- Many others in browsers, OS kernels, etc.

Attackers can:
- Control what data is written to freed memory
- Corrupt function pointers
- Execute arbitrary code

## Detection Commands

```bash
# Build with ASan
gcc -g -O0 -fsanitize=address -fno-omit-frame-pointer use_after_free.c -o use_after_free_asan

# Run with Valgrind
valgrind --free-fill=FF --track-origins=yes ./use_after_free

# Use NDB to set watchpoints
ndb> watchpoint <address>
```

## Summary

Use-after-free bugs occur when:
1. Memory is freed
2. A pointer to that memory still exists
3. The program tries to access the memory through that pointer

Prevention:
- NULL pointers after free
- Use tools like ASan and Valgrind
- Copy data you need to keep
- Be careful with inner pointers
- Follow ownership rules clearly
