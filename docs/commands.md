# NDB Command Reference

This document provides a comprehensive reference of all available commands in the NDB debugger.

## Execution Control

### `continue` / `c`
Continue execution until next breakpoint, watchpoint, or signal.

**Syntax:**
```
continue
c
```

**Description:**
Resumes target execution. The program will run until it hits a breakpoint, watchpoint, receives a signal, or exits. If currently stopped at a breakpoint, the debugger will step past it before continuing.

**Examples:**
```
(gdb) continue
(gdb) c
```

### `step` / `s`
Execute a single machine instruction.

**Syntax:**
```
step
s
```

**Description:**
Executes exactly one instruction and returns control to the debugger. If at a breakpoint, the instruction at that location is executed first.

**Examples:**
```
(gdb) step
(gdb) s
```

### `next` / `n`
Step over function calls.

**Syntax:**
```
next
n
```

**Description:**
Similar to `step`, but treats function calls as single instructions. If the current instruction is a CALL, execution continues until the function returns. Otherwise, executes a single instruction.

**Examples:**
```
(gdb) next
(gdb) n
```

## Breakpoint Commands

### `break` / `b`
Set a breakpoint at a specific location.

**Syntax:**
```
break [address]
break [function_name]
b [address]
b [function_name]
```

**Description:**
Sets a software breakpoint at the specified address or function. For PIE (Position Independent Executable) binaries, virtual addresses are automatically adjusted to runtime addresses. If no argument is provided, sets a breakpoint at the current RIP register value.

**Examples:**
```
(gdb) break 0x401000
(gdb) break main
(gdb) b 0x1234
(gdb) b
```

### `breakpoint list` / `bl`
List all breakpoints.

**Syntax:**
```
breakpoint list
bl
```

**Description:**
Displays all currently set breakpoints with their indices, addresses, enabled/disabled status, and hit counts.

**Examples:**
```
(gdb) breakpoint list
(gdb) bl
```

### `breakpoint enable` / `be`
Enable a breakpoint.

**Syntax:**
```
breakpoint enable <index>
be <index>
```

**Description:**
Enables the breakpoint with the specified index. Use `breakpoint list` to see breakpoint indices.

**Examples:**
```
(gdb) breakpoint enable 0
(gdb) be 2
```

### `breakpoint disable` / `bd`
Disable a breakpoint.

**Syntax:**
```
breakpoint disable <index>
bd <index>
```

**Description:**
Temporarily disables the breakpoint with the specified index without deleting it. Use `breakpoint enable` to re-enable it.

**Examples:**
```
(gdb) breakpoint disable 1
(gdb) bd 0
```

### `breakpoint delete`
Delete a breakpoint.

**Syntax:**
```
breakpoint delete <index>
breakpoint del <index>
```

**Description:**
Permanently removes the breakpoint with the specified index.

**Examples:**
```
(gdb) breakpoint delete 0
(gdb) breakpoint del 1
```

## Watchpoint Commands

### `watch` / `w`
Set a watchpoint at an address.

**Syntax:**
```
watch <address>
w <address>
```

**Description:**
Sets a hardware watchpoint at the specified address. The debugger will stop when the watched memory location is written to. Uses hardware debug registers (limited availability).

**Note:** Watchpoints are set as write-only by default for maximum compatibility.

**Examples:**
```
(gdb) watch 0x405000
(gdb) w 0x12345678
```

### `watchpoint list` / `wl`
List all watchpoints.

**Syntax:**
```
watchpoint list
wl
w
```

**Description:**
Displays all currently set watchpoints with their indices, addresses, types, and status.

**Examples:**
```
(gdb) watchpoint list
(gdb) wl
```

### `watchpoint delete`
Delete a watchpoint.

**Syntax:**
```
watchpoint delete <index>
watchpoint del <index>
```

**Description:**
Permanently removes the watchpoint with the specified index.

**Examples:**
```
(gdb) watchpoint delete 0
(gdb) watchpoint del 1
```

## Register Commands

### `register dump` / `r`
Display all register values.

**Syntax:**
```
register dump
r
```

**Description:**
Shows the current values of all general-purpose registers (RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP, RIP, R8-R15).

**Examples:**
```
(gdb) register dump
(gdb) r
```

### `register read`
Read a specific register.

**Syntax:**
```
register read <register_name>
```

**Description:**
Displays the value of a specific register. Supported register names: rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, rip, r8, r9, r10, r11, r12, r13, r14, r15.

**Examples:**
```
(gdb) register read rax
(gdb) register read rip
```

### `register write`
Write a value to a register.

**Syntax:**
```
register write <register_name> <value>
```

**Description:**
Writes a value to a specific register. The value can be specified in decimal or hexadecimal (prefix with 0x).

**Examples:**
```
(gdb) register write rax 0x1234
(gdb) register write rsp 0x7fffffffe000
```

## Memory Commands

### `memory` / `m`
Read memory at an address.

**Syntax:**
```
memory read <address>
memory <address>
m <address>
```

**Description:**
Reads and displays 8 bytes (64 bits) of memory at the specified address. The address can be specified in decimal or hexadecimal format.

**Examples:**
```
(gdb) memory read 0x405000
(gdb) memory 0x12345678
(gdb) m 0x7fffffffe000
```

### `memory write`
Write a value to memory.

**Syntax:**
```
memory write <address> <value>
```

**Description:**
Writes a 32-bit value to the specified memory address. Displays both the new value and the original value that was overwritten.

**Examples:**
```
(gdb) memory write 0x405000 0x90909090
(gdb) memory write 0x12345678 42
```

## Disassembly Commands

### `disas` / `disassemble`
Disassemble code around an address.

**Syntax:**
```
disas [address|function] [--before=N] [--after=N]
disassemble [address|function] [--before=N] [--after=N]
```

**Description:**
Displays assembly code around the specified address or function. If no address is given, disassembles around the current RIP. The `--before` and `--after` options control how many instructions to show before and after the target.

**Options:**
- `--before=N` - Show N instructions before the target (default: 5)
- `--after=N` - Show N instructions after the target (default: 5)

**Examples:**
```
(gdb) disas
(gdb) disas 0x401000
(gdb) disas main
(gdb) disas 0x401000 --before=10 --after=10
```

## Source Code Commands

### `list` / `l`
Display source code.

**Syntax:**
```
list [file] [line] [count]
list [line] [count]
l [file] [line] [count]
```

**Description:**
Shows source code lines. If no file is specified, attempts to determine the source file from the current RIP using addr2line. The current execution line is marked with an arrow (→).

**Arguments:**
- `file` - Source file path (optional)
- `line` - Starting line number (default: 1 or current line)
- `count` - Number of lines to display (default: 10)

**Examples:**
```
(gdb) list
(gdb) list main.c 10 20
(gdb) list 100 5
(gdb) l /path/to/source.c 1
```

## Information Commands

### `backtrace` / `bt`
Display the current call stack.

**Syntax:**
```
backtrace
bt
```

**Description:**
Shows a backtrace of the current call stack, including frame numbers, instruction pointers, and function names with offsets.

**Requirements:**
Requires libunwind-ptrace to be installed (`sudo apt install libunwind-dev`).

**Examples:**
```
(gdb) backtrace
(gdb) bt
```

### `info functions` / `info func` / `info f`
List all functions in the target.

**Syntax:**
```
info functions
info func
info f
info
```

**Description:**
Displays all functions found in the target binary's symbol table, including their virtual addresses.

**Examples:**
```
(gdb) info functions
(gdb) info func
(gdb) info
```

## Configuration Commands

### `set output`
Set output verbosity level.

**Syntax:**
```
set output <level>
```

**Description:**
Controls how much information the debugger displays.

**Levels:**
- `quiet` - Only show signals
- `normal` - Standard output (default)
- `verbose` - Additional details
- `debug` - Maximum diagnostic output

**Examples:**
```
(gdb) set output quiet
(gdb) set output verbose
(gdb) set output debug
```

### `set expand`
Set section expansion level.

**Syntax:**
```
set expand <level>
```

**Description:**
Controls how detailed section headers and separators are displayed.

**Levels:**
- `none` - Compact display, minimal separators
- `normal` - Standard formatting
- `full` - All details and expanded sections

**Examples:**
```
(gdb) set expand none
(gdb) set expand normal
(gdb) set expand full
```

### `filter`
Filter output categories.

**Syntax:**
```
filter <category1,category2,...>
```

**Description:**
Enables only the specified output categories, suppressing all others. Useful for focusing on specific information types.

**Categories:**
- `process` - Process events and status
- `signal` - Signal information
- `backtrace` - Stack traces
- `registers` - Register dumps
- `memory` - Memory operations
- `breakpoint` - Breakpoint/watchpoint events

**Examples:**
```
(gdb) filter signal,backtrace
(gdb) filter registers,memory
(gdb) filter breakpoint
```

## Help and Utility Commands

### `help` / `h`
Display help information.

**Syntax:**
```
help
h
```

**Description:**
Shows a summary of all available commands organized by category.

**Examples:**
```
(gdb) help
(gdb) h
```

### `quit` / `q` / `exit`
Exit the debugger.

**Syntax:**
```
quit
q
exit
```

**Description:**
Terminates the debugging session and detaches from the target process.

**Examples:**
```
(gdb) quit
(gdb) q
(gdb) exit
```

## Command Aliases Summary

| Full Command | Short Alias |
|--------------|-------------|
| continue | c |
| step | s |
| next | n |
| register dump | r |
| memory read | m |
| backtrace | bt |
| break | b |
| breakpoint list | bl |
| breakpoint enable | be |
| breakpoint disable | bd |
| watch | w |
| watchpoint list | wl |
| list | l |
| help | h |
| quit | q |

## Notes

- **Address Format**: Addresses can be specified in hexadecimal (prefix with `0x`) or decimal format
- **PIE Support**: For PIE binaries, virtual addresses are automatically adjusted to runtime addresses
- **DWARF Debugging**: Source-level debugging with DWARF is currently disabled due to library compatibility issues
- **Hardware Watchpoints**: Limited by CPU debug registers (typically 4 watchpoints on x86_64)
- **libunwind**: Required for backtrace functionality. Install with `sudo apt install libunwind-dev`
