#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"
rm -rf include LICENSE.MIT

# v3.11.3
python3 ../update_dependency.py https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz json_temp

cp -r json_temp/include .
cp json_temp/LICENSE.MIT .

rm -rf json_temp
