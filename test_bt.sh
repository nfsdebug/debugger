#!/bin/bash

# Test script for backtrace functionality

echo "=== Testing Backtrace Feature ==="
echo ""

# Test 1: Run main_interactive with test_backtrace and use 'bt' command
echo "Test 1: Using 'bt' command in main_interactive"
echo ""

# Create a command script for the debugger
cat > /tmp/bt_test.cmd << 'EOF'
break main
run
continue
bt
quit
EOF

./target/main_interactive ./target/test_backtrace < /tmp/bt_test.cmd 2>&1 | tee /tmp/bt_test_output.txt

echo ""
echo "=== Checking for backtrace output ==="
grep -A10 "BACKTRACE" /tmp/bt_test_output.txt || echo "ERROR: No BACKTRACE section found!"
grep "level3\|level2\|level1\|main" /tmp/bt_test_output.txt || echo "ERROR: No function names in backtrace!"

echo ""
echo "=== Full output saved to /tmp/bt_test_output.txt ==="
