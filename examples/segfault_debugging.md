# Debugging Session: Segmentation Fault (Null Pointer Dereference)

## Overview

This transcript demonstrates how to debug a segmentation fault caused by dereferencing a NULL pointer using NDB debugger.

## The Bug

**File:** `segfault_example.c`

The program crashes when trying to print a `Student` structure through a NULL pointer. The `print_student()` function doesn't validate that the pointer is valid before dereferencing it.

## Compilation

```bash
# Compile with debug symbols
gcc -g -O0 segfault_example.c -o segfault_example

# Or compile the project version
make segfault_example
```

## NDB Debugging Session

```
$ ./target/main_interactive ./segfault_example
NDB Debugger v0.1
Type 'help' for commands

ndb> run
Starting program: ./segfault_example

=== Segfault Example ===

Creating student 1...
Student ID: 1
Name: Alice
Score: 95.50

Creating student 2 (simulating malloc failure)...
Attempting to print student 2...

Program received signal SIGSEGV, Segmentation fault.
0x00000000004011b6 in print_student (student=0x0) at segfault_example.c:23
23	    printf("Student ID: %d\n", student->id);
```

### Analysis Steps

#### 1. Examine the Crash Location

```
ndb> backtrace
#0  0x00000000004011b6 in print_student (student=0x0) at segfault_example.c:23
#1  0x00000000004012a8 in main () at segfault_example.c:63
```

The backtrace shows:
- We crashed in `print_student()` at line 23
- The parameter `student` is `0x0` (NULL)
- We were called from `main()` at line 63

#### 2. View the Crashing Code

```
ndb> list
18      void print_student(Student *student) {
19          // BUG: No NULL check here!
20          printf("Student ID: %d\n", student->id);
21          printf("Name: %s\n", student->name);
22          printf("Score: %.2f\n", student->score);
23      }
```

#### 3. Examine Variables

```
ndb> print student
$1 = (Student *) 0x0

ndb> print *student
Cannot access memory at address 0x0
```

The pointer is NULL! We can't dereference it.

#### 4. Check the Call Site

```
ndb> frame 1
#1  0x00000000004012a8 in main () at segfault_example.c:63
63	    print_student(s2);

ndb> print s2
$2 = (Student *) 0x0
```

In `main()`, we're passing `s2` which is NULL to `print_student()`.

#### 5. Find Where s2 Was Set

```
ndb> list 58-65
58	    // This will cause a segfault
59	    printf("Creating student 2 (simulating malloc failure)...\n");
60	    Student *s2 = NULL;  // Simulating allocation failure
61	    // In real code, this might happen if malloc fails
62	    // or if a function returns NULL on error
63
64	    printf("Attempting to print student 2...\n");
65	    print_student(s2);  // SEGFAULT HERE!
```

Line 60 shows that `s2` is explicitly set to NULL to simulate a failed allocation.

## Root Cause

The bug is in `print_student()` - it doesn't check if the `student` pointer is NULL before dereferencing it. When `malloc()` fails (or is simulated to fail), we get a NULL pointer, and attempting to access `student->id` causes a segmentation fault.

## The Fix

Add a NULL check at the beginning of `print_student()`:

```c
void print_student(Student *student) {
    // FIX: Check for NULL pointer
    if (student == NULL) {
        printf("Error: Student pointer is NULL!\n");
        return;
    }

    printf("Student ID: %d\n", student->id);
    printf("Name: %s\n", student->name);
    printf("Score: %.2f\n", student->score);
}
```

Or better yet, check the return value of `create_student()` in `main()`:

```c
Student *s2 = create_student(2, "Bob", 87.5);
if (s2 == NULL) {
    printf("Error: Failed to create student!\n");
    return 1;
}
print_student(s2);
free(s2);
```

## What We Learned

1. **Always check pointer values** before dereferencing them
2. **Always check malloc() return values** - they can fail!
3. **Use backtrace** to see the call chain leading to the crash
4. **Examine function parameters** to see what values are being passed
5. **NULL pointers dereference** cause immediate segfaults with clear error messages

## Additional NDB Commands Used

- `backtrace` or `bt` - Show call stack
- `frame <n>` - Switch to stack frame
- `list` - Show source code
- `print <var>` - Show variable value
- `info registers` - Show CPU registers (would show RAX=0)
- `disassemble` - Show assembly around crash

## Prevention

- Enable compiler warnings: `-Wall -Wextra`
- Use static analysis tools
- Use sanitizers: `-fsanitize=undefined,address`
- Always validate pointers before use
- Use assertions: `assert(student != NULL);`
