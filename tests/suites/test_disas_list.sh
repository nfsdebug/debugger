#!/bin/bash
# @file test_disas_list.sh
# @brief Disassembly and source listing functionality tests (robust version)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
DEBUGGER="$PROJECT_ROOT/target/main_interactive"

run_cmd() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_functions" 2>&1 || true
}

run_cmd_simple() {
    local cmds="$1"
    (echo "$cmds"; sleep 1; echo "quit") | timeout 3 "$DEBUGGER" "$TEST_BIN_DIR/test_simple" 2>&1 || true
}

test_disasm_command() {
    test_section "Disassemble Command"

    local output=$(run_cmd "disas")
    if echo "$output" | grep -q "DISASSEMBLY"; then
        echo -e "${GREEN}✓${NC} Disasm command works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Disasm command works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_disasm_alias() {
    test_section "Disassemble Alias"

    local output=$(run_cmd "disassemble")
    if echo "$output" | grep -q "DISASSEMBLY"; then
        echo -e "${GREEN}✓${NC} Full name 'disassemble' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Full name 'disassemble' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_disasm_function() {
    test_section "Disassemble Function"

    local output=$(run_cmd "disas main")
    if echo "$output" | grep -q "resolved to\|DISASSEMBLY"; then
        echo -e "${GREEN}✓${NC} Disasm function works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Disasm function works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_disasm_options() {
    test_section "Disassemble Options"

    local output=$(run_cmd_simple "disas --before=2 --after=2")
    if echo "$output" | grep -q "DISASSEMBLY"; then
        echo -e "${GREEN}✓${NC} Disasm options work"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Disasm options work"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_list_command() {
    test_section "List Command"

    local output=$(run_cmd_simple "list")
    if echo "$output" | grep -q "SOURCE\|File:"; then
        echo -e "${GREEN}✓${NC} List command works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} List command works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_list_alias() {
    test_section "List Alias"

    local output=$(run_cmd_simple "l")
    if echo "$output" | grep -q "SOURCE\|File:"; then
        echo -e "${GREEN}✓${NC} Alias 'l' works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Alias 'l' works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_info_functions() {
    test_section "Info Functions"

    local output=$(run_cmd "info functions")
    if echo "$output" | grep -q "FUNCTIONS\|functions"; then
        echo -e "${GREEN}✓${NC} Info functions works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Info functions works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

test_info_alias() {
    test_section "Info Alias"

    local output=$(run_cmd "info")
    if echo "$output" | grep -q "FUNCTIONS\|functions"; then
        echo -e "${GREEN}✓${NC} Info alias works"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} Info alias works"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run all tests in this suite
test_disasm_command
test_disasm_alias
test_disasm_function
test_disasm_options
test_list_command
test_list_alias
test_info_functions
test_info_alias
