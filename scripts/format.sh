#!/usr/bin/env bash
# Usage: ./scripts/format.sh [--check]
set -euo pipefail

CHECK="${1:-}"
FILES=$(find src/ tests/ -name '*.cpp' -o -name '*.hpp' | sort)

if [ "$CHECK" = "--check" ]; then
  clang-format --dry-run --Werror $FILES
  echo "✅ All files formatted correctly"
else
  clang-format -i $FILES
  echo "✅ Formatted $(echo "$FILES" | wc -w) files"
fi
