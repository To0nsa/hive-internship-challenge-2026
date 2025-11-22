#!/usr/bin/env bash
# clang-format-all.sh — format all C/C++ sources in ../src with clang-format

set -euo pipefail

# Resolve project root and src dir relative to this script
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/src"

# Make sure clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
  echo "error: clang-format not found in PATH" >&2
  exit 1
fi

echo "Formatting sources under: $SRC_DIR"

# Find candidate files, skipping typical third-party dirs
find "$SRC_DIR" \
  \( -path '*/third_party/*' -o -path '*/external/*' \) -prune -o \
  -type f \( \
      -name '*.c'    -o \
      -name '*.cc'   -o \
      -name '*.cpp'  -o \
      -name '*.cxx'  -o \
      -name '*.h'    -o \
      -name '*.hh'   -o \
      -name '*.hpp'  -o \
      -name '*.hxx' \
    \) -print0 \
  | xargs -0 clang-format -i

echo "Done."
