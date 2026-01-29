# NDB Debugger - Complete Tutorial

## Table of Contents

1. [Getting Started](#getting-started)
   - [What is a Debugger?](#what-is-a-debugger)
   - [Installation](#installation)
   - [Basic Concepts](#basic-concepts)
   - [Your First Debugging Session](#your-first-debugging-session)

2. [Basic Debugging Workflow](#basic-debugging-workflow)
   - [Starting a Debugging Session](#starting-a-debugging-session)
   - [Setting Breakpoints](#setting-breakpoints)
   - [Stepping Through Code](#stepping-through-code)
   - [Inspecting Variables and Memory](#inspecting-variables-and-memory)
   - [Viewing the Call Stack](#viewing-the-call-stack)

3. [Intermediate Features](#intermediate-features)
   - [Conditional Breakpoints](#conditional-breakpoints)
   - [Watchpoints](#watchpoints)
   - [Disassembly View](#disassembly-view)
   - [Register Inspection](#register-inspection)

4. [Practical Examples](#practical-examples)
   - [Debugging a Segfault Crash](#debugging-a-segfault-crash)
   - [Debugging a Memory Issue](#debugging-a-memory-issue)
   - [Finding a Logic Error](#finding-a-logic-error)
   - [Debugging an Infinite Loop](#debugging-an-infinite-loop)

5. [Tips and Tricks](#tips-and-tricks)
   - [Keyboard Shortcuts](#keyboard-shortcuts)
   - [Efficient Debugging Workflows](#efficient-debugging-workflows)
   - [Troubleshooting Common Issues](#troubleshooting-common-issues)

---

## Getting Started

### What is a Debugger?

A debugger is a tool that allows you to inspect and control the execution of a program. Think of it as having X-ray vision and time-travel capabilities for your code. With a debugger, you can:

- **Pause** your program at any point
- **Step** through code line by line or instruction by instruction
- **Inspect** variable values and memory contents
- **Modify** variables and registers on the fly
- **Analyze** crashes and errors as they happen

Debuggers are essential for:
- Finding and fixing bugs
- Understanding how code works
- Learning about low-level program behavior
- Performance analysis

### Installation

#### Prerequisites

NDB requires several libraries to be installed on your system. On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libdwarf-dev libunwind-dev libunwind-ptrace-dev
sudo apt install libreadline-dev liblinenoise-dev libargtable3-dev
```

#### Building NDB

```bash
# Clone or navigate to the debugger directory
cd /home/sbstndbs/debugger

# Create a build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the debugger
make

# The debugger binary will be at ../target/debug_console
# or ../target/main_interactive
```

#### Verifying Installation

```bash
# Check that the debugger exists
ls -l target/debug_console
ls -l target/main_interactive

# Get help on command-line options
./target/debug_console --help
```

### Basic Concepts

Before diving in, let's understand some fundamental debugging concepts:

#### Breakpoint

A breakpoint is a marker you place in your code that tells the debugger to pause execution when it reaches that point. The program runs normally until it hits a breakpoint, then it stops and gives you control.

```
Normal execution  --->  [BREAKPOINT]  --->  PAUSED (inspect/debug)
                      |                     |
                      v                     v
                 resumes execution    continue stepping
```

#### Stepping

Once paused, you can control execution:

- **Step (s)**: Execute one machine instruction. If it's a function call, step INTO the function.
- **Next (n)**: Execute one instruction, but step OVER function calls (don't enter them).
- **Continue (c)**: Resume normal execution until next breakpoint or signal.

```
    main()          helper_function()
    --------        ----------------
    x = 5;      -->    y = x * 2;
    call helper()     return y;
    z = 10;      ----------------
    print(z);

    Using "step"  at "call helper"   -> enters helper_function
    Using "next"  at "call helper"   -> skips to "z = 10"
```

#### Call Stack

The call stack (or backtrace) shows the chain of function calls that led to the current point:

```
Frame 2: main()
Frame 1: process_data()
Frame 0: calculate_result()  <-- Currently here
```

Each frame contains:
- The function that was called
- The location in that function
- Local variables and parameters

#### Registers

CPU registers are the fastest storage locations in a computer. The most important ones:

- **RIP**: Instruction Pointer - points to the next instruction to execute
- **RSP**: Stack Pointer - points to the top of the stack
- **RBP**: Base Pointer - used to reference local variables
- **RAX, RBX, RCX, RDX**: General-purpose registers

#### Memory vs. Source Code

Debuggers operate at two levels:

1. **Source Level**: Lines of C code, variable names, function names
2. **Machine Level**: Memory addresses, CPU instructions, registers

NDB provides tools for both levels.

### Your First Debugging Session

Let's start with a simple program. Create a file called `first_debug.c`:

```c
#include <stdio.h>

int main() {
    printf("Program started\n");
    int x = 42;
    printf("x = %d\n", x);
    x = 100;
    printf("x = %d\n", x);
    printf("Program ended\n");
    return 0;
}
```

Compile it with debug symbols:

```bash
gcc -g -o first_debug first_debug.c
```

Now start the debugger:

```bash
./target/debug_console ./first_debug
```

You should see something like:

```
dbg> _
```

This is NDB's prompt. Let's try some basic commands:

```
dbg> help
# Shows all available commands

dbg> info functions
# Lists all functions in the program
# Output:
# 0x400500: main
# 0x4003e0: _start

dbg> break main
# Sets a breakpoint at the main function
# Output:
# Breakpoint set at 0x400500 (main)

dbg> breakpoint list
# Shows all breakpoints
```

Now let's run the program:

```
dbg> continue
# Output:
# [Process started: PID 12345]
# [Breakpoint hit #0 at 0x400500 in main]
```

The program has stopped at `main`. Let's explore:

```
dbg> register read rip
# Output: rip = 0x400500

dbg> disas
# Shows assembly code around current position

dbg> step
# Execute one instruction

dbg> step
# Execute another instruction

dbg> continue
# Continue to end of program
```

Congratulations! You've completed your first debugging session.

---

## Basic Debugging Workflow

### Starting a Debugging Session

There are two ways to start debugging with NDB:

#### Method 1: Start with a New Program

```bash
./target/debug_console ./your_program [args...]
```

NDB will:
1. Fork and trace the program
2. Wait for initial signal from execve
3. Display the prompt, ready for commands

#### Method 2: View Process Information

When the program starts, NDB extracts and displays:

```
[Process Info]
PID: 12345
Base Address: 0x400000
Entry Point: 0x4004e0
```

The base address is important for PIE (Position Independent Executable) binaries.

### Setting Breakpoints

Breakpoints are your primary tool for controlling program execution.

#### Break by Function Name

```
dbg> break main
# Output: Breakpoint set at 0x4004f0 (main)

dbg> break helper_function
# Output: Breakpoint set at 0x400520 (helper_function)
```

#### Break by Address

```
dbg> break 0x400500
# Output: Breakpoint set at 0x400500

dbg> break 0x401000
# Output: Breakpoint set at 0x401000
```

#### Break at Current Position

```
dbg> break
# Sets breakpoint at current RIP value
# Output: Breakpoint set at 0x400500
```

#### List Breakpoints

```
dbg> breakpoint list
# Or simply: bl

# Output:
# #0: addr=0x4004f0 enabled hits=0 main
# #1: addr=0x400520 enabled hits=0 helper_function
```

The output shows:
- Breakpoint index (#0, #1)
- Runtime address
- Enabled/disabled status
- Hit count
- Function name (if available)

#### Enable/Disable Breakpoints

Sometimes you want to temporarily disable a breakpoint without deleting it:

```
dbg> breakpoint disable 0
# Disable breakpoint #0

dbg> breakpoint enable 0
# Re-enable breakpoint #0

dbg> bl
# #0: addr=0x4004f0 disabled hits=0 main
# #1: addr=0x400520 enabled hits=1 helper_function
```

#### Delete Breakpoints

```
dbg> breakpoint delete 0
# Permanently remove breakpoint #0
```

### Stepping Through Code

Once you hit a breakpoint, you can step through your program.

#### Single Instruction Stepping

The `step` command executes one machine instruction:

```c
// Example code:
int x = 5;
int y = x + 10;
```

```
dbg> step
# Output: [Single step at 0x400503]
# Instruction: mov dword ptr [rbp-0x4], 0x5

dbg> step
# Output: [Single step at 0x40050a]
# Instruction: mov edx, dword ptr [rbp-0x4]

dbg> register read rax
# Check register values after each step
```

#### Step Over Functions

The `next` command is like `step`, but treats function calls as one instruction:

```c
int result = helper_function(42);  // <-- Stepping HERE
print(result);
```

Using `step`:
```
dbg> step
# Jumps INTO helper_function
# You're now stepping through helper_function's code
```

Using `next`:
```
dbg> next
# Executes helper_function completely
# Stops at the NEXT line (print)
# You didn't have to step through helper_function
```

#### Continue Execution

The `continue` command resumes normal execution:

```
dbg> continue
# Program runs until:
# - Next breakpoint is hit
# - A signal is received (like SIGSEGV)
# - Program exits

# Output when hitting a breakpoint:
# [Breakpoint hit #1 at 0x400520 in helper_function]
# RIP: 0x400520

# Output when program exits:
# [Process exited with code 0]
```

#### Example Debugging Session

Let's step through a simple function:

```c
int add(int a, int b) {
    int result = a + b;
    return result;
}

int main() {
    int x = 5;
    int y = 10;
    int sum = add(x, y);
    printf("sum = %d\n", sum);
    return 0;
}
```

Debugging session:

```
dbg> break main
dbg> break add
dbg> continue
# [Breakpoint hit at main]

dbg> step
# x = 5

dbg> step
# y = 10

dbg> next
# [Calls add() and stops at printf]
# We used 'next' to skip over add's internals

dbg> continue
# Program completes
```

### Inspecting Variables and Memory

#### Reading Memory

Use the `memory` command to read memory at a specific address:

```
dbg> memory 0x7fffffffe000
# Or: m 0x7fffffffe000

# Output:
# Memory at 0x7fffffffe000: 0x000000000000002a
# Decimal: 42, Hex: 0x2a
```

This shows 8 bytes (64 bits) at the specified address.

#### Finding Variable Addresses

Variables live in memory at specific addresses. You can find them using:

1. **From disassembly**: Look at stack offsets
2. **From registers**: Check RBP-relative offsets

Example:
```
dbg> disas
# Output:
# 0x400500: main()
#   400500:  push rbp
#   400501:  mov rbp, rsp
#   400504:  mov dword ptr [rbp-0x4], 0x2a    ; int x = 42
#   40050b:  mov eax, dword ptr [rbp-0x4]
```

Here, variable `x` is at `[rbp-0x4]`. To read it:

```
dbg> register read rbp
# rbp = 0x7fffffffe000

dbg> memory 0x7fffffffdfc
# 0x7fffffffe000 - 0x4 = 0x7fffffffdfc
# Output: Memory at 0x7fffffffdfc: 0x0000002a
```

#### Writing to Memory

You can modify memory contents:

```
dbg> memory write 0x7fffffffdfc 0x64
# Write 0x64 (100 decimal) to the address

# Output:
# Wrote 0x64 to 0x7fffffffdfc
# Previous value: 0x0000002a
```

This directly modifies the program's memory!

#### Inspecting the Stack

The stack grows downward and contains:
- Local variables
- Function parameters
- Return addresses

```
dbg> register read rsp
# rsp = 0x7fffffffdfc0

dbg> memory 0x7fffffffdfc0
# Look at top of stack

dbg> memory 0x7fffffffdfc0
dbg> memory 0x7fffffffdfc8
dbg> memory 0x7fffffffdfd0
# Walk down the stack
```

### Viewing the Call Stack

The backtrace command shows the call stack:

```
dbg> backtrace
# Or: bt

# Output:
# #0 0x400520  calculate_result(x=5, y=10)
# #1 0x4004f0  process_data(value=100)
# #2 0x400480  main()
```

Each frame shows:
- Frame number (#0 is current)
- Instruction pointer address
- Function name with parameters (if available)

#### Practical Example: Understanding Recursion

Consider this recursive function:

```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

Debugging `factorial(5)`:

```
dbg> break factorial
dbg> continue
# [Breakpoint hit, n=5]

dbg> bt
# #0 factorial(n=5)
# #1 main()

dbg> step  # recurse deeper
dbg> bt
# #0 factorial(n=4)
# #1 factorial(n=5)
# #2 main()

dbg> step  # deeper
dbg> bt
# #0 factorial(n=3)
# #1 factorial(n=4)
# #2 factorial(n=5)
# #3 main()
```

Each recursive call adds a stack frame!

---

## Intermediate Features

### Conditional Breakpoints

**Note:** NDB currently supports basic breakpoints. For complex conditions, you can combine breakpoints with manual inspection.

#### Simulating Conditional Breakpoints

If you want to stop only when a condition is met:

```
# Set breakpoint
dbg> break loop_function
dbg> continue

# At each hit, check the condition manually
dbg> register read rax
# If rax != expected value, continue:
dbg> continue

# Repeat until condition is met
```

Example - Stop when loop counter reaches specific value:

```c
for (int i = 0; i < 100; i++) {
    process(i);
}
```

```
dbg> break process
dbg> continue

# Hit #1: i = 0
dbg> register read edi  # first parameter register
# edi = 0
# Not what we want, continue
dbg> continue

# Hit #2: i = 1
dbg> register read edi
# edi = 1
# Still not there
dbg> continue

# ...repeat until edi == 50
```

### Watchpoints

Watchpoints allow you to monitor memory addresses and break when they're written to. Unlike breakpoints (which pause at specific code locations), watchpoints pause when specific data changes.

#### Setting a Watchpoint

```
dbg> watch 0x7fffffffe000
# Or: w 0x7fffffffe000

# Output:
# Watchpoint set at 0x7fffffffe000
# Using debug register DR0
```

#### Listing Watchpoints

```
dbg> watchpoint list
# Or: wl

# Output:
# #0: addr=0x7fffffffe000 type=write enabled
```

#### How Watchpoints Work

Watchpoints use hardware debug registers (DR0-DR3 on x86-64). This means:

- **Limited quantity**: Only 4 watchpoints at a time
- **Hardware-based**: Zero performance overhead when not hit
- **Write-only**: Currently supports write watchpoints

#### Practical Example: Detecting Unauthorized Memory Access

```c
int sensitive_data = 42;

void corrupt_data() {
    sensitive_data = 999;  // Bug: unauthorized modification
}

int main() {
    printf("Initial: %d\n", sensitive_data);
    corrupt_data();
    printf("After: %d\n", sensitive_data);
    return 0;
}
```

Debugging:

```
dbg> break main
dbg> continue

# Find address of sensitive_data
dbg> disas
# Look for: mov dword ptr [rbp-0x4], 0x2a
# Assume it's at 0x7fffffffe004

dbg> watch 0x7fffffffe004
# Watchpoint set

dbg> continue
# Program runs...

# [Watchpoint hit #0 at 0x7fffffffe004]
# Old value: 0x0000002a (42)
# New value: 0x000003e7 (999)
# RIP: 0x400520 (in corrupt_data)

# Aha! The data was modified in corrupt_data()
```

#### Deleting Watchpoints

```
dbg> watchpoint delete 0
# Remove watchpoint #0
```

### Disassembly View

The disassembly feature shows you the actual machine instructions your code compiles to.

#### Basic Disassembly

```
dbg> disas
# Disassemble around current RIP

# Output:
# Disassembly around 0x400500:
#   0x400500:  push rbp
#   0x400501:  mov rbp, rsp
#   0x400504:  mov dword ptr [rbp-0x4], 0x2a
#   0x40050b:  mov eax, dword ptr [rbp-0x4]
#   0x40050e:  pop rbp
#   0x40050f:  ret
# → 0x400510:  mov edi, eax
#   0x400512:  call printf
#   0x400517:  ...
```

The arrow (→) shows the current instruction.

#### Disassemble a Specific Address

```
dbg> disas 0x400600
# Disassemble around address 0x400600
```

#### Disassemble a Function

```
dbg> disas main
# Disassemble the main function

dbg> disas helper_function
# Disassemble helper_function
```

#### Controlling Context

Use `--before` and `--after` to control how much to show:

```
dbg> disas --before=10 --after=10
# Show 10 instructions before and after current RIP

dbg> disas main --before=5 --after=20
# Show 5 before, 20 after start of main
```

#### Reading Assembly: A Quick Guide

Common x86-64 instructions:

```
mov   dest, src     ; Move data
push  reg           ; Push register onto stack
pop   reg           ; Pop from stack
call  addr          ; Call function
ret                  ; Return from function
cmp   a, b          ; Compare a and b
jmp   addr          ; Unconditional jump
je    addr          ; Jump if equal
jne   addr          ; Jump if not equal
```

Addressing modes:

```
mov rax, 0x42              ; Immediate: load constant 0x42
mov rax, rbx               ; Register: copy value from rbx
mov rax, [rbp-0x4]         ; Memory: load from stack variable
mov [rbp-0x8], rax         ; Memory: store to stack variable
```

### Register Inspection

Registers are the CPU's internal storage. Inspecting them helps you understand low-level behavior.

#### Dump All Registers

```
dbg> register dump
# Or: r

# Output:
# RAX: 0x000000000000002a  (42)
# RBX: 0x00007fffffffe000
# RCX: 0x0000000000000000  (0)
# RDX: 0x0000000000400500
# RSI: 0x00007fffffffe100
# RDI: 0x0000000000000005  (5)
# RBP: 0x00007fffffffe000
# RSP: 0x00007fffffffdfc0
# RIP: 0x0000000000400500
# R8:  0x0000000000000000
# R9:  0x0000000000000000
# R10: 0x0000000000000000
# R11: 0x0000000000000246
# R12: 0x0000000000000000
# R13: 0x0000000000000000
# R14: 0x0000000000000000
# R15: 0x0000000000000000
```

#### Read a Specific Register

```
dbg> register read rax
# Output: rax = 0x2a (42)

dbg> register read rip
# Output: rip = 0x400500

dbg> register read rsp
# Output: rsp = 0x7fffffffdfc0
```

#### Write to a Register

You can modify register values:

```
dbg> register write rax 0x100
# Set rax to 0x100

# Output:
# Wrote 0x100 to rax
# Previous value: 0x2a
```

**Warning**: Modifying registers can crash your program if done incorrectly!

#### Understanding Key Registers

**RIP (Instruction Pointer)**
- Points to the next instruction to execute
- Automatically updated by CPU
- You can write to it (but be careful!)

```
dbg> register read rip
# rip = 0x400500

dbg> register write rip 0x400600
# Jump to address 0x400600
# Use with caution!
```

**RSP (Stack Pointer)**
- Points to the top of the stack
- Stack grows downward (toward lower addresses)
- Used for function calls, local variables

**RBP (Base Pointer)**
- Used as a reference point for local variables
- Typically set to RSP at function start
- Variables accessed as `[rbp-offset]`

**RDI, RSI, RDX, RCX, R8, R9**
- Used for function parameters (in order)
- RDI = 1st parameter, RSI = 2nd, etc.

Example:
```c
int func(int a, int b, int c);
```

When calling `func(1, 2, 3)`:
- `edi` = 1 (first parameter)
- `rsi` = 2 (second parameter)
- `rdx` = 3 (third parameter)

---

## Practical Examples

### Debugging a Segfault Crash

A segmentation fault (SIGSEGV) occurs when your program tries to access invalid memory. Let's debug one.

#### The Buggy Program

Create `segfault_example.c`:

```c
#include <stdio.h>
#include <stdlib.h>

struct Point {
    int x;
    int y;
};

struct Point* create_point(int x, int y) {
    struct Point* p = malloc(sizeof(struct Point));
    p->x = x;
    p->y = y;
    return p;
}

void print_point(struct Point* p) {
    printf("Point: (%d, %d)\n", p->x, p->y);
}

int main() {
    printf("Creating points...\n");

    struct Point* p1 = create_point(10, 20);
    struct Point* p2 = NULL;  // Bug: forgot to initialize!

    printf("Printing points...\n");
    print_point(p1);
    print_point(p2);  // This will crash!

    free(p1);
    free(p2);  // This will also crash!

    return 0;
}
```

Compile and run:

```bash
gcc -g -o segfault_example segfault_example.c
```

#### Debugging Session

```
dbg> ./target/debug_console ./segfault_example

[Process started: PID 12345]

dbg> continue
# Let the program run until it crashes

# [Signal received: SIGSEGV (Segmentation fault)]
# Fault address: 0x0
# RIP: 0x400520
```

The program crashed! Now let's investigate:

```
dbg> backtrace
# #0 0x400520  print_point(p=0x0)
# #1 0x400480  main()

# Aha! print_point was called with p=NULL (0x0)
```

Let's look at the crash location:

```
dbg> disas
#   0x400515:  push rbp
#   0x400516:  mov rbp, rsp
# → 0x400519:  mov eax, dword ptr [rdi]   ; Crash here!
#   0x40051b:  mov esi, eax
#   0x40051d:  mov eax, dword ptr [rdi+0x4]
```

The crash happens when trying to read from `[rdi]`. RDI holds the first parameter (p), which is NULL!

```
dbg> register read rdi
# rdi = 0x0

# Confirmed: p is NULL
```

Now let's find where this was called from:

```
dbg> list
# If available, shows source code

dbg> info functions
# Look for main

dbg> disas main
# Find the call to print_point
#   0x400475:  call print_point  ; p2 passed here
```

Let's check what's in main at the call site:

```
dbg> break main
dbg> continue
# [Breakpoint hit at main]

dbg> step
dbg> step
# ... step until p2 is assigned

dbg> register read rax
# After: p2 = NULL
# rax = 0x0
```

The bug is clear: `p2` was set to NULL but never initialized with `create_point()`!

#### The Fix

```c
// Change:
struct Point* p2 = NULL;

// To:
struct Point* p2 = create_point(30, 40);
```

### Debugging a Memory Issue

Let's debug a double-free bug (freeing memory twice).

#### The Buggy Program

Create `memory_bug.c`:

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int* data = malloc(sizeof(int));
    *data = 42;

    printf("Value: %d\n", *data);

    free(data);
    printf("Freed once\n");

    free(data);  // Bug: double free!
    printf("Freed twice\n");

    return 0;
}
```

Compile:
```bash
gcc -g -o memory_bug memory_bug.c
```

#### Debugging with Watchpoints

We'll use a watchpoint to detect when `data` is modified.

```
dbg> ./target/debug_console ./memory_bug

dbg> break main
dbg> continue

# [Breakpoint hit at main]

# Find address of 'data' variable
dbg> disas
# Look for: mov qword ptr [rbp-0x8], rax
# Assume data is at [rbp-0x8]

dbg> register read rbp
# rbp = 0x7fffffffe000

# Calculate data's address
dbg> memory 0x7ffffffdf8
# This is where 'data' pointer is stored

# Now watch the memory that data points to
dbg> memory 0x7ffffffdf8
# Read the pointer value: 0x555555558000

dbg> watch 0x555555558000
# Watch the allocated memory

dbg> continue
# Program runs, first free happens

# [Watchpoint hit: write to 0x555555558000]
# Something modified the watched memory

# Continue...
dbg> continue

# [Signal received: SIGSEGV]
# Double-free detected by system
```

The watchpoint helped us track memory modifications!

### Finding a Logic Error

Logic errors don't crash - they produce wrong results. Let's find one.

#### The Buggy Program

Create `logic_bug.c`:

```c
#include <stdio.h>

// Calculate average of two numbers
int average(int a, int b) {
    int sum = a + b;
    int avg = sum / 2;  // Bug: integer division!
    return avg;
}

// Calculate factorial
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    printf("Testing average...\n");

    int avg1 = average(10, 20);
    printf("Average of 10 and 20: %d\n", avg1);

    int avg2 = average(15, 16);
    printf("Average of 15 and 16: %d\n", avg2);  // Should be 15.5, gets 15!

    printf("\nTesting factorial...\n");

    int fact = factorial(5);
    printf("5! = %d\n", fact);

    return 0;
}
```

#### Debugging the Average Bug

```
dbg> ./target/debug_console ./logic_bug

dbg> break average
dbg> continue

# [Breakpoint hit at average(10, 20)]

dbg> register read edi
# edi = 10 (first parameter)

dbg> register read esi
# esi = 20 (second parameter)

dbg> step
# sum = 10 + 20 = 30

dbg> register read eax
# eax = 30

dbg> step
# avg = 30 / 2 = 15

dbg> register read eax
# eax = 15
# Correct so far

dbg> continue
# Program completes, avg1 = 15

# Run again with different values
dbg> break average
dbg> continue

# [Breakpoint hit at average(15, 16)]

dbg> step
dbg> step
# avg = 31 / 2 = 15

# But 15.5 would be more accurate!
# The bug: integer division truncates
```

#### The Fix

Use floating-point or improve the logic:

```c
// Option 1: Use floats
float average(int a, int b) {
    return (a + b) / 2.0f;
}

// Option 2: Better integer logic
int average(int a, int b) {
    return (a + b + ((a + b) % 2)) / 2;  // Round to nearest
}
```

### Debugging an Infinite Loop

Infinite loops cause programs to hang. Let's debug one.

#### The Buggy Program

Create `infinite_loop.c`:

```c
#include <stdio.h>

int main() {
    int count = 0;
    int sum = 0;

    // Bug: forgot to increment count!
    while (count < 10) {
        sum += count;
        printf("count=%d, sum=%d\n", count, sum);
        // Missing: count++;
    }

    printf("Final sum: %d\n", sum);
    return 0;
}
```

#### Debugging Session

```
dbg> ./target/debug_console ./infinite_loop

dbg> break main
dbg> continue

# [Breakpoint hit at main]

dbg> disas
# Find the while loop

# Set breakpoint at loop start
dbg> break 0x400510  # Address of while loop
dbg> continue

# [Breakpoint hit, iteration 1]
dbg> register read eax
# eax = 0 (count)

dbg> continue
# [Breakpoint hit, iteration 2]
dbg> register read eax
# eax = 0 (count didn't change!)

dbg> continue
# [Breakpoint hit, iteration 3]
dbg> register read eax
# eax = 0 (still 0!)

# Aha! count is never incremented
```

The loop never ends because `count` stays at 0, which is always < 10!

#### The Fix

```c
while (count < 10) {
    sum += count;
    printf("count=%d, sum=%d\n", count, sum);
    count++;  // Add this!
}
```

---

## Tips and Tricks

### Keyboard Shortcuts

NDB supports readline/linenoise for command editing:

**Navigation:**
- `Ctrl+A`: Move to beginning of line
- `Ctrl+E`: Move to end of line
- `Ctrl+B`: Move backward one character
- `Ctrl+F`: Move forward one character
- `Alt+B`: Move backward one word
- `Alt+F`: Move forward one word

**Editing:**
- `Ctrl+U`: Delete from cursor to beginning
- `Ctrl+K`: Delete from cursor to end
- `Ctrl+W`: Delete word before cursor
- `Ctrl+Y`: Paste deleted text

**History:**
- `Up/Down arrows`: Navigate command history
- `Ctrl+R`: Search command history
- `Ctrl+G`: Cancel search

**Miscellaneous:**
- `Ctrl+L`: Clear screen
- `Ctrl+C`: Send SIGINT to program (if running)
- `Ctrl+D`: Exit debugger

### Efficient Debugging Workflows

#### Workflow 1: The "Binary Search" Method

When you don't know where the bug is:

```
1. Set breakpoint in the middle of your code
2. Run and check if bug has occurred yet
3. If yes, bug is in first half
4. If no, bug is in second half
5. Repeat with the relevant half

Example:
dbg> break middle_function
dbg> continue
# Check state
# If bug visible: break early_function
# If bug not visible: break late_function
```

#### Workflow 2: Crash Investigation

```
1. Let program run to crash
dbg> continue
# [Signal: SIGSEGV]

2. Check backtrace
dbg> bt
# See where you are

3. Examine crash location
dbg> disas
dbg> register read rip
dbg> memory [fault_address]

4. Go up the stack
dbg> up  # Not available, use bt to find frame
dbg> frame [address]

5. Check variables in parent
dbg> memory [variable_addresses]
```

#### Workflow 3: Variable Tracking

```
1. Find variable address
dbg> disas function_name
# Look for [rbp-offset] references

2. Calculate actual address
dbg> register read rbp
dbg> memory [rbp - offset]

3. Set watchpoint (if needed)
dbg> watch [address]

4. Trace modifications
dbg> continue
# Watch for watchpoint hits
```

#### Workflow 4: Function Entry/Exit

```
1. Set breakpoints at function boundaries
dbg> break function_start
dbg> break function_end

2. Log parameters on entry
dbg> continue
# [Hit function_start]
dbg> register read rdi  # param 1
dbg> register read rsi  # param 2
# ... record these

3. Log return value on exit
dbg> continue
# [Hit function_end]
dbg> register read rax  # return value
```

### Troubleshooting Common Issues

#### Issue: "ptrace: Operation not permitted"

**Cause:** Security restrictions on ptrace

**Solutions:**
```bash
# Temporarily disable ptrace restrictions
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope

# Or run as root (not recommended for normal debugging)
sudo ./target/debug_console ./your_program
```

#### Issue: Breakpoints not being hit

**Possible causes:**

1. **Wrong address**: PIE binary address mismatch
   ```
   # Check if PIE:
   file your_program
   # Look for: "interpreter /lib64/ld-linux-x86-64.so.2"

   # NDB handles PIE automatically, but verify:
   dbg> info functions
   # Check addresses make sense
   ```

2. **Code not executed**: The breakpoint location is never reached
   ```
   # Verify program flow:
   dbg> break main
   dbg> continue
   dbg> disas
   # Step through to see actual path
   ```

3. **Optimization removed code**: Compiler optimized away your code
   ```
   # Recompile with -O0:
   gcc -g -O0 -o program program.c
   ```

#### Issue: Can't see source code

**Cause:** Debug symbols not included

**Solution:**
```bash
# Compile with -g flag:
gcc -g -o program program.c

# Verify symbols exist:
file program
# Should show: "with debug_info"

# Check with nm:
nm program | grep main
```

#### Issue: Program runs too fast

**Problem:** Can't set breakpoints fast enough

**Solution:**
```bash
# 1. Start with breakpoint at main:
dbg> break main

# 2. Then set other breakpoints while stopped:
dbg> break function_name
dbg> break 0xaddress

# 3. Now continue
dbg> continue
```

#### Issue: Backtrace is incomplete or wrong

**Causes:**

1. **Missing libunwind**: Install it
   ```bash
   sudo apt install libunwind-dev libunwind-ptrace-dev
   ```

2. **Optimized code**: Recompile with -O0
   ```bash
   gcc -g -O0 -o program program.c
   ```

3. **Corrupted stack**: Check RSP/RBP
   ```
   dbg> register read rsp
   dbg> register read rbp
   # They should look reasonable (not NULL, not garbage)
   ```

#### Issue: Watchpoints don't work

**Causes:**

1. **No available debug registers**: Only 4 watchpoints at a time
   ```
   dbg> watchpoint list
   # Delete unused watchpoints
   dbg> watchpoint delete 0
   ```

2. **Address not aligned**: Watchpoints need aligned addresses
   ```
   # For 4-byte watchpoint, address must be multiple of 4
   # For 8-byte watchpoint, address must be multiple of 8
   ```

3. **Non-writable memory**: Can't watch read-only memory
   ```
   # Check memory permissions:
   cat /proc/PID/maps
   # Look for 'r--p' (read-only)
   ```

### Best Practices

1. **Always compile with -g**: Include debug symbols
2. **Use -O0 for debugging**: Disable optimizations
3. **Start simple**: Set one breakpoint, verify, then add more
4. **Save your debugging session**: Copy-paste commands to a file
5. **Use descriptive notes**: Comment your debugging session
   ```
   dbg> break 0x400500  # Entry to buggy function
   dbg> break 0x400620  # After crash happens
   ```
6. **Verify assumptions**: Don't assume - verify with `disas` and `register read`
7. **Check both code and data**: Bugs can be in either
8. **Use watchpoints sparingly**: You only have 4
9. **Keep a clean build**: Rebuild after code changes
10. **Document your findings**: Write down what you discovered

### Advanced Tips

#### Tip 1: Use Shell Scripts for Repetitive Tasks

Create a file `debug_session.txt`:
```
break main
break process_data
continue
register read rdi
memory 0x7fffffffe000
```

Run it:
```bash
cat debug_session.txt | ./target/debug_console ./your_program
```

#### Tip 2: Batch Testing

Test multiple scenarios:
```bash
for i in {1..10}; do
    ./target/debug_console ./program $i <<EOF
break main
continue
quit
EOF
done
```

#### Tip 3: Log Debugging Sessions

```bash
./target/debug_console ./program 2>&1 | tee debug.log
```

#### Tip 4: Compare Good vs. Bad

If you have a working version and a broken version:

```bash
# Terminal 1: Debug working version
./target/debug_console ./program_good

# Terminal 2: Debug broken version
./target/debug_console ./program_bad

# Compare register values, memory, etc.
```

#### Tip 5: Use Address Sanitizer

Combine NDB with AddressSanitizer for better error detection:
```bash
gcc -g -O0 -fsanitize=address -o program program.c
./target/debug_console ./program
```

---

## Conclusion

You've now learned the fundamentals of using NDB to debug C programs. Here's what you've mastered:

- Getting started with NDB and basic debugging concepts
- Setting and managing breakpoints
- Stepping through code with step, next, and continue
- Inspecting variables, memory, and registers
- Using watchpoints to track memory changes
- Debugging common issues like segfaults, memory bugs, logic errors, and infinite loops
- Efficient debugging workflows and troubleshooting

### Next Steps

1. **Practice**: Debug your own programs with NDB
2. **Explore**: Try all commands and features
3. **Read**: Check out the [Command Reference](commands.md) for complete command documentation
4. **Contribute**: Consider contributing to NDB development

### Additional Resources

- [NDB Command Reference](commands.md) - Complete command documentation
- [NDB Architecture](architecture.md) - Internal design and implementation
- [ptrace(2) man page](https://man7.org/linux/man-pages/man2/ptrace.2.html) - Learn about ptrace system calls
- [x86-64 Assembly Guide](https://www.felixcloutier.com/x86/) - Intel architecture reference

Happy debugging!

---

*This tutorial covers NDB version 1.0. For the latest updates and features, check the project repository.*
