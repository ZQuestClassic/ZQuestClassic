# Merges two single-architecture mac builds into universal binaries.
#
# The universal mac release compiles the host architecture into `build` and the
# other one into a second build folder (a single configure with both archs in
# CMAKE_OSX_ARCHITECTURES is not supported - see "Universal (multi-arch) mac
# binaries" in the root CMakeLists.txt). This lipo-merges the second into the
# first, in place.
#
# Usage: python merge_mac_universal.py build/RelWithDebInfo build-x86_64/RelWithDebInfo

import re
import shutil
import subprocess
import sys

from pathlib import Path

root_dir = Path(__file__).parent.parent.parent.resolve()

# Thin Mach-O magic numbers (little-endian on disk), 64-bit and 32-bit.
MACHO_THIN_MAGICS = {b'\xcf\xfa\xed\xfe', b'\xce\xfa\xed\xfe'}


def run_cmd(cmd):
    cmd = [str(c) for c in cmd]
    print('+ ' + ' '.join(cmd), flush=True)
    subprocess.check_call(cmd)


def is_thin_macho(path: Path) -> bool:
    if path.is_symlink() or not path.is_file():
        return False
    try:
        with path.open('rb') as f:
            return f.read(4) in MACHO_THIN_MAGICS
    except OSError:
        return False


def strip_build_tree_rpaths(path: Path):
    """Remove LC_RPATH entries pointing into the build tree.

    The slices of a universal build carry rpaths naming their own build
    folders, and install_name_tool can only delete an rpath present in every
    slice of a fat file - so this must run on the thin binaries, before
    merging. (Otherwise fixup_bundle fails at package time when it tries to
    clean them up.) They are meaningless outside the build tree anyway; the
    packaging step adds the install rpath the app needs.
    """
    out = subprocess.check_output(['otool', '-l', str(path)], text=True)
    rpaths = set(re.findall(r'^\s+path (.+) \(offset \d+\)$', out, re.MULTILINE))
    for rpath in sorted(rpaths):
        if rpath.startswith(str(root_dir)):
            run_cmd(['install_name_tool', '-delete_rpath', rpath, path])


def merge_mac_binaries(primary_dir: Path, secondary_dir: Path):
    """lipo-merge every Mach-O in secondary_dir into its counterpart in primary_dir.

    This covers the executables and dylibs that get packaged, and also the
    debug info inside the .dSYM bundles next to them (dsymutil's DWARF
    companion files are Mach-O too), so the sentry debug info upload from
    primary_dir covers both architectures.
    """
    merged = []
    for path in sorted(primary_dir.rglob('*')):
        if not is_thin_macho(path):
            continue
        other = secondary_dir / path.relative_to(primary_dir)
        if not is_thin_macho(other):
            continue
        if not any(part.endswith('.dSYM') for part in path.parts):
            strip_build_tree_rpaths(path)
            strip_build_tree_rpaths(other)
        run_cmd(['lipo', '-create', path, other, '-output', path])
        merged.append(path)

    if not merged:
        sys.exit(f'No binaries to merge found in {primary_dir}')

    print(f'Created {len(merged)} universal binaries.')
    run_cmd(['lipo', '-info', merged[0]])

    # Some binaries exist for one architecture only: the script JIT is x86_64
    # only, so its library (asmjit) is only built for that slice. Copy those
    # over as they are - a single-architecture library is fine, as only that
    # slice of the programs asks for it.
    for other in sorted(secondary_dir.rglob('*')):
        if not is_thin_macho(other):
            continue
        path = primary_dir / other.relative_to(secondary_dir)
        if path.exists():
            continue
        if not any(part.endswith('.dSYM') for part in other.parts):
            strip_build_tree_rpaths(other)
        path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(other, path)
        print(f'Copied single-architecture binary: {path.relative_to(primary_dir)}')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(f'usage: {sys.argv[0]} <primary dir> <secondary dir>')
    merge_mac_binaries(Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve())
