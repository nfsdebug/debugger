# NDB Debugger

## Overview

NDB is a CLI debugger for Linux x86-64 using `ptrace`, `libdwarf`, and `libunwind`.

## Quick Start

```bash
# Build
make

# Run
./target/main_interactive ./your_program
```

## Features

- **Breakpoints**: Set, list, enable, disable, delete breakpoints at functions or addresses
- **Watchpoints**: Monitor memory addresses for changes
- **Stepping**: Step into/over instructions, continue execution
- **Disassembly**: Disassemble functions with configurable context
- **Registers**: Dump and read CPU registers
- **Memory**: Read memory at addresses
- **Backtrace**: View call stack
- **Symbols**: List functions, resolve addresses to symbols

## Building

**Dependencies:**
```bash
# Ubuntu/Debian
sudo apt install libdwarf-dev libunwind-dev libunwind-ptrace-dev libreadline-dev
```

**Compile:**
```bash
make
```

## Testing

```bash
make test          # Run all tests (34 tests)
make test-basic
make test-breakpoints
make test-step
make test-watchpoints
make test-registers
make test-disas
```

## Documentation

- [Test Suite](tests/README.md) - Automated testing overview
- [Command Reference](docs/commands.md) - All debugger commands *(coming soon)*
- [Architecture](docs/architecture.md) - Project structure *(coming soon)*

## Project Structure

```
src/
├── main_interactive.c   # Main CLI entry point
├── core/                 # Core debugging functionality
│   ├── breakpoints.c     # Breakpoint management
│   ├── disasm.c          # Disassembly engine
│   ├── symbols.c         # Symbol resolution
│   └── watchpoints.c     # Watchpoint support
├── cli/                  # Command-line interface
│   ├── parser.c          # Command parsing
│   └── readline.c        # Readline integration
└── display/              # Output formatting
    ├── output.c
    └── theme.c
```

## License

See LICENSE file.
