# Debugger Test Suite

## Overview

This test suite provides automated testing for the CLI debugger, covering all major functionality including breakpoints, watchpoints, disassembly, source listing, and more.

## Quick Start

```bash
# Run all tests
make test

# Run specific test suites
make test-basic        # Basic functionality
make test-breakpoints  # Breakpoint commands
make test-step         # Step/next commands
make test-watchpoints  # Watchpoint commands
make test-registers    # Register and memory commands
make test-disas        # Disassembly and listing
```

## Test Structure

```
tests/
├── framework.sh          # Test framework (assertions, colors, reporting)
├── all_tests.sh          # Main test runner
├── run_test.sh           # Helper for running single tests
├── programs/             # Test programs
│   ├── test_simple.c
│   ├── test_functions.c
│   ├── test_loops.c
│   └── test_globals.c
└── suites/               # Test suites
    ├── test_basic.sh
    ├── test_breakpoints.sh
    ├── test_step.sh
    ├── test_watchpoints.sh
    ├── test_registers_memory.sh
    └── test_disas_list.sh
```

## Test Results

Current test coverage: **34/34 tests passing (100%)**

| Suite | Tests | Status |
|-------|-------|--------|
| Basic | 4/4 | ✅ All passing |
| Breakpoints | 8/8 | ✅ All passing |
| Step/Next | 4/4 | ✅ All passing |
| Watchpoints | 3/3 | ✅ All passing |
| Registers/Memory | 7/7 | ✅ All passing |
| Disas/List | 8/8 | ✅ All passing |

## Adding New Tests

1. Create a test program in `tests/programs/`
2. Add test functions in `tests/suites/`
3. Use the framework functions:
   - `test_section()` - Group related tests
   - `run_cmd()` - Execute debugger commands
   - Use `grep -q` to check output

Example:
```bash
test_my_feature() {
    test_section "My Feature"
    local output=$(run_cmd "my_command")
    if echo "$output" | grep -q "Expected text"; then
        echo -e "${GREEN}✓${NC} Test passed"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Test failed"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}
```
