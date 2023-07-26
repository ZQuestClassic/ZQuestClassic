#!/bin/bash

# Creates a mac application bundle.
# First, install these tools:
#     brew install dylibbundler create-dmg
#
# Does not need user input. When the Finder ZeldaClassic.app -> Applications window opens
# don't do anything - just wait.

set -e

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )

build_dir="${1:-${ROOT}/build/Release}"
packages_dir="$build_dir/packages"
package_dir="$packages_dir/zc"
mac_package_dir="$build_dir/packages/zelda-classic-mac"

set -eu
cd "$ROOT"

python scripts/package.py --build_folder "$build_dir" --skip_archive

# Set SKIP_APP_BUNDLE=1 to skip building an osx application bundle.
# This won't be able to distribute easily, because OSX will prevent users from running
# unverified binaries unless they right-click->Open and ignore a scary warning. Even then,
# when zlauncher/zquest opens other ZC processes OSX will prevent it without a way to ignore
# the intervention, so some features won't work.
if test "${SKIP_APP_BUNDLE+x}"; then
  rm -rf "$mac_package_dir"
  mv "$package_dir" "$mac_package_dir"
  echo "Done"
  exit
fi

# Prepare the Mac application bundle.
contents="$mac_package_dir/ZeldaClassic.app/Contents"

rm -rf "$mac_package_dir"
mkdir -p "$contents/MacOS"
cp "$DIR/Info.plist" "$contents"
mv "$package_dir/zlauncher" "$contents/MacOS"
mv "$package_dir" "$contents/Resources"

# Generate icon.
ICONDIR="$contents/Resources/icons.iconset"
ICON=resources/zc.png

mkdir "$ICONDIR"

# Normal screen icons
for size in 16 32 64 128 256 512; do
sips -z $size $size $ICON --out $ICONDIR/icon_${size}x${size}.png ;
done

# Retina display icons
for size in 32 64 256 512; do
sips -z $size $size $ICON --out "$ICONDIR"/icon_$(expr $size / 2)x$(expr $size / 2)x2.png ;
done

# Make a multi-resolution Icon
iconutil -c icns -o "$contents/Resources/icons.icns" "$ICONDIR"
rm -rf "$ICONDIR"

# Move shared libraries out of bundle (to be re-placed by dylibbundler)
tmp_libs_dir="$mac_package_dir/libs"
mkdir -p "$tmp_libs_dir"
find "$contents/Resources" -name "*.dylib" -exec mv {} "$tmp_libs_dir" \;

# Correct the library paths in the executable, and codesign.
dylibbundler -od -b -d "$contents/libs/" -s "$tmp_libs_dir" \
    -x "$contents/MacOS/zlauncher" -x "$contents/Resources/zquest" \
    -x "$contents/Resources/zelda" -x "$contents/Resources/zscript"
rm -rf "$tmp_libs_dir"

if test "${PACKAGE_DEBUG_INFO+x}"; then
  xcrun dsymutil \
    "$contents/MacOS/zlauncher" \
    "$contents/Resources/zelda" \
    "$contents/Resources/zquest" \
    "$contents/Resources/zscript" \
    $(find "$contents/libs" -name '*.dylib' -type f) \
    -o "$mac_package_dir/ZeldaClassic.app.dSYM"
fi

cd "$packages_dir"
rm -f ZeldaClassic.dmg
create-dmg \
  --volname "ZeldaClassic" \
  --volicon "$contents/Resources/icons.icns" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "ZeldaClassic.app" 200 190 \
  --hide-extension "ZeldaClassic.app" \
  --app-drop-link 600 185 \
  ZeldaClassic.dmg \
  "$mac_package_dir"

echo "Done!"
