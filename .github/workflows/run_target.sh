#!/usr/bin/env bash

set -euxo pipefail

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT=$( dirname "$DIR" )/..

bash "$DIR/run_python.sh" "$ROOT/scripts/run_target.py" "$@"
