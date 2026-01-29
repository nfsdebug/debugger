# NDB Debugger Architecture

## Project Overview

NDB is a command-line debugger for Linux x86-64 platforms. It provides interactive debugging capabilities using system-level tracing facilities (`ptrace`), debug information parsing (`libdwarf`), and stack unwinding (`libunwind-ptrace`).

The debugger follows a modular architecture with clear separation between:
- Core debugging functionality (process control, breakpoints, watchpoints)
- Command-line interface (parsing, REPL, readline)
- Display/output subsystem (theming, formatting, sections)

## Directory Structure

```
src/
├── main_interactive.c      # Main entry point, interactive loop
├── debug_console.c         # Alternative console entry point
├── utilities.c/h           # Shared utility functions
│
├── core/                   # Core debugging modules
│   ├── debugger.c/h        # Process control via ptrace, signal handling
│   ├── breakpoints.c/h     # Software breakpoints (INT3)
│   ├── watchpoints.c/h     # Hardware watchpoints (DR0-DR3)
│   ├── symbols.c/h         # ELF symbol resolution
│   ├── disasm.c/h          # Disassembly engine
│   ├── dwarf.c/h           # DWARF debug info parsing
│   ├── registers.c/h       # Register read/write operations
│   └── memory.c/h          # Memory read/write operations
│
├── cli/                    # Command-line interface
│   ├── parser.c/h          # Command parsing (argtable3)
│   ├── repl.c/h            # Interactive mode (linenoise)
│   ├── readline.c/h        # Readline wrapper
│   └── config.c/h          # Configuration management
│
└── display/                # Output and display subsystem
    ├── output.c/h          # Output levels (quiet/normal/verbose/debug)
    ├── theme.c/h           # Color/ANSI code management
    └── sections.c/h        # Modular output sections
```

## Key Components

### Core Modules (`src/core/`)

#### debugger.c/h
The low-level process control layer using `ptrace` system calls.

**Key structures:**
```c
typedef struct {
    pid_t pid, ppid;
    gid_t gid;
    char path[512];
    uint64_t offset, base;
} process_info_t;

typedef struct {
    int signo, err_no, code;
    void *addr;
    const char *name;
} signal_info_t;

typedef struct {
    process_info_t proc;
    signal_info_t signal;
    int running, exited, exit_code;
} debugger_state_t;
```

**Responsibilities:**
- Process spawning with `PTRACE_TRACEME`
- Base address detection from `/proc/PID/maps`
- Process control: continue, single-step, wait for signals
- Signal information extraction

#### breakpoints.c/h
Software breakpoint management using INT3 (0xCC) instruction.

**Key structures:**
```c
typedef enum {
    BP_FUNCTION, BP_ADDRESS, BP_CONDITIONAL
} bp_type_t;

typedef struct {
    bp_type_t type;
    union {
        char func_name[128];
        void *address;
    } location;
    uint64_t actual_address;
    uint8_t original_byte;
    int enabled;
    int hit_count;
} breakpoint_t;

typedef struct {
    breakpoint_t *bps;
    int count, capacity;
} breakpoint_state_t;
```

**Responsibilities:**
- Set breakpoints by address or function name
- Save/restore original instruction bytes
- Enable/disable/delete breakpoints
- Step-over logic (restore instruction, single-step, re-set INT3)
- Hit detection

#### watchpoints.c/h
Hardware watchpoints using x86-64 debug registers (DR0-DR3, DR6, DR7).

**Key structures:**
```c
typedef enum {
    WP_WRITE, WP_READ, WP_READ_WRITE
} wp_type_t;

typedef struct {
    uint64_t address;
    wp_type_t type;
    int size;        // 1, 2, 4, or 8 bytes
    int enabled;
    int reg_index;   // DR0-DR3
} watchpoint_t;

typedef struct {
    watchpoint_t watchpoints[4];
    int count;
    pid_t pid;
} wp_state_t;
```

**Responsibilities:**
- Manage x86-64 debug registers
- Set read/write/read-write watchpoints
- Configure DR7 (debug control register)
- Check DR6 (debug status register) for hits

#### symbols.c/h
ELF symbol table parsing for function resolution.

**Key structures:**
```c
typedef struct {
    char name[256];
    uint64_t address, size;
    int type;  // 0=unknown, 1=function, 2=object
} symbol_t;

typedef struct {
    symbol_t *symbols;
    int count;
    char *program_path;
} symbol_table_t;
```

**Responsibilities:**
- Parse ELF symbol table from target binary
- Resolve function names to addresses
- Reverse lookup (address to symbol name)
- List available functions

#### disasm.c/h
Disassembly engine using platform-specific instructions.

**Responsibilities:**
- Disassemble around current RIP
- Disassemble around specific address
- Disassemble around function (via symbol resolution)

#### dwarf.c/h
DWARF debug information parsing (currently disabled due to libdwarf stability issues).

**Key structures:**
```c
typedef struct {
    char file[256];
    int line;
    uint64_t address;
} source_location_t;

typedef struct {
    char name[256];
    uint64_t low_pc, high_pc;
} function_info_t;
```

**Responsibilities:**
- Parse DWARF debug sections
- Map addresses to source locations (file:line)
- Map source locations to addresses
- Source code listing

#### registers.c/h
CPU register access via ptrace.

**Key structures:**
```c
typedef enum {
    REG_RAX, REG_RBX, ..., REG_RIP, REG_RFLAGS, ...
} register_id_t;

typedef struct {
    char name[8];
    uint64_t value;
    int valid;
} register_value_t;
```

**Responsibilities:**
- Read/write individual registers
- Dump all registers
- Register name resolution

#### memory.c/h
Memory access operations via ptrace.

**Key structures:**
```c
typedef struct {
    void *start, *end;
    char permissions[5];  // rwxp
    char path[256];
} memory_region_t;
```

**Responsibilities:**
- Read/write memory words
- Read/write byte arrays
- Parse `/proc/PID/maps` for memory regions

### CLI Modules (`src/cli/`)

#### parser.c/h
Command parsing using argtable3.

**Key structures:**
```c
typedef enum {
    CMD_CONTINUE, CMD_SINGLE_STEP, CMD_STEP_OVER,
    CMD_REGISTER_DUMP, CMD_REGISTER_READ, CMD_REGISTER_WRITE,
    CMD_BREAKPOINT_FUNC, CMD_BREAKPOINT_ADDR, CMD_BREAKPOINT_CURRENT,
    CMD_WATCHPOINT_ADDR, CMD_WATCHPOINT_LIST, CMD_WATCHPOINT_DELETE,
    CMD_MEMORY_READ, CMD_MEMORY_WRITE, CMD_BACKTRACE,
    CMD_LIST_SOURCE, CMD_INFO_FUNCTIONS, CMD_DISASM,
    CMD_SET_OUTPUT, CMD_SET_EXPAND, CMD_HELP, CMD_QUIT
} command_type_t;

typedef struct {
    command_type_t type;
    char *string_arg;
    uint64_t addr_arg, value_arg;
    int int_arg;
    char **list_arg;
    int list_count;
} command_t;
```

**Responsibilities:**
- Parse command strings into structured commands
- Validate command arguments
- Provide help/usage information

#### repl.c/h
Interactive REPL loop using linenoise.

**Key structures:**
```c
typedef struct {
    const char *prompt;
    int history_max_len;
    const char *history_path;
} repl_config_t;
```

**Responsibilities:**
- Read-eval-print loop
- Command completion (linenoise)
- Hints display (optional inline help)
- History management

#### readline.c/h
Wrapper around readline for line input.

**Responsibilities:**
- Initialize readline/history
- Read input lines
- Add to history
- Cleanup

#### config.c/h
Configuration management (libconfig integration).

**Key structures:**
```c
typedef struct {
    output_config_t output;
    theme_config_t theme;
    expand_level_t default_expand;
    char prompt[32];
    int history_size;
    char history_path[512];
    int auto_log;
    char log_path[512];
    int summary_interval;
    int use_colors, show_timestamps;
} debugger_config_t;
```

**Responsibilities:**
- Load/save configuration files
- Manage user preferences
- Apply settings to display subsystems

### Display Modules (`src/display/`)

#### output.c/h
Output level management and routing.

**Key structures:**
```c
typedef enum {
    OUTPUT_QUIET, OUTPUT_NORMAL, OUTPUT_VERBOSE, OUTPUT_DEBUG
} output_level_t;

typedef enum {
    CAT_PROCESS, CAT_SIGNAL, CAT_BACKTRACE,
    CAT_REGISTERS, CAT_MEMORY, CAT_BREAKPOINT
} output_category_t;

typedef struct {
    unsigned long signals_received, breakpoints_hit;
    unsigned long steps_executed, memory_reads, memory_writes;
    unsigned long errors;
    time_t start_time, last_update;
} output_stats_t;
```

**Responsibilities:**
- Filter output by level (quiet/normal/verbose/debug)
- Filter by category (process, signal, backtrace, etc.)
- Statistics tracking
- Log file management with rotation

#### theme.c/h
Terminal capability detection and ANSI color codes.

**Key structures:**
```c
typedef enum {
    COLOR_RESET, COLOR_BLACK, COLOR_RED, ..., COLOR_BRIGHT_WHITE
} color_t;

typedef enum {
    STYLE_BOLD, STYLE_DIM, STYLE_UNDERLINE, ...
} style_t;

typedef enum {
    COLOR_NEVER, COLOR_AUTO, COLOR_ALWAYS
} color_mode_t;
```

**Responsibilities:**
- Detect terminal capabilities (256-color, truecolor)
- Generate ANSI escape sequences
- NO_COLOR environment variable support
- Box drawing characters for UI elements

#### sections.c/h
Modular output sections with expand/collapse.

**Key structures:**
```c
typedef enum {
    EXPAND_NONE, EXPAND_NORMAL, EXPAND_FULL
} expand_level_t;

typedef enum {
    SECTION_BACKTRACE, SECTION_REGISTERS,
    SECTION_MEMORY, SECTION_PROCESS, SECTION_SIGNAL
} section_type_t;
```

**Responsibilities:**
- Section header rendering
- Expand/collapse state management
- Section visibility control
- Formatted output for backtrace, registers, memory

## Data Flow

### Command Parsing and Execution

```
User Input
    |
    v
[readline/linenoise]  (rl_readline)
    |
    v
[parser]              (parser_parse_command)
    |
    v
command_t structure   (type, args)
    |
    v
Command Switch        (run_interactive)
    |
    +-- CMD_CONTINUE -----> [debugger.c] -----> ptrace(PTRACE_CONT)
    +-- CMD_SINGLE_STEP --> [debugger.c] -----> ptrace(PTRACE_SINGLESTEP)
    +-- CMD_BREAKPOINT_ --> [breakpoints.c] --> INT3 injection
    +-- CMD_WATCHPOINT_ --> [watchpoints.c] --> DR0-DR3 configuration
    +-- CMD_MEMORY_ -----> [memory.c] ------> ptrace(PEEKDATA/POKEDATA)
    +-- CMD_REGISTER_ ---> [registers.c] ---> ptrace(GETREGS/SETREGS)
    +-- CMD_BACKTRACE ---> [libunwind] -----> unw_step()
    +-- CMD_DISASM -----> [disasm.c] ------> instruction decode
    +-- CMD_INFO_ ------> [symbols.c] -----> ELF lookup
    |
    v
[output.c]            (output_print)
    |
    v
[theme.c]             (ANSI formatting)
    |
    v
Terminal Display
```

### Process Lifecycle

1. **Initialization**
   - `main()` spawns target with `fork()` + `ptrace(PTRACE_TRACEME)`
   - Target calls `execvp()` to run program
   - Debugger waits for initial `SIGSTOP` from execve
   - Extract base address from `/proc/PID/maps`
   - Initialize libunwind address space
   - Load symbols from ELF file
   - Initialize breakpoint/watchpoint state

2. **Interactive Loop**
   - Display prompt ("dbg> ")
   - Read command via linenoise/readline
   - Parse command
   - Execute command (may involve ptrace calls)
   - Format and display result
   - Repeat until quit

3. **Execution Control**
   - `continue`: `ptrace(PTRACE_CONT)` -> wait for signal
   - `step`: `ptrace(PTRACE_SINGLESTEP)` -> wait for signal
   - On `SIGTRAP`: check for breakpoint/watchpoint hit
   - On crash signal (`SIGSEGV`, `SIGILL`): show backtrace
   - On exit: display exit code

### Breakpoint Handling

**Setting a breakpoint:**
1. Read current byte at target address via `PTRACE_PEEKTEXT`
2. Store original byte in breakpoint state
3. Write `0xCC` (INT3) via `PTRACE_POKETEXT`

**Hitting a breakpoint:**
1. Target executes INT3, triggers `SIGTRAP`
2. Debugger reads RIP (points after INT3)
3. Check if RIP-1 matches any breakpoint address
4. If match: "step past" logic

**Stepping past a breakpoint:**
1. Restore original instruction byte via `PTRACE_POKETEXT`
2. Adjust RIP backward by 1 (`regs.rip -= 1`)
3. Single-step via `PTRACE_SINGLESTEP`
4. Re-write INT3 via `PTRACE_POKETEXT`

### Watchpoint Handling (x86-64)

**Setting a watchpoint:**
1. Find available debug register (DR0-DR3)
2. Configure DR7 with address, type (read/write), size
3. On subsequent stops, check DR6 for watchpoint hits

## Integration with External Libraries

### ptrace (Linux Kernel)
**Purpose:** System-level tracing and process control

**Key calls used:**
- `PTRACE_TRACEME` - Child allows tracing
- `PTRACE_SEIZE` - Attach to running process (future)
- `PTRACE_CONT` - Continue execution
- `PTRACE_SINGLESTEP` - Execute one instruction
- `PTRACE_GETREGS` / `PTRACE_SETREGS` - Register access
- `PTRACE_PEEKTEXT` / `PTRACE_POKETEXT` - Memory access
- `PTRACE_PEEKDATA` / `PTRACE_POKEDATA` - Memory access

### libdwarf
**Purpose:** Parse DWARF debug information for source-level debugging

**Usage:**
- Map addresses to source file:line
- Map source file:line to addresses
- Extract function boundaries, local variables

**Note:** Currently disabled due to stability issues (libdwarf abort() on certain binaries)

### libunwind-ptrace
**Purpose:** Platform-independent stack unwinding

**Key functions:**
- `unw_create_addr_space()` - Create address space for ptrace
- `_UPT_create()` - Initialize ptrace unwinding
- `unw_init_remote()` - Initialize cursor for target process
- `unw_get_reg()` - Read register (IP, SP)
- `unw_get_proc_name()` - Get function name
- `unw_step()` - Move to previous stack frame

### readline / linenoise
**Purpose:** Interactive command line input

**Features:**
- Command history
- Line editing
- Auto-completion
- Hints (linenoise-ng)

### libconfig
**Purpose:** Configuration file management

**Usage:**
- Load user preferences from `~/.ndbrc`
- Save configuration changes
- Default settings management

### argtable3
**Purpose:** Command-line argument parsing

**Usage:**
- Parse debugger commands with arguments
- Generate usage/help text
- Type validation

## Address Translation

The debugger handles both PIE (Position-Independent Executable) and non-PIE binaries:

```
Virtual Address (from ELF/symbols) + Base Address (from /proc/PID/maps) = Runtime Address
```

- **Virtual Address:** Address as stored in ELF file (usually starting at 0x400000 for non-PIE, 0 for PIE)
- **Base Address:** Runtime load base (extracted from r-x segment in /proc/PID/maps)
- **Runtime Address:** Actual address in target process memory space

Example for PIE binary:
```
Function main: vaddr = 0x1234
Base address: 0x555555554000
Runtime address: 0x555555555234
```

## Threading

Currently, the debugger is single-threaded. The debugger process runs the interactive loop, while the traced process runs in a parent-child relationship via fork/ptrace.

Future multi-threading support would require:
- `PTRACE_SEIZE` + `PTRACE_LISTEN` for thread stop/start
- Per-thread register/breakpoint management
- Thread-aware backtraces

## Error Handling

Error handling is distributed across modules:
- **ptrace errors:** Checked via `errno`, reported via `output_error()`
- **Memory access:** `PTRACE_PEEKDATA` returns -1 and sets `errno` on fault
- **Symbol resolution:** Returns 0 or NULL on failure
- **Breakpoint limits:** Maximum 32 breakpoints (configurable)
- **Watchpoint limits:** 4 hardware watchpoints (x86-64 DR0-DR3)

## Build System

The project uses CMake with optional features:
- `BUILD_INTERACTIVE` - linenoise support (default: ON)
- `BUILD_CONFIG` - libconfig support (default: ON)
- `BUILD_TESTS` - Build test suite (default: OFF)

Compiler flags: `-Wall -Wextra -Wpedantic -g -gdwarf-2`

## Testing

Test suite located in `tests/` directory:
- Basic functionality tests
- Breakpoint set/hit/remove
- Single-step execution
- Watchpoint behavior
- Register operations
- Disassembly output

Tests run as shell scripts that launch the debugger, send commands, and verify output.
