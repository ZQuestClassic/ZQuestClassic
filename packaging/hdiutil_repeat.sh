#!/usr/bin/env bash

# Workaround XProtect race condition for "hdiutil create" for MacOS
# https://gitlab.kitware.com/cmake/cmake/-/issues/25671
# https://github.com/HEXRD/hexrdgui/blob/master/packaging/hdiutil_repeat.sh

# BSD 3-Clause License

# Copyright (c) 2024, Kitware, Inc., Lawrence Livermore National Laboratory,
# and Air Force Research Laboratory.
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
#     list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.

# 3. Neither the name of the copyright holder nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set -e

# Try repeatedly, up to 30 times.
# This prevents spurious errors caused by a race condition with XProtect
# See https://github.com/actions/runner-images/issues/7522
i=0
args=("$@")

# Local edit: "hdiutil detach <mount point>" unmounts the volume, then ejects its disk. If the
# eject fails (busy), the volume is already gone, so retrying with the mount point can only fail
# with "No such file or directory". Find the disk now, and retry with that.
if [ "$1" = "detach" ] && [ -d "${!#}" ]; then
	disk=$(diskutil info "${!#}" | awk -F': *' '/Part of Whole/ {print $2}')
	if [ -n "$disk" ]; then
		retry_args=(detach "/dev/$disk")
	fi
fi

until
hdiutil "${args[@]}"
do
if [ $i -eq 30 ]; then exit 1; fi
i=$((i+1))
if [ -n "${retry_args+x}" ]; then
	args=("${retry_args[@]}")
fi
sleep 5
done
