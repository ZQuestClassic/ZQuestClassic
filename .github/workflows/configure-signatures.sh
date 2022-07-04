#!/usr/bin/env bash

set -euxo pipefail

echo '#define DEV_SIGNOFF "Build_Script"' > src/metadata/sigs/devsig.h.sig
echo '#define __TIMEZONE__ "UTC"' >> src/metadata/sigs/devsig.h.sig

if [ "$1" == 'win' ]; then
  echo '#define V_ZC_COMPILERSIG 1' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FIRST 19' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_SECOND 32' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_THIRD 31332' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_NAME "MSVC"' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_VERSION "v17, 2022"' >> src/metadata/sigs/compilersig.h.sig
elif [ "$1" == 'mac' ]; then
  echo '#define V_ZC_COMPILERSIG 1' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FIRST __clang_major__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_SECOND __clang_minor__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_THIRD __clang_patchlevel__' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_NAME "clang"' >> src/metadata/sigs/compilersig.h.sig
  echo '#define COMPILER_VERSION __clang_version__' >> src/metadata/sigs/compilersig.h.sig
else
  echo "unknown platform $1"
  exit 1
fi

if [ ! -z "${3:-}" ]; then
  echo '#undef V_ZC_ALPHA' >> src/metadata/versionsig.h
  echo '#undef V_ZC_BETA' >> src/metadata/versionsig.h
  echo '#undef V_ZC_GAMMA' >> src/metadata/versionsig.h
  echo '#undef V_ZC_RELEASE' >> src/metadata/versionsig.h
  
  if [ "$2" == "alpha" ]; then
    echo "#define V_ZC_ALPHA $3" >> src/metadata/versionsig.h
  else
    echo "#define V_ZC_ALPHA 0" >> src/metadata/versionsig.h
  fi
  
  if [ "$2" == "beta" ]; then
    echo "#define V_ZC_BETA $3" >> src/metadata/versionsig.h
  else
    echo "#define V_ZC_BETA 0" >> src/metadata/versionsig.h
  fi
  
  if [ "$2" == "gamma" ]; then
    echo "#define V_ZC_GAMMA $3" >> src/metadata/versionsig.h
  else
    echo "#define V_ZC_GAMMA 0" >> src/metadata/versionsig.h
  fi
  
  if [ "$2" == "release" ]; then
    echo "#define V_ZC_RELEASE $3" >> src/metadata/versionsig.h
  else
    echo "#define V_ZC_RELEASE 0" >> src/metadata/versionsig.h
  fi
fi

echo '#undef ZC_IS_NIGHTLY' >> src/metadata/versionsig.h
if [ "$4" == "false" ]; then
  echo '#define ZC_IS_NIGHTLY 0' >> src/metadata/versionsig.h
else
  echo '#define ZC_IS_NIGHTLY 1' >> src/metadata/versionsig.h
fi
