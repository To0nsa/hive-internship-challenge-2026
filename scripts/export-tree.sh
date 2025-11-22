#!/usr/bin/env bash
# export-tree.sh — dump a directory tree to a txt file, ignoring .git and .vscode
# Usage:
#   ./export-tree.sh                  # tree of project root -> scripts/output/tree.txt
#   ./export-tree.sh <DIR>            # tree of DIR -> scripts/output/tree.txt
#   ./export-tree.sh <DIR> <OUTFILE>  # tree of DIR -> custom OUTFILE

set -euo pipefail

# Directory where this script lives (scripts/)
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# Assume project root is parent of scripts/ by default
ROOT_DIR_DEFAULT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
ROOT_DIR="${1:-"$ROOT_DIR_DEFAULT"}"

# Default output under scripts/output/
OUT_FILE="${2:-"$SCRIPT_DIR/output/tree.txt"}"

if ! command -v tree >/dev/null 2>&1; then
  echo "Error: 'tree' is not installed. Install it (e.g. 'sudo apt-get install tree') and retry." >&2
  exit 1
fi

mkdir -p "$(dirname "$OUT_FILE")"

# -a : include hidden files
# -I : ignore .git and .vscode
tree -a -I '.git|.vscode|build' "$ROOT_DIR" > "$OUT_FILE"

echo "Tree written to: $OUT_FILE"
