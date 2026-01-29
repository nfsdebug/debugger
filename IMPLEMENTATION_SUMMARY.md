# Line-Based Breakpoint Implementation Summary

## Overview
Successfully implemented line-based breakpoint functionality in the NDB debugger by enabling DWARF support and adding file:line syntax parsing.

## Key Changes

### 1. src/main_interactive.c
**Before:**
```c
/* DWARF disabled due to libdwarf abort issue */
/* #include "core/dwarf.h" */
/* static dwarf_state_t g_dwarf; */
```

**After:**
```c
/* DWARF support for line-based breakpoints */
#include "core/dwarf.h"
static dwarf_state_t g_dwarf;
```

**New Implementation:**
- `cmd_breakpoint_set_func()` now handles:
  - Function names: `break main`
  - File:line: `break main.c:42`
  - Function:line: `break main:25`
- Uses DWARF to resolve source locations to addresses
- Properly handles PIE binaries with base address adjustment

### 2. src/cli/parser.c
**Before:**
```c
cmd->string_arg = strdup(subcmd);  // After tolower_str()
```

**After:**
```c
/* Function name or file:line spec - don't lowercase, preserve case for filenames */
cmd->type = CMD_BREAKPOINT_FUNC;
cmd->string_arg = strdup(subcmd);  /* Use original case for filenames */
```

### 3. makefile
**Before:**
```makefile
$(TARGET)/main_interactive : ... $(SRC)/core/breakpoints.c ... -o $@ -lunwind ...
```

**After:**
```makefile
$(TARGET)/main_interactive : ... $(SRC)/core/dwarf.c ... -o $@ -ldwarf -lunwind ...
```

## Test Results

All tests pass:
```
✅ DWARF loading (21 source lines)
✅ Function breakpoints (b function_name)
✅ File:line breakpoints (b file.c:42)
✅ Multiple breakpoints
✅ Breakpoint hitting
✅ Symbol resolution (20 symbols)
✅ Source listing
```

## Usage

```bash
# Build
make clean && make

# Run
./target/main_interactive ./target/test_breakpoint_line

# Set breakpoints
dbg> b main                          # Function name
dbg> b test_breakpoint_line.c:27     # File:line
dbg> breakpoint list                 # List all
dbg> c                               # Continue
```

## Files Modified
- `/home/sbstndbs/debugger/src/main_interactive.c`
- `/home/sbstndbs/debugger/src/cli/parser.c`
- `/home/sbstndbs/debugger/makefile`

## Documentation
- `LINE_BREAKPOINT_FIX_REPORT.md` - Detailed technical report
- `LINE_BREAKPOINT_QUICKSTART.md` - Quick start guide
- `test_line_breakpoint_comprehensive.sh` - Test suite
