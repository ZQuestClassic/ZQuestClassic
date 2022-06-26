#!/bin/sh
src="../.."
out="${src}/output"
log="${out}/_auto/buildpack_log.txt"
nb="${out}/_auto/buildpack"
rel="${1:-${src}/build}"
incl="${out}/include"
docs="${out}/docs"
theme="${out}/themes"
tsets="${out}/tilesets"
util="${out}/utilities"
conf="${out}/config"
mod="${out}/modules"
pack="${out}/package"

if [ -d "$nb" ]; then
	echo "Clearing old buildpack folder..."
	echo "Clearing old buildpack folder...">"$log"
    rm -r "$nb"
    mkdir "$nb"
else
    mkdir "$nb"
fi

echo "Copying '/output/package' files..."
echo "Copying '/output/package' files...">"$log"
cp -r "${pack}/"* "$nb">>"$log"

echo "Copying '/Release' - zelda, zquest, zscript, zlauncher, libzcsound.so..."
echo "Copying '/Release' - zelda, zquest, zscript, zlauncher, libzcsound.so...">>"$log"
cp "${rel}/zelda" "${nb}" >>"$log"
cp "${rel}/zquest" "${nb}" >>"$log"
cp "${rel}/zscript" "${nb}" >>"$log"
cp "${rel}/zlauncher" "${nb}" >>"$log"
cp "${rel}/libzcsound.so" "${nb}" >>"$log"

echo "Copying 'changelog.txt'..."
echo "Copying 'changelog.txt'...">>"${log}"
cp "${src}/changelog.txt" "${nb}" >>"${log}"

echo "Copying '/output/include'..."
echo "Copying '/output/include'...">>"${log}"
mkdir "${nb}/include"
cp -r "${incl}/"* "${nb}/include" >>"${log}"

echo "Copying '/output/docs'..."
echo "Copying '/output/docs'...">>"${log}"
#mkdir "${nb}/docs"
cp -r "${docs}/"* "${nb}/docs" >>"${log}"

echo "Copying '/output/themes'..."
echo "Copying '/output/themes'...">>"${log}"
mkdir "${nb}/themes"
cp -r "${theme}/"* "${nb}/themes" >>"${log}"

echo "Copying '/output/tilesets'..."
echo "Copying '/output/tilesets'...">>"${log}"
mkdir "${nb}/tilesets"
cp -r "${tsets}/"* "${nb}/tilesets" >>"${log}"

echo "Copying '/output/utilities'..."
echo "Copying '/output/utilities'...">>"${log}"
mkdir "${nb}/utilities"
cp -r "${util}/"* "${nb}/utilities" >>"${log}"

echo "Copying '/output/configs'..."
echo "Copying '/output/configs'...">>"${log}"
mkdir "${nb}/configs"
cp -r "${conf}/"* "${nb}" >>"${log}"

echo "Copying '/output/modules'..."
echo "Copying '/output/modules'...">>"${log}"
mkdir "${nb}/modules"
cp -r "${mod}/"* "${nb}/modules" >>"${log}"
