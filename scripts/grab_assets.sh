#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."

set -e

cd "$ROOT_DIR"
git clone https://github.com/ZQuestClassic/commons .tmp/commons || true
cd .tmp/commons
git pull

assets_dir="$ROOT_DIR/resources/assets/zc"
credits="$assets_dir/CREDITS.txt"
rm "$credits" || true

echo "= ZC_Logo.png\n" >> "$credits"
cat "ZC Logo & Icon/CREDITS.txt" >> "$credits"
cp "ZC Logo & Icon/ZC_Logo.png" "$assets_dir"
convert "$assets_dir/ZC_Logo.png" -trim +repage "$assets_dir/ZC_Logo.png"

echo "\n= ZC_Forever_HD.mp3\n" >> "$credits"
cat "ZC Forever/CREDITS.txt" >> "$credits"
cp "ZC Forever/ZC_Forever_HD.mp3" "$assets_dir"
