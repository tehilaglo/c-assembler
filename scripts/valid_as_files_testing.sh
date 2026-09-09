#!/usr/bin/env bash
set -e

echo "=========================================="
echo "Running Valid Example Tests..."
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

leaks -atExit -- \
"$PROJECT_ROOT/assembler" \
"$PROJECT_ROOT/examples/valid/as_example_1" \
"$PROJECT_ROOT/examples/valid/as_example_2" \
"$PROJECT_ROOT/examples/valid/as_example_3" \
"$PROJECT_ROOT/examples/valid/as_example_4" \
"$PROJECT_ROOT/examples/valid/as_example_5" \
"$PROJECT_ROOT/examples/valid/as_example_6" \
"$PROJECT_ROOT/examples/valid/as_example_7" \
"$PROJECT_ROOT/examples/valid/as_example_8" \
2>&1 | tee "$PROJECT_ROOT/valid_as_examples_output"

echo "Valid example tests completed. (output files saved to $PROJECT_ROOT/examples/valid)"
