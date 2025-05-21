#!/usr/bin/env bash

set -euxo pipefail

case "$(uname -sr)" in
   Darwin*)
        ulimit -c unlimited
        sudo -E python -Xutf8 "$@"
        ;;

   Linux*)
        ulimit -c unlimited
        xvfb-run --auto-servernum python -Xutf8 "$@"
        ;;

   CYGWIN*|MINGW*|MINGW32*|MSYS*)
        python -Xutf8 "$@"
        ;;

   *)
      exit 1
esac
