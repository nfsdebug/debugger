# Line-Based Breakpoint Quick Start Guide

## Building

```bash
# Build the debugger with DWARF support
make clean
make

# Verify it built successfully
ls -l target/main_interactive
```

## Testing

```bash
# Run the comprehensive test suite
./test_line_breakpoint_comprehensive.sh
```

## Usage

### Start the Debugger

```bash
# Debug a program compiled with -g
./target/main_interactive ./target/test_breakpoint_line
```

### Set Breakpoints

```bash
# By function name
dbg> b main

# By file and line number
dbg> b test_breakpoint_line.c:27

# List all breakpoints
dbg> breakpoint list
# or
dbg> bl

# Delete a breakpoint
dbg> breakpoint delete 0
```

### Control Execution

```bash
# Continue until breakpoint
dbg> c

# Single step
dbg> s

# Step over (skip function calls)
dbg> n
```

### View Information

```bash
# List functions
dbg> info functions

# List source code
dbg> list ./target/test_breakpoint_line.c 25 10

# Show registers
dbg> register dump
# or
dbg> r

# Show backtrace
dbg> backtrace
# or
dbg> bt
```

## Example Session

```
$ ./target/main_interactive ./target/test_breakpoint_line

dbg> b test_breakpoint_line.c:22
[BREAKPOINT] Line test_breakpoint_line.c:22 resolved to 0x11ed
[BREAKPOINT] Breakpoint #0 set at 0x5555555551ed

dbg> b test_breakpoint_line.c:30
[BREAKPOINT] Line test_breakpoint_line.c:30 resolved to 0x1224
[BREAKPOINT] Breakpoint #1 set at 0x555555555224

dbg> bl
[- BREAKPOINTS]
[BREAKPOINT]   Total: 2 breakpoint(s)

[BREAKPOINT]   [+] # 0  0x00005555555551ed  hits: 0
[BREAKPOINT]   [+] # 1  0x0000555555555224  hits: 0

dbg> c
[BREAKPOINT]
[- BREAKPOINT HIT]
[BREAKPOINT]   Breakpoint #0 hit at 0x00005555555551ed (count: 1)

dbg> c
[BREAKPOINT]
[- BREAKPOINT HIT]
[BREAKPOINT]   Breakpoint #1 hit at 0x0000555555555224 (count: 1)

dbg> quit
```

## Troubleshooting

### "No DWARF info" Error

Make sure your target program is compiled with debug symbols:

```bash
gcc -g program.c -o program
```

### "Cannot find file.c:42 in debug info"

1. Check the file path is correct
2. Use `list` command to verify the file exists in debug info
3. For relative paths, try using the full path

### Breakpoint Not Hitting

1. Use `breakpoint list` to verify breakpoint is enabled `[+]`
2. Check the address is correct with `disas` command
3. Make sure you're continuing execution with `c`

## Implementation Details

The line-based breakpoint feature uses:

1. **DWARF Debug Info** - Parses source line information from ELF binaries
2. **ELF Symbol Table** - Resolves function names to addresses
3. **PIE Support** - Automatically adjusts addresses based on runtime base
4. **INT3 Breakpoints** - Uses hardware breakpoint instruction

For more details, see `LINE_BREAKPOINT_FIX_REPORT.md`
