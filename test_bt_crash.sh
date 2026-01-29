#!/bin/bash

# Test script for backtrace on crash

echo "=== Testing Backtrace on SIGSEGV ==="
echo ""

# Create a command script for the debugger - let it run until crash
cat > /tmp/bt_crash_test.cmd << 'EOF'
run
EOF

timeout 5 ./target/main_interactive ./target/test_backtrace < /tmp/bt_crash_test.cmd 2>&1 | tee /tmp/bt_crash_output.txt

echo ""
echo "=== Checking for crash and backtrace ==="
echo ""
echo "Looking for SIGSEGV signal..."
grep -i "sigsegv\|segmentation" /tmp/bt_crash_output.txt && echo "✓ SIGSEGV detected!" || echo "✗ No SIGSEGV found"

echo ""
echo "Looking for BACKTRACE output..."
grep -A5 "BACKTRACE" /tmp/bt_crash_output.txt && echo "✓ BACKTRACE found!" || echo "✗ No BACKTRACE found"

echo ""
echo "Looking for function names in backtrace..."
grep -E "level3|level2|level1" /tmp/bt_crash_output.txt && echo "✓ Function names found!" || echo "Note: Function symbols may not be available"

echo ""
echo "=== Full output ==="
grep -A10 "BACKTRACE" /tmp/bt_crash_output.txt

echo ""
echo "Full output saved to /tmp/bt_crash_output.txt"
