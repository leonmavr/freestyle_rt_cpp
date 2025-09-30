#!/bin/bash

if ! command -v clang-format &> /dev/null
then
    exit 1
fi

readonly ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/..
cd "$ROOT_DIR"
echo $ROOT_DIR


TARGETS=("src" "include" "main.cpp")
EXTENSIONS=("*.cpp" "*.cc" "*.c" "*.h" "*.hpp")

# Format each target
for target in "${TARGETS[@]}"; do
  if [ -d "$target" ]; then
    # It's a directory – format all matching files recursively
    for ext in "${EXTENSIONS[@]}"; do
      find "$target" -type f -name "$ext" | while read -r file; do
        clang-format -i "$file"
        sed -i 's/\t/  /g' "$file"
      done
    done
  elif [ -f "$target" ]; then
    # It's a file – format directly
    clang-format -i "$target"
    sed -i 's/\t/  /g' "$target"
  fi
done

echo "=== clang-format applied to all source files. ==="

