#!/bin/sh

# Same as buildpack.sh, but in the end creates a mac application bundle.
# First, install this tool:
#     brew install dylibbundler

src="../.."
out="${src}/output"
nb="${out}/_auto/buildpack"
mac_nb="${out}/_auto/zelda-classic-mac"
rel="${src}/build"

sh buildpack.sh
set -eu

# Change some defaults.
sed -i -e 's/fullscreen = 0/fullscreen = 1/' "$nb/zquest.cfg"
sed -i -e 's/fullscreen = 0/fullscreen = 1/' "$nb/zc.cfg"
find "$nb" -name "*.cfg-e" -exec rm {} \;

# Delete things Mac build won't need.
# TODO: better system for defining what files are bundled for each platform.
find "$nb" -name "*.exe" -exec rm {} \;
find "$nb" -name "*.dll" -exec rm {} \;
rm -rf "$nb/Addons"
rm -rf "$nb/utilities"

# Prepare the Mac application bundle.
contents="$mac_nb/ZeldaClassic.app/Contents"

rm -rf "$mac_nb"
mkdir -p "$contents/MacOS"
cp Info.plist "$contents"
mv buildpack/zlauncher "$contents/MacOS"
mv buildpack "$contents/Resources"
find "$rel" -name "*.dylib" -exec cp {} "$contents/Resources" \;

# Generate icon.
ICONDIR="$contents/Resources/icons.iconset"
ICON=../zc.png

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
tmp_libs_dir="$mac_nb/libs"
mkdir -p "$tmp_libs_dir"
find "$contents/Resources" -name "*.dylib" -exec mv {} "$tmp_libs_dir" \;

# Correct the library paths in the executable, and codesign.
dylibbundler -od -b -d "$contents/libs/" -s "$tmp_libs_dir" \
    -x "$contents/MacOS/zlauncher" -x "$contents/Resources/zquest" \
    -x "$contents/Resources/zelda" -x "$contents/Resources/zscript"
rm -rf "$tmp_libs_dir"

echo "Done!"
