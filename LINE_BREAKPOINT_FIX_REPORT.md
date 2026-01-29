# Line-Based Breakpoint Fix Report

## Summary

Successfully fixed the symbol resolution and line-based breakpoint functionality in the NDB debugger. The debugger now supports:

- ✅ `break <function_name>` - Set breakpoint at function entry
- ✅ `break <filename>:<line>` - Set breakpoint at specific source line
- ✅ DWARF debug information parsing
- ✅ ELF symbol table loading
- ✅ Proper address resolution for PIE (Position Independent Executable) binaries

## What Was Broken

### 1. DWARF Support Disabled
**File:** `/home/sbstndbs/debugger/src/main_interactive.c`

**Issue:** DWARF support was commented out due to "libdwarf abort issue". This prevented:
- Loading source line information
- Resolving `file:line` to runtime addresses
- Source-level debugging

**Evidence:**
```c
/* DWARF disabled due to libdwarf abort issue */
/* #include "core/dwarf.h" */
...
/* DWARF disabled due to libdwarf abort issue */
/* static dwarf_state_t g_dwarf; */
```

### 2. No File:Line Syntax Support
**File:** `/home/sbstndbs/debugger/src/main_interactive.c`

**Issue:** The `cmd_breakpoint_set_func()` function explicitly rejected file:line syntax:

```c
if (colon) {
    output_error("Line number breakpoints (func:line) require DWARF support (currently disabled)");
    return -1;
}
```

### 3. Parser Case Conversion
**File:** `/home/sbstndbs/debugger/src/cli/parser.c`

**Issue:** The parser was converting breakpoint specifications to lowercase, breaking case-sensitive Linux filenames.

### 4. Missing DWARF Linking
**File:** `/home/sbstndbs/debugger/makefile`

**Issue:** The `main_interactive` target was not linking `libdwarf` or compiling `dwarf.c`.

## Fixes Implemented

### 1. Enabled DWARF Support

**File:** `/home/sbstndbs/debugger/src/main_interactive.c`

Re-enabled DWARF by uncommenting the necessary includes and variables:
```c
/* DWARF support for line-based breakpoints */
#include "core/dwarf.h"

static dwarf_state_t g_dwarf;
```

Also enabled DWARF loading in main():
```c
/* Initialize DWARF from program */
if (dwarf_load(&g_dwarf, argv[1]) < 0) {
    output_error("No DWARF info - compile with -g for source-level debugging\n");
}
```

### 2. Implemented File:Line Breakpoint Support

**File:** `/home/sbstndbs/debugger/src/main_interactive.c`

Completely rewrote `cmd_breakpoint_set_func()` to handle:
- Plain function names: `break function_name`
- File:line syntax: `break main.c:42`
- Function:line syntax: `break main:25`

**Key logic:**
1. Check for colon (`:`) in specification
2. If found, split into `before` and `after` parts
3. Check if `before` is a function name (via symbol table)
4. If it's a function:line, find the function's source file via DWARF, then resolve the line
5. If it's a file:line, resolve directly via DWARF
6. If no colon, treat as plain function name

### 3. Fixed Parser Case Sensitivity

**File:** `/home/sbstndbs/debugger/src/cli/parser.c`

Modified the breakpoint parsing to preserve case for filenames:
```c
} else {
    /* Function name or file:line spec - don't lowercase, preserve case for filenames */
    cmd->type = CMD_BREAKPOINT_FUNC;
    cmd->string_arg = strdup(subcmd);  /* Use original case for filenames */
    free(subcmd);
    free(copy);
    return 0;
}
```

### 4. Updated Build System

**File:** `/home/sbstndbs/debugger/makefile`

Added DWARF support to the `main_interactive` target:
```makefile
$(TARGET)/main_interactive : $(SRC)/main_interactive.c ... $(SRC)/core/dwarf.c ...
    gcc ... -o $@ -ldwarf -lunwind -lunwind-ptrace -lunwind-generic -lreadline
```

## Test Results

Created comprehensive test suite at `/home/sbstndbs/debugger/test_line_breakpoint_comprehensive.sh`

### Test 1: DWARF Loading
```
✅ PASS: DWARF loaded with 21 source lines
```

### Test 2: Function Name Breakpoint
```
✅ PASS: Function 'function_a' resolved to 0x555555555169
```

### Test 3: File:Line Breakpoint
```
✅ PASS: test_breakpoint_line.c:27 resolved to 0x120a
```

### Test 4: Multiple Breakpoints
```
✅ PASS: Set 2 breakpoints successfully
```

### Test 5: Hitting Breakpoints
```
✅ PASS: Breakpoint hit at 0x0000555555555224 (count: 1)
```

### Test 6: Symbol Resolution
```
✅ PASS: Loaded 20 symbols including function_a, function_b, main
```

### Test 7: Source Code Listing
```
✅ PASS: Source listing works
```

## Usage Examples

### Set breakpoint at function
```
dbg> b function_a
[BREAKPOINT] Function 'function_a' resolved to 0x555555555169
[BREAKPOINT] Breakpoint #0 set at 0x555555555169
```

### Set breakpoint at file:line
```
dbg> b test_breakpoint_line.c:27
[BREAKPOINT] Line test_breakpoint_line.c:27 resolved to 0x55555555520a
[BREAKPOINT] Breakpoint #0 set at 0x55555555520a
```

### List breakpoints
```
dbg> breakpoint list
[- BREAKPOINTS]
[BREAKPOINT]   Total: 1 breakpoint(s)

[BREAKPOINT]   [+] # 0  0x000055555555520a  hits: 0
```

### Continue to breakpoint
```
dbg> c

[BREAKPOINT]
[- BREAKPOINT HIT]
[BREAKPOINT]   Breakpoint #0 hit at 0x000055555555520a (count: 1)
```

## Files Modified

1. `/home/sbstndbs/debugger/src/main_interactive.c` - Enabled DWARF, implemented file:line support
2. `/home/sbstndbs/debugger/src/cli/parser.c` - Fixed case sensitivity for filenames
3. `/home/sbstndbs/debugger/makefile` - Added DWARF linking

## Files Created

1. `/home/sbstndbs/debugger/target/test_breakpoint_line.c` - Test program
2. `/home/sbstndbs/debugger/test_line_breakpoint_comprehensive.sh` - Test suite
3. `/home/sbstndbs/debugger/LINE_BREAKPOINT_FIX_REPORT.md` - This report

## Architecture

The fix leverages the existing DWARF infrastructure:

1. **ELF Symbol Table** (`src/core/symbols.c`)
   - Loads function symbols from .symtab/.dynsym
   - Provides `symbols_find_address()` for function name resolution

2. **DWARF Parser** (`src/core/dwarf.c`)
   - Uses libdwarf to parse debug information
   - Provides `dwarf_line_to_addr()` for file:line → address resolution
   - Provides `dwarf_addr_to_line()` for address → file:line reverse lookup

3. **Breakpoint Management** (`src/core/breakpoints.c`)
   - Manages INT3 breakpoints
   - Handles enable/disable/delete operations
   - Tracks hit counts

## Requirements

- libdwarf-dev: `sudo apt install libdwarf-dev`
- Compile target with `-g` flag for DWARF debug info

## Conclusion

The line-based breakpoint functionality is now fully operational. Users can set breakpoints using:
- Function names: `break main`
- File and line: `break main.c:42`

The implementation properly handles:
- PIE binaries (address offset adjustment)
- Case-sensitive filenames
- Multiple breakpoints
- Breakpoint hitting and reporting
- Source code listing

All test cases pass successfully.
