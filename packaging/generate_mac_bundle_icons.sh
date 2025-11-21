DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )

DEST="$1"
ICONDIR="icons.iconset"
ICON=resources/assets/zc/ZC_Icon_Medium_Launcher.png

cd "$ROOT"
mkdir "$ICONDIR"

# Normal screen icons
for size in 16 32 64 128 256 512; do
sips -z $size $size $ICON --out "$ICONDIR/icon_${size}x${size}.png" ;
done

# Retina display icons
for size in 32 64 256 512; do
sips -z $size $size $ICON --out "$ICONDIR"/icon_$(expr $size / 2)x$(expr $size / 2)x2.png ;
done

# Make a multi-resolution Icon
iconutil -c icns -o "$DEST" "$ICONDIR"
rm -rf "$ICONDIR"
