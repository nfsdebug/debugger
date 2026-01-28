#!/bin/bash
# @file test_breakpoints.sh
# @brief Breakpoint functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

# Helper to run command and check output
run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_functions" 2>&1 || true
}

test_breakpoint_at_function() {
    test_section "Breakpoint at Function"

    local output=$(run_cmd "b main")
    if echo "$output" | grep -q "Breakpoint.*set"; then
        echo -e "${GREEN}✓${NC} Set breakpoint at 'main'"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Set breakpoint at 'main'"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_at_helper() {
    test_section "Breakpoint at Helper Function"

    local output=$(run_cmd "b helper_function")
    if echo "$output" | grep -q "Breakpoint.*set"; then
        echo -e "${GREEN}✓${NC} Set breakpoint at 'helper_function'"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Set breakpoint at 'helper_function'"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_list() {
    test_section "List Breakpoints"

    local output=$(run_cmd "b main
breakpoint list")
    if echo "$output" | grep -q "Breakpoint #0"; then
        echo -e "${GREEN}✓${NC} List shows breakpoints"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} List shows breakpoints"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_alias_bl() {
    test_section "Breakpoint List Alias"

    local output=$(run_cmd "b main
bl")
    if echo "$output" | grep -q "Breakpoint"; then
        echo -e "${GREEN}✓${NC} Alias 'bl' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'bl' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_multiple_breakpoints() {
    test_section "Multiple Breakpoints"

    local output=$(run_cmd "b main
b helper_function
breakpoint list")
    if echo "$output" | grep -q "Breakpoint #1"; then
        echo -e "${GREEN}✓${NC} Multiple breakpoints set"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Multiple breakpoints set"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_disable() {
    test_section "Disable Breakpoint"

    local output=$(run_cmd "b main
breakpoint disable 0
breakpoint list")
    if echo "$output" | grep -q "disabled"; then
        echo -e "${GREEN}✓${NC} Breakpoint disabled"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Breakpoint disabled"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_enable() {
    test_section "Enable Breakpoint"

    local output=$(run_cmd "b main
breakpoint disable 0
breakpoint enable 0
breakpoint list")
    if echo "$output" | grep -q "enabled"; then
        echo -e "${GREEN}✓${NC} Breakpoint enabled"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Breakpoint enabled"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_breakpoint_delete() {
    test_section "Delete Breakpoint"

    local output=$(run_cmd "b main
breakpoint delete 0
breakpoint list")
    if echo "$output" | grep -q "No breakpoints"; then
        echo -e "${GREEN}✓${NC} Breakpoint deleted"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Breakpoint deleted"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_breakpoint_at_function
test_breakpoint_at_helper
test_breakpoint_list
test_breakpoint_alias_bl
test_multiple_breakpoints
test_breakpoint_disable
test_breakpoint_enable
test_breakpoint_delete
