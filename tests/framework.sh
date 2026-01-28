#!/bin/bash
# @file framework.sh
# @brief Test framework for debugger

# Colors
export RED='\033[0;31m'
export GREEN='\033[0;32m'
export YELLOW='\033[1;33m'
export BLUE='\033[0;34m'
export CYAN='\033[0;36m'
export NC='\033[0m' # No Color

# Test statistics
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# Test results file
TEST_RESULTS="/tmp/debugger_test_results_$$"
echo "" > "$TEST_RESULTS"

# Print test header
test_header() {
    local name="$1"
    echo -e "\n${CYAN}══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}  TEST: $name${NC}"
    echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"
}

# Print section header
test_section() {
    local name="$1"
    echo -e "\n${BLUE}─── $name ───${NC}"
}

# Assert that a command succeeds
assert_success() {
    local description="$1"
    shift
    local output

    TESTS_RUN=$((TESTS_RUN + 1))

    if output=$("$@" 2>&1); then
        echo -e "${GREEN}✓${NC} $description"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        echo "PASS: $description" >> "$TEST_RESULTS"
        return 0
    else
        echo -e "${RED}✗${NC} $description"
        echo -e "${RED}  Command: $*${NC}"
        echo -e "${RED}  Output: $output${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        echo "FAIL: $description - Command: $*" >> "$TEST_RESULTS"
        return 1
    fi
}

# Assert that a command fails
assert_fail() {
    local description="$1"
    shift
    local output

    TESTS_RUN=$((TESTS_RUN + 1))

    if output=$("$@" 2>&1); then
        echo -e "${RED}✗${NC} $description (command should have failed)"
        echo -e "${RED}  Command: $*${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        echo "FAIL: $description - Command succeeded when it should fail: $*" >> "$TEST_RESULTS"
        return 1
    else
        echo -e "${GREEN}✓${NC} $description"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        echo "PASS: $description" >> "$TEST_RESULTS"
        return 0
    fi
}

# Assert that output contains a string
assert_contains() {
    local description="$1"
    local needle="$2"
    shift 2
    local output

    TESTS_RUN=$((TESTS_RUN + 1))

    output=$("$@" 2>&1)

    if echo "$output" | grep -q "$needle"; then
        echo -e "${GREEN}✓${NC} $description"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        echo "PASS: $description" >> "$TEST_RESULTS"
        return 0
    else
        echo -e "${RED}✗${NC} $description"
        echo -e "${RED}  Expected to find: $needle${NC}"
        echo -e "${RED}  In output: $output${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        echo "FAIL: $description - Expected '$needle' in output" >> "$TEST_RESULTS"
        return 1
    fi
}

# Assert that output does NOT contain a string
assert_not_contains() {
    local description="$1"
    local needle="$2"
    shift 2
    local output

    TESTS_RUN=$((TESTS_RUN + 1))

    output=$("$@" 2>&1)

    if echo "$output" | grep -q "$needle"; then
        echo -e "${RED}✗${NC} $description (found unexpected: $needle)"
        echo -e "${RED}  Output: $output${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        echo "FAIL: $description - Found unexpected '$needle' in output" >> "$TEST_RESULTS"
        return 1
    else
        echo -e "${GREEN}✓${NC} $description"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        echo "PASS: $description" >> "$TEST_RESULTS"
        return 0
    fi
}

# Assert that two values are equal
assert_equals() {
    local description="$1"
    local expected="$2"
    local actual="$3"

    TESTS_RUN=$((TESTS_RUN + 1))

    if [ "$expected" = "$actual" ]; then
        echo -e "${GREEN}✓${NC} $description"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        echo "PASS: $description" >> "$TEST_RESULTS"
        return 0
    else
        echo -e "${RED}✗${NC} $description"
        echo -e "${RED}  Expected: $expected${NC}"
        echo -e "${RED}  Actual: $actual${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        echo "FAIL: $description - Expected '$expected', got '$actual'" >> "$TEST_RESULTS"
        return 1
    fi
}

# Skip a test
test_skip() {
    local description="$1"
    local reason="$2"

    TESTS_RUN=$((TESTS_RUN + 1))
    TESTS_SKIPPED=$((TESTS_SKIPPED + 1))

    echo -e "${YELLOW}⊘${NC} $description"
    echo -e "${YELLOW}  Reason: $reason${NC}"
    echo "SKIP: $description - $reason" >> "$TEST_RESULTS"
    return 0
}

# Print test summary
test_summary() {
    echo -e "\n${CYAN}══════════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}  TEST SUMMARY${NC}"
    echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}"
    echo -e "  Total:   $TESTS_RUN"
    echo -e "${GREEN}  Passed:  $TESTS_PASSED${NC}"
    if [ $TESTS_FAILED -gt 0 ]; then
        echo -e "${RED}  Failed:  $TESTS_FAILED${NC}"
    else
        echo -e "  Failed:  $TESTS_FAILED"
    fi
    if [ $TESTS_SKIPPED -gt 0 ]; then
        echo -e "${YELLOW}  Skipped: $TESTS_SKIPPED${NC}"
    fi
    echo -e "${CYAN}══════════════════════════════════════════════════════════════${NC}\n"

    # Write summary to results file
    {
        echo "=== TEST SUMMARY ==="
        echo "Total:   $TESTS_RUN"
        echo "Passed:  $TESTS_PASSED"
        echo "Failed:  $TESTS_FAILED"
        echo "Skipped: $TESTS_SKIPPED"
    } >> "$TEST_RESULTS"

    # Return non-zero if any tests failed
    [ $TESTS_FAILED -eq 0 ]
}

# Get test results file
get_results_file() {
    echo "$TEST_RESULTS"
}

# Cleanup test artifacts
test_cleanup() {
    rm -f "$TEST_RESULTS"
}
