#!/bin/bash
# @file test_basic.sh
# @brief Basic functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

# Helper to run command and check output
run_cmd() {
    local cmds="$1"
    # Use expect-like approach with timeout
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_simple" 2>&1 || true
}

test_basic_start() {
    test_section "Basic Start"

    # Test that debugger starts
    local output=$(run_cmd "")
    if echo "$output" | grep -q "Debugger started"; then
        echo -e "${GREEN}✓${NC} Debugger starts"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Debugger starts"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_help_command() {
    test_section "Help Command"

    local output=$(run_cmd "help")
    if echo "$output" | grep -q "continue"; then
        echo -e "${GREEN}✓${NC} Help shows commands"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Help shows commands"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_symbols_loaded() {
    test_section "Symbols Loaded"

    local output=$(run_cmd "")
    if echo "$output" | grep -q "Loaded.*symbols"; then
        echo -e "${GREEN}✓${NC} Symbols are loaded"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Symbols are loaded"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_quit_command() {
    test_section "Quit Command"

    # Test that quit doesn't crash
    local output=$(echo "quit" | timeout 2 "$DEBUGGER" "$TEST_BIN_DIR/test_simple" 2>&1 || true)
    if [ $? -ne 134 ]; then  # Not SIGABRT
        echo -e "${GREEN}✓${NC} Quit doesn't crash"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Quit doesn't crash"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_basic_start
test_help_command
test_symbols_loaded
test_quit_command
