#!/usr/bin/env bash

set -euxo pipefail

case "$(uname -sr)" in
   Darwin*)
        ./zquest "$@"
        ;;

   Linux*)
        sudo -E xvfb-run --auto-servernum ./zquest "$@"
        ;;

   CYGWIN*|MINGW*|MINGW32*|MSYS*)
        ./zquest.exe "$@"
        ;;

   *)
      exit 1
esac
