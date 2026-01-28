#!/bin/bash
# @file run_test.sh
# @brief Helper script to run a single test command

DEBUGGER="$1"
PROGRAM="$2"
shift 2
COMMANDS="$@"

# Create a temporary file with commands
CMD_FILE=$(mktemp)
echo "$COMMANDS" > "$CMD_FILE"

# Run debugger with command file
timeout 5 "$DEBUGGER" "$PROGRAM" < "$CMD_FILE"

# Cleanup
rm -f "$CMD_FILE"
