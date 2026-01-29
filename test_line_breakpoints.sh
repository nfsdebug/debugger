#!/bin/bash

echo "=== Testing Line-Based Breakpoint Functionality ==="
echo ""

DEBUGGER="./target/main_interactive"
TARGET="./target/test_breakpoint_line"

if [ ! -f "$DEBUGGER" ]; then
    echo "ERROR: Debugger not found at $DEBUGGER"
    exit 1
fi

if [ ! -f "$TARGET" ]; then
    echo "ERROR: Test target not found at $TARGET"
    exit 1
fi

echo "Test 1: Check if DWARF info is loaded"
echo "Command: info functions"
echo ""

# Test with a script that runs the debugger and sets a line breakpoint
cat > /tmp/test_bp.txt << 'INNER_EOF'
info functions
b test_breakpoint_line.c:22
breakpoint list
quit
INNER_EOF

echo "Running debugger with automated test..."
$DEBUGGER $TARGET < /tmp/test_bp.txt 2>&1 | head -50

echo ""
echo "=== Test Complete ==="
