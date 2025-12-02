#!/bin/bash
set -euo pipefail

### About:
# Converts all .png, .jpg and .jpeg files in `resources/textures` and `resources/bg`
# directories to P3 8-bit .ppm format
### Requirements:
# ImageMagick (`magick` or `convert` command)

readonly script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly textures_dir="$script_dir/../resources/textures"
readonly bg_dir="$script_dir/../resources/bg"

# allow missing directories but warn
[ ! -d "$textures_dir" ] && echo "Warning: Textures directory not found: $textures_dir" >&2
[ ! -d "$bg_dir" ] && echo "Warning: BG directory not found: $bg_dir" >&2

if command -v magick >/dev/null 2>&1; then
    CONVERT_CMD=(magick)
elif command -v convert >/dev/null 2>&1; then
    CONVERT_CMD=(convert)
else
    echo "ERROR: ImageMagick not installed. Exiting..." >&2
    exit 1
fi

# better globbing - if no matches, the arrays will be empty instead of containing the pattern
shopt -s nullglob

# collect images from both directories
images=()
[ -d "$textures_dir" ] && for p in "$textures_dir"/*.{png,jpg,jpeg}; do images+=("$p"); done
[ -d "$bg_dir" ] && for p in "$bg_dir"/*.{png,jpg,jpeg}; do images+=("$p"); done

[ ${#images[@]} -eq 0 ] &&\
    echo "No image files (.png/.jpg/.jpeg) found in: $textures_dir or $bg_dir" &&\
    exit 0

count=0
for f in "${images[@]}"; do
    outfile="${f%.*}.ppm"
    "${CONVERT_CMD[@]}" "$f" -depth 8 -compress none -define ppm:format=plain "$outfile"
    count=$((count + 1))
done

echo "=== Converted $count images to .ppm ==="
