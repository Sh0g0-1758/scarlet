#!/usr/bin/env bash
#
# clean.sh — remove all .o, .s and executables (except “scarlet”)
#
# Usage:
#   ./clean.sh [directory]
#
# If no directory is given, operates in the current directory.

set -euo pipefail

# 1) Determine directory, then switch into it
DIR="${1:-.}"
cd "$DIR" || { echo "ERROR: Cannot cd into '$DIR'"; exit 1; }

# 2) Make sure globs that don’t match just expand to nothing
shopt -s nullglob

# 3) Loop over all non-hidden entries in the directory
for file in *; do
  # skip non-regular files
  [[ -f "$file" ]] || continue

  # never delete “scarlet”
  [[ "$file" == "scarlet" ]] && continue

  # delete if it’s a .o or .s, or if it’s executable by anyone
  if [[ "$file" == *.o ]] || [[ "$file" == *.s ]] || [[ -x "$file" ]]; then
    rm -- "$file"
  fi
done
