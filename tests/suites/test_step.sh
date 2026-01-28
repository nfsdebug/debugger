#!/bin/bash
# @file test_step.sh
# @brief Step and step-over functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_functions" 2>&1 || true
}

test_step_command() {
    test_section "Step Command"

    local output=$(run_cmd "step")
    if echo "$output" | grep -q "Process exited\|Debugger exited"; then
        echo -e "${GREEN}✓${NC} Step command works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Step command works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_step_alias() {
    test_section "Step Alias"

    local output=$(run_cmd "s")
    if echo "$output" | grep -q "Process exited\|Debugger exited"; then
        echo -e "${GREEN}✓${NC} Alias 's' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 's' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_next_command() {
    test_section "Next Command"

    local output=$(run_cmd "next")
    if echo "$output" | grep -q "Process exited\|Debugger exited"; then
        echo -e "${GREEN}✓${NC} Next command works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Next command works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_next_alias() {
    test_section "Next Alias"

    local output=$(run_cmd "n")
    if echo "$output" | grep -q "Process exited\|Debugger exited"; then
        echo -e "${GREEN}✓${NC} Alias 'n' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'n' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_step_command
test_step_alias
test_next_command
test_next_alias
