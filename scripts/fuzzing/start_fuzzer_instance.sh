#!/bin/bash

if [[ ! -n "${IN_DOCKER}" ]]; then
    docker exec -e IN_DOCKER=1 -it zc-fuzz bash /src/scripts/fuzzing/start_fuzzer_instance.sh "$@"
    exit
fi

set -ex

cd /src/build_fuzz
afl-fuzz "$@"
