#!/usr/bin/env bash

set -euxo pipefail

echo '#define DEV_SIGNOFF "Build_Script"' > src/metadata/sigs/devsig.h.sig
echo '#define __TIMEZONE__ "UTC"' >> src/metadata/sigs/devsig.h.sig
echo '' >> src/metadata/sigs/devsig.h.sig

echo '' > src/metadata/sigs/compilersig.h.sig
if [ "$1" == 'msvc' ]; then
  echo '#define V_ZC_COMPILERSIG 1' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FIRST 19' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_SECOND 32' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_THIRD 31332' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_NAME "MSVC"' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_VERSION "v17, 2022"' >> src/metadata/sigs/compilersig.h.sig
elif [ "$1" == 'clang' ]; then
  echo '#define V_ZC_COMPILERSIG 1' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FIRST __clang_major__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_SECOND __clang_minor__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_THIRD __clang_patchlevel__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_NAME "clang"' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_VERSION __clang_version__' >> src/metadata/sigs/compilersig.h.sig
elif [ "$1" == 'gcc' ]; then
  # TODO: we don't use gcc in CI at the moment.
  exit 1
else
  echo "unknown compiler $1"
  exit 1
fi
echo '' >> src/metadata/sigs/compilersig.h.sig
