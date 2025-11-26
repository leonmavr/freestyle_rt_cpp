#!/bin/bash
set -euo pipefail

### About:
# Converts all .png files in "textures" directory to P3 8-bit .ppm format
### Requirements:
# ImageMagick (`magick` or `convert` command)

readonly script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly textures_dir="$script_dir/../textures"

[ ! -d "$textures_dir" ] &&\
    echo "Textures directory not found: $textures_dir" >&2 &&\
    exit 1

if command -v magick >/dev/null 2>&1; then
    CONVERT_CMD=(magick)
elif command -v convert >/dev/null 2>&1; then
    CONVERT_CMD=(convert)
else
    echo "ERROR: ImageMagick not installed. Exiting..." >&2
    exit 1
fi

readonly pngs=("$textures_dir"/*.png)
if [ ${#pngs[@]} -eq 0 ]; then
    echo "No .png files found in: $textures_dir"
    exit 0
fi

for f in "${pngs[@]}"; do
    "${CONVERT_CMD[@]}" "$f" -depth 8 -compress none -define ppm:format=plain "${f%.png}.ppm"
done

echo "=== Converted all png textures to ppm ==="