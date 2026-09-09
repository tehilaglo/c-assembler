#!/usr/bin/env bash
set -e

echo "=========================================="
echo "Running Pass2 Error Tests..."
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

leaks -atExit -- \
"$PROJECT_ROOT/assembler" \
"$PROJECT_ROOT/examples/errors/pass2_errors" \
2>&1 | tee "$PROJECT_ROOT/pass2_errors_output"

echo "Pass2 error tests completed."
