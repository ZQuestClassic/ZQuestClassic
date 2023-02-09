#!/usr/bin/env bash

set -euxo pipefail

case "$(uname -sr)" in
   Darwin*)
        ./zelda "$@"
        ;;

   Linux*)
        sudo -E xvfb-run --auto-servernum ./zelda "$@"
        ;;

   CYGWIN*|MINGW*|MINGW32*|MSYS*)
        ./zelda.exe "$@"
        ;;

   *)
      exit 1
esac
