#!/usr/bin/env bash
set -e

echo "=========================================="
echo "Running Pass1 Error Tests..."
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

leaks -atExit -- \
"$PROJECT_ROOT/assembler" \
"$PROJECT_ROOT/examples/errors/pass1_errors_1" \
"$PROJECT_ROOT/examples/errors/pass1_errors_2" \
"$PROJECT_ROOT/examples/errors/pass1_errors_3" \
2>&1 | tee "$PROJECT_ROOT/pass1_errors_output"

echo "Pass1 error tests completed."
