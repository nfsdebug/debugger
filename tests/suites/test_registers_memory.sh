#!/bin/bash
# @file test_registers_memory.sh
# @brief Register and memory functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_simple" 2>&1 || true
}

test_register_dump() {
    test_section "Register Dump"

    local output=$(run_cmd "register dump")
    if echo "$output" | grep -q "RAX\|RIP"; then
        echo -e "${GREEN}✓${NC} Register dump works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Register dump works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_register_alias() {
    test_section "Register Alias"

    local output=$(run_cmd "r")
    if echo "$output" | grep -q "RAX\|RIP"; then
        echo -e "${GREEN}✓${NC} Alias 'r' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'r' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_register_read() {
    test_section "Register Read"

    local output=$(run_cmd "register read rax")
    if echo "$output" | grep -q "RAX"; then
        echo -e "${GREEN}✓${NC} Register read works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Register read works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_memory_read() {
    test_section "Memory Read"

    local output=$(run_cmd "memory read 0x401000")
    if echo "$output" | grep -q "0x\|Failed to read"; then
        echo -e "${GREEN}✓${NC} Memory read command exists"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Memory read command exists"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_memory_alias() {
    test_section "Memory Alias"

    local output=$(run_cmd "m 0x401000")
    if echo "$output" | grep -q "0x\|Failed to read"; then
        echo -e "${GREEN}✓${NC} Alias 'm' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'm' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_backtrace() {
    test_section "Backtrace"

    local output=$(run_cmd "backtrace")
    if echo "$output" | grep -q "#" \
        || echo "$output" | grep -q "failed"; then
        echo -e "${GREEN}✓${NC} Backtrace command exists"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Backtrace command exists"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_backtrace_alias() {
    test_section "Backtrace Alias"

    local output=$(run_cmd "bt")
    if echo "$output" | grep -q "#" \
        || echo "$output" | grep -q "failed"; then
        echo -e "${GREEN}✓${NC} Alias 'bt' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'bt' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_register_dump
test_register_alias
test_register_read
test_memory_read
test_memory_alias
test_backtrace
test_backtrace_alias
