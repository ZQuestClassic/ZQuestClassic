#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."

set -e

cd "$ROOT_DIR"
git clone https://github.com/ZQuestClassic/commons .tmp/commons || true
cd .tmp/commons
git pull

assets_dir="$ROOT_DIR/resources/assets/zc"
commons_dir="$ROOT_DIR/.tmp/commons"
credits="$assets_dir/CREDITS.txt"
rm "$credits" || true

echo "= ZC_Icon_Medium_Editor.png\n" >> "$credits"
echo "= ZC_Icon_Medium_Launcher.png\n" >> "$credits"
echo "= ZC_Icon_Medium_Player.png\n" >> "$credits"
echo "= ZC_Logo.png\n" >> "$credits"
cat "ZC Logo & Icon/CREDITS.txt" >> "$credits"

# Windows won't use these - instead we make a .ico and embed directly in the executable.
cp "ZC Logo & Icon/ZC_Icon_Medium_Editor.png" "$assets_dir"
cp "ZC Logo & Icon/ZC_Icon_Medium_Launcher.png" "$assets_dir"
cp "ZC Logo & Icon/ZC_Icon_Medium_Player.png" "$assets_dir"

cp "ZC Logo & Icon/ZC_Logo.png" "$assets_dir"
convert "$assets_dir/ZC_Logo.png" -define png:exclude-chunks=date,time -trim +repage "$assets_dir/ZC_Logo.png"

echo "\n= ZC_Forever_HD.mp3\n" >> "$credits"
cat "ZC Forever/CREDITS.txt" >> "$credits"
cp "ZC Forever/ZC_Forever_HD.mp3" "$assets_dir"

# Create .ico files for icons on Windows.

create_icon () {
	small=$1
	medium=$2
	output=$3

	# First few sizes uses low detail logo.
	convert "$small" -resize 16x16 icon_16x16.png
	convert "$small" -resize 32x32 icon_32x32.png
	convert "$small" -resize 48x48 icon_48x48.png

	# Biggest sizes uses high detail logo.
	convert "$medium" -resize 128x128 icon_128x128.png
	convert "$medium" -resize 256x256 icon_256x256.png

	# Create ico
	convert icon_256x256.png icon_128x128.png icon_48x48.png icon_32x32.png icon_16x16.png "$output"

	# Cleanup
	rm icon_256x256.png icon_128x128.png icon_48x48.png icon_32x32.png icon_16x16.png
}

create_icon "$commons_dir/ZC Logo & Icon/ZC_Icon_Small_Launcher.png" "$commons_dir/ZC Logo & Icon/ZC_Icon_Medium_Launcher.png" "$ROOT_DIR/zc_launcher_icon.ico"
create_icon "$commons_dir/ZC Logo & Icon/ZC_Icon_Small_Player.png" "$commons_dir/ZC Logo & Icon/ZC_Icon_Medium_Player.png" "$ROOT_DIR/zc_player_icon.ico"
create_icon "$commons_dir/ZC Logo & Icon/ZC_Icon_Small_Editor.png" "$commons_dir/ZC Logo & Icon/ZC_Icon_Medium_Editor.png" "$ROOT_DIR/zc_editor_icon.ico"
