#!/usr/bin/env bash
# export-all.sh — bundle all .h and .cpp from src/ into one txt file.
# Usage:
#   ./export-all.sh                 # uses survive/src -> survive/scripts/output/src-bundle.txt
#   ./export-all.sh <SRC_DIR>       # custom source, same default output path
#   ./export-all.sh <SRC_DIR> <OUT> # fully custom paths

set -euo pipefail

# Resolve paths relative to this script
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"

SRC_DIR="${1:-"$ROOT_DIR/src"}"
OUT_FILE="${2:-"$SCRIPT_DIR/output/src-bundle.txt"}"

mkdir -p "$(dirname "$OUT_FILE")"
: > "$OUT_FILE"  # truncate

# Collect only .h and .cpp (skip hidden paths), sorted deterministically
while IFS= read -r -d '' f; do
  {
    echo "===== BEGIN $f ====="
    cat "$f"
    echo
    echo "===== END $f ====="
    echo
  } >> "$OUT_FILE"
done < <(
  find "$SRC_DIR" -type f ! -path '*/.*' \( -name '*.h' -o -name '*.cpp' \) -print0 | sort -z
)

# Small summary
files_count=$(find "$SRC_DIR" -type f ! -path '*/.*' \( -name '*.h' -o -name '*.cpp' \) | wc -l | tr -d ' ')
lines_count=$(wc -l < "$OUT_FILE" | tr -d ' ')
echo "Bundled $files_count files into $OUT_FILE ($lines_count lines)."
