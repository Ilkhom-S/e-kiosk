#!/bin/bash
# Run clang-format recursively on all C/C++ source files in the repo (macOS/Linux version)
# Excludes build, bin, out, thirdparty, node_modules, .git, vcpkg_installed

ROOT="$(cd "${1:-$(dirname "$0")/..}" && pwd)"
EXE="clang-format"
EXTS="h hpp c cpp cc cxx"
EXCLUDE_DIRS="build bin out thirdparty node_modules .git vcpkg_installed"

echo "ROOT: $ROOT"
echo "EXTS: $EXTS"
echo "EXCLUDE_DIRS: $EXCLUDE_DIRS"

if ! command -v "$EXE" >/dev/null 2>&1; then
  echo "clang-format not found in PATH. Install LLVM/clang or add clang-format to PATH." >&2
  exit 2
fi

EXCLUDE_PATTERN="build|bin|out|thirdparty|node_modules|\.git|vcpkg_installed"

for e in $EXTS; do
  find "$ROOT" -type f -name "*.$e" -print | grep -v -E "/($EXCLUDE_PATTERN)/" | while read -r file; do
    echo "Formatting $file"
    "$EXE" -i "$file"
  done
done

echo "clang-format run complete."