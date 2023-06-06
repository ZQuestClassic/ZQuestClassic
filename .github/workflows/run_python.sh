#!/usr/bin/env bash

set -euxo pipefail

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    sudo -E xvfb-run --auto-servernum python -Xutf8 "$@"
else
    python -Xutf8 "$@"
fi
