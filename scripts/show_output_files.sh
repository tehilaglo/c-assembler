#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$PROJECT_ROOT/output_files"

# Create output directory in the parent directory
mkdir -p "$OUTPUT_DIR"

# Recursively search the parent directory and move matching files
find "$PROJECT_ROOT" -path "$OUTPUT_DIR" -prune -o \
-type f \( -name "*.am" -o -name "*.ob" -o -name "*.ent" -o -name "*.ext" \) \
-exec mv {}  "$OUTPUT_DIR"/ \;

echo "Output files moved to $OUTPUT_DIR"
