#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR/.."
echo "Generating docs-www/sources/zscript ..."
python scripts/generate_zscript_api_docs.py

cd "$SCRIPT_DIR"
echo "Building html docs ..."
make html

# old ghost docs
rm -rf build/html/_static/old/ghost
mkdir -p build/html/_static/old/ghost
cp -r ../resources/docs/ghost/{index.html,search.html,autoGhost,css,enemyScript,eweapon,fonts,globalScript,img,js,mkdocs,other,usage} build/html/_static/old/ghost

# old tango docs
rm -rf build/html/_static/old/tango
mkdir -p build/html/_static/old/tango
cp -r ../resources/docs/tango/{index.html,search.html,mkdocs,faq,css,fonts,js,img,tangoCode,zscriptFuncs,usage,errors,constants,dataFormats} build/html/_static/old/tango

rm -rf build/html/_sources
rm -rf build/html/_static/scripts/fontawesome.js
find build/html -name "*.map" -type f -delete
find build/html -name "*.ttf" -type f -delete
# for intersphinx. don't need, and don't now how to disable generation
rm build/html/objects.inv
