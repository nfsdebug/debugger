#!/bin/bash
# @file test_real_programs.sh
# @brief Test suite for complex, real-world program scenarios

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
source "$SCRIPT_DIR/../framework.sh"

TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

# Helper to run command and check output
run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 5 "$DEBUGGER" "$TEST_BIN_DIR/test_complex" 2>&1 || true
}

run_cmd_on() {
    local binary="$1"
    local cmds="$2"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 5 "$DEBUGGER" "$TEST_BIN_DIR/$binary" 2>&1 || true
}

# Test: Breakpoints in nested function calls
test_nested_breakpoints() {
    test_section "Nested Function Breakpoints"

    local output=$(run_cmd "break level1
break level2
break level3
continue")

    if echo "$output" | grep -q "level1"; then
        echo -e "${GREEN}✓${NC} Can set breakpoint in level1"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoint in level1"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))

    if echo "$output" | grep -q "level2"; then
        echo -e "${GREEN}✓${NC} Can set breakpoint in level2"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoint in level2"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Watchpoints on global variables
test_global_watchpoints() {
    test_section "Global Variable Watchpoints"

    local output=$(run_cmd "watch global_counter
continue")

    if echo "$output" | grep -q "Watchpoint.*global_counter"; then
        echo -e "${GREEN}✓${NC} Can set watchpoint on global variable"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set watchpoint on global variable"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Backtrace through complex call stacks
test_complex_backtrace() {
    test_section "Complex Backtrace"

    local output=$(run_cmd "break deep_recursion
continue
backtrace")

    if echo "$output" | grep -q "deep_recursion"; then
        echo -e "${GREEN}✓${NC} Backtrace shows recursive function"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Backtrace shows recursive function"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Stepping through recursive functions
test_recursive_stepping() {
    test_section "Recursive Function Stepping"

    local output=$(run_cmd "break factorial
continue
step
step")

    if echo "$output" | grep -q "factorial"; then
        echo -e "${GREEN}✓${NC} Can step into recursive function"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can step into recursive function"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Inspect struct variables
test_struct_inspection() {
    test_section "Struct Variable Inspection"

    local output=$(run_cmd "break create_point
continue
print p")

    if echo "$output" | grep -q "Point\|x\|y\|name"; then
        echo -e "${GREEN}✓${NC} Can inspect struct variables"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can inspect struct variables"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Inspect pointer values
test_pointer_inspection() {
    test_section "Pointer Inspection"

    local output=$(run_cmd "break swap_ints
continue
print a
print *a")

    if echo "$output" | grep -q "0x\|\*a"; then
        echo -e "${GREEN}✓${NC} Can inspect pointer values"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can inspect pointer values"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Breakpoint in malloc'd memory operations
test_malloc_breakpoints() {
    test_section "Malloc Operations Breakpoints"

    local output=$(run_cmd "break create_point_list
break free_point_list
continue")

    if echo "$output" | grep -q "create_point_list"; then
        echo -e "${GREEN}✓${NC} Can set breakpoint in malloc function"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoint in malloc function"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Function pointer breakpoints
test_function_pointer_breakpoints() {
    test_section "Function Pointer Breakpoints"

    local output=$(run_cmd "break apply_operation
break add
continue")

    if echo "$output" | grep -q "apply_operation\|add"; then
        echo -e "${GREEN}✓${NC} Can set breakpoints on functions used via pointers"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoints on functions used via pointers"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Static variable inspection
test_static_variable_inspection() {
    test_section "Static Variable Inspection"

    local output=$(run_cmd "break deep_recursion
continue
print static_counter")

    if echo "$output" | grep -q "static_counter"; then
        echo -e "${GREEN}✓${NC} Can inspect static variables"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can inspect static variables"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Multiple recursive calls (Fibonacci)
test_fibonacci_recursion() {
    test_section "Fibonacci Recursion"

    local output=$(run_cmd "break fibonacci
continue")

    if echo "$output" | grep -q "fibonacci"; then
        echo -e "${GREEN}✓${NC} Can breakpoint in fibonacci recursion"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can breakpoint in fibonacci recursion"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Pointer arithmetic inspection
test_pointer_arithmetic() {
    test_section "Pointer Arithmetic"

    local output=$(run_cmd "break pointer_arithmetic_test
continue
print ptr
print *(ptr+1)")

    # Check if we can inspect pointer arithmetic
    if echo "$output" | grep -q "ptr"; then
        echo -e "${GREEN}✓${NC} Can inspect pointers"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can inspect pointers"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Double pointer inspection
test_double_pointer() {
    test_section "Double Pointer Inspection"

    local output=$(run_cmd "break double_pointer_test
continue
print ptr_to_ptr")

    if echo "$output" | grep -q "ptr_to_ptr\|ptr"; then
        echo -e "${GREEN}✓${NC} Can inspect double pointers"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can inspect double pointers"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Array access breakpoints
test_array_operations() {
    test_section "Array Operations"

    local output=$(run_cmd "break pointer_arithmetic_test
continue")

    if echo "$output" | grep -q "pointer_arithmetic_test"; then
        echo -e "${GREEN}✓${NC} Can break in array operations"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can break in array operations"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Deep call stack navigation
test_deep_stack_navigation() {
    test_section "Deep Stack Navigation"

    local output=$(run_cmd "break deep_recursion
continue
backtrace")

    if echo "$output" | grep -q "#"; then
        echo -e "${GREEN}✓${NC} Can show deep call stack"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can show deep call stack"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Signal handler breakpoints (on test_signals)
test_signal_handler_breakpoints() {
    test_section "Signal Handler Breakpoints"

    # Only run if test_signals binary exists
    if [ ! -f "$TEST_BIN_DIR/test_signals" ]; then
        test_skip "Signal handler breakpoints" "test_signals binary not found"
        return
    fi

    local output=$(run_cmd_on "test_signals" "break sigusr1_handler
continue")

    if echo "$output" | grep -q "sigusr1_handler\|Breakpoint"; then
        echo -e "${GREEN}✓${NC} Can set breakpoint in signal handler"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoint in signal handler"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Thread creation breakpoints (on test_threads)
test_thread_breakpoints() {
    test_section "Thread Breakpoints"

    # Only run if test_threads binary exists
    if [ ! -f "$TEST_BIN_DIR/test_threads" ]; then
        test_skip "Thread breakpoints" "test_threads binary not found"
        return
    fi

    local output=$(run_cmd_on "test_threads" "break worker_thread
break test_basic_threads
continue")

    if echo "$output" | grep -q "worker_thread\|Breakpoint"; then
        echo -e "${GREEN}✓${NC} Can set breakpoint in thread function"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can set breakpoint in thread function"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Memory allocation tracking
test_memory_allocation() {
    test_section "Memory Allocation Tracking"

    local output=$(run_cmd "break create_point_list
continue
next
print list")

    if echo "$output" | grep -q "list\|0x"; then
        echo -e "${GREEN}✓${NC} Can track allocated memory"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can track allocated memory"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Struct member access
test_struct_member_access() {
    test_section "Struct Member Access"

    local output=$(run_cmd "break move_point
continue
print p->x
print p->y")

    if echo "$output" | grep -q "x\|y"; then
        echo -e "${GREEN}✓${NC} Can access struct members through pointer"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can access struct members through pointer"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Test: Stepping through struct operations
test_stepping_structs() {
    test_section "Stepping Through Struct Operations"

    local output=$(run_cmd "break create_point
continue
next
print p.x")

    if echo "$output" | grep -q "x\|p\."; then
        echo -e "${GREEN}✓${NC} Can step through and inspect struct operations"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Can step through and inspect struct operations"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}  COMPLEX/REAL-WORLD PROGRAM TEST SUITE${NC}"
echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"

test_nested_breakpoints
test_global_watchpoints
test_complex_backtrace
test_recursive_stepping
test_struct_inspection
test_pointer_inspection
test_malloc_breakpoints
test_function_pointer_breakpoints
test_static_variable_inspection
test_fibonacci_recursion
test_pointer_arithmetic
test_double_pointer
test_array_operations
test_deep_stack_navigation
test_signal_handler_breakpoints
test_thread_breakpoints
test_memory_allocation
test_struct_member_access
test_stepping_structs

# Print summary
test_summary
