#!/usr/bin/env bash

set -euxo pipefail

echo '#define __TIMEZONE__ "UTC"' > src/metadata/metadata.h
echo '' >> src/metadata/metadata.h

if [ "$1" == 'msvc' ]; then
  echo '#define COMPILER_V_FIRST (_MSC_VER/100)' >> src/metadata/metadata.h
  echo '#define COMPILER_V_SECOND (_MSC_VER%100)' >> src/metadata/metadata.h
  echo '#define COMPILER_V_THIRD (_MSC_FULL_VER%100000)' >> src/metadata/metadata.h
  echo '#define COMPILER_V_FOURTH _MSC_BUILD' >> src/metadata/metadata.h
  echo '#define COMPILER_NAME "MSVC"' >> src/metadata/metadata.h
  echo '#define METADATA_IMPL_STRINGIFY(x) #x' >> src/metadata/metadata.h
  echo '#define COMPILER_VERSION METADATA_IMPL_STRINGIFY(_MSC_FULL_VER)' >> src/metadata/metadata.h
  echo '' >> src/metadata/metadata.h

  # Only used by .rc files.
  num=$(python -c 'import sys, re; print(re.match("(\\d+)\\.(\\d+)\\.(\\d+)", sys.argv[1])[1])' $2)
  echo "#define V_ZC_FIRST $num" >> src/metadata/metadata.h
  num=$(python -c 'import sys, re; print(re.match("(\\d+)\\.(\\d+)\\.(\\d+)", sys.argv[1])[2])' $2)
  echo "#define V_ZC_SECOND $num" >> src/metadata/metadata.h
  num=$(python -c 'import sys, re; print(re.match("(\\d+)\\.(\\d+)\\.(\\d+)", sys.argv[1])[3])' $2)
  echo "#define V_ZC_THIRD $num" >> src/metadata/metadata.h
  num=$(python -c 'import sys, re; r = re.match(".*prerelease\\.(\\d+)", sys.argv[1]); print(r[1] if r else 0)' $2)
  echo "#define V_ZC_FOURTH $num" >> src/metadata/metadata.h
elif [ "$1" == 'clang' ]; then
  echo '#define COMPILER_V_FIRST __clang_major__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_SECOND __clang_minor__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_THIRD __clang_patchlevel__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/metadata.h
  echo '#define COMPILER_NAME "clang"' >> src/metadata/metadata.h
  echo '#define COMPILER_VERSION __clang_version__' >> src/metadata/metadata.h
elif [ "$1" == 'gcc' ]; then
  echo '#define COMPILER_V_FIRST __GNUC__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_SECOND __GNUC_MINOR__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_THIRD __GNUC_PATCHLEVEL__' >> src/metadata/metadata.h
  echo '#define COMPILER_V_FOURTH 0' >> src/metadata/metadata.h
  echo '#define COMPILER_NAME "gcc"' >> src/metadata/metadata.h
  echo '#define COMPILER_VERSION __VERSION__' >> src/metadata/metadata.h
else
  echo "unknown compiler $1"
  exit 1
fi
echo '' >> src/metadata/metadata.h
