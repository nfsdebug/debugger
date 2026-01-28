#!/bin/bash
# @file test_watchpoints.sh
# @brief Watchpoint functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_globals" 2>&1 || true
}

test_watchpoint_set() {
    test_section "Set Watchpoint"

    local output=$(run_cmd "b main
continue
watch 0x4014")
    if echo "$output" | grep -q "Watchpoint.*set"; then
        echo -e "${GREEN}✓${NC} Watchpoint set command works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Watchpoint set command works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_watchpoint_alias() {
    test_section "Watchpoint Alias"

    local output=$(run_cmd "b main
continue
w 0x4014")
    if echo "$output" | grep -q "Watchpoint.*set\|adjusted"; then
        echo -e "${GREEN}✓${NC} Alias 'w' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'w' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_watchpoint_list() {
    test_section "List Watchpoints"

    local output=$(run_cmd "b main
continue
watch 0x4014
watchpoint list")
    if echo "$output" | grep -q "Watchpoints:"; then
        echo -e "${GREEN}✓${NC} Watchpoint list works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Watchpoint list works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_watchpoint_set
test_watchpoint_alias
test_watchpoint_list
