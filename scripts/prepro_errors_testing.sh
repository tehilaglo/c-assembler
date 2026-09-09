#!/usr/bin/env bash
set -e

echo "=========================================="
echo "Running Preprocessor Error Tests..."
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

leaks -atExit -- \
"$PROJECT_ROOT/assembler" \
"$PROJECT_ROOT/examples/errors/prepro_errors_example" \
2>&1 | tee "$PROJECT_ROOT/prepro_errors_output"

echo "Preprocessor error tests completed."
