#!/usr/bin/env bash
# run-all.sh — run all project maintenance scripts in order

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

echo "[1/3] Running clang-format-all.sh..."
"$SCRIPT_DIR/clang-format-all.sh"

echo "[2/3] Running export-all.sh..."
"$SCRIPT_DIR/export-all.sh"

echo "[3/3] Running export-tree.sh..."
"$SCRIPT_DIR/export-tree.sh"

echo "✅ All scripts completed."
