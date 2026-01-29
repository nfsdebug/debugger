#!/bin/bash
# @file all_tests.sh
# @brief Run all tests

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Source test framework
source "$SCRIPT_DIR/framework.sh"

# Test binaries directory
TEST_BIN_DIR="$PROJECT_ROOT/tests/binaries"
mkdir -p "$TEST_BIN_DIR"

# Build test programs
build_test_programs() {
    test_header "Building Test Programs"
    echo "Building test programs in $TEST_BIN_DIR..."

    # Only build existing test programs
    if [ -f "$SCRIPT_DIR/programs/test_simple.c" ]; then
        gcc -g -O0 -o "$TEST_BIN_DIR/test_simple" "$SCRIPT_DIR/programs/test_simple.c"
    fi

    if [ -f "$SCRIPT_DIR/programs/test_functions.c" ]; then
        gcc -g -O0 -o "$TEST_BIN_DIR/test_functions" "$SCRIPT_DIR/programs/test_functions.c"
    fi

    if [ -f "$SCRIPT_DIR/programs/test_loops.c" ]; then
        gcc -g -O0 -o "$TEST_BIN_DIR/test_loops" "$SCRIPT_DIR/programs/test_loops.c"
    fi

    if [ -f "$SCRIPT_DIR/programs/test_globals.c" ]; then
        gcc -g -O0 -o "$TEST_BIN_DIR/test_globals" "$SCRIPT_DIR/programs/test_globals.c"
    fi

    # Complex test programs for advanced testing
    if [ -f "$SCRIPT_DIR/programs/test_complex.c" ]; then
        gcc -g -O0 -o "$TEST_BIN_DIR/test_complex" "$SCRIPT_DIR/programs/test_complex.c" -lpthread
    fi

    echo "Test programs built successfully"
}

# Run all test suites
run_all_tests() {
    test_header "Running All Test Suites"

    # Build test programs
    build_test_programs

    # Source all test suites
    for test_suite in "$SCRIPT_DIR"/suites/*.sh; do
        if [ -f "$test_suite" ]; then
            echo ""
            source "$test_suite"
        fi
    done

    # Print summary
    test_summary

    # Return exit code based on results
    if [ $TESTS_FAILED -gt 0 ]; then
        exit 1
    else
        exit 0
    fi
}

# Run specific test suite
run_suite() {
    local suite_name="$1"

    build_test_programs

    local suite_file="$SCRIPT_DIR/suites/${suite_name}.sh"
    if [ -f "$suite_file" ]; then
        source "$suite_file"
        test_summary

        if [ $TESTS_FAILED -gt 0 ]; then
            exit 1
        fi
    else
        echo "Test suite '$suite_name' not found"
        exit 1
    fi
}

# Main
if [ $# -eq 0 ]; then
    run_all_tests
else
    run_suite "$1"
fi
