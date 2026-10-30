#!/usr/bin/env python3
"""Checks that a Windows package only imports OS functions that Windows 7 has.

Windows won't start an exe if it, or a DLL it loads, imports a function the OS doesn't have.
On Windows 7 that shows up as "The application was unable to start correctly", before ZC (or
Sentry) runs any code, so it gets no crash report. Code that targets a newer Windows SDK can add
such an import without anyone noticing: sentry-native 0.17 imported WakeByAddressSingle
(Windows 8+) in 3.0.0-prerelease.232, and every exe stopped loading on Windows 7.

Every exe and DLL in the package is read, and each function it imports from a DLL that isn't
in the package is looked up in scripts/windows7_imports.txt, the list of OS imports known to
exist on Windows 7. Delay-loaded imports are skipped, since they only resolve when called.

If this fails for a new import, check the "Minimum supported client" in the function's
Microsoft docs. Both the function and the DLL must exist on Windows 7: an api-ms-win-core-*-l1-2-*
DLL is Windows 8+ even when the function itself is older. If they are, add the printed line to
windows7_imports.txt. Otherwise, load it at runtime with GetProcAddress and list the workaround
in docs/windows_support.md.

    python scripts/check_windows_imports.py <package .zip or folder>
"""

import argparse
import struct
import sys
import zipfile

from pathlib import Path

ALLOWLIST_PATH = Path(__file__).resolve().parent / 'windows7_imports.txt'

# The Universal CRT, which Windows 7 gets from the Visual C++ redistributable or Windows Update.
CRT_PREFIXES = ('api-ms-win-crt-',)


def read_imports(data: bytes) -> dict[str, set[str]]:
    """Returns the (non-delay-loaded) imports of a PE file, as {dll name: {function name}}.

    Functions imported by ordinal are named '#<ordinal>'. DLL names are lowercase.
    """
    pe_offset = struct.unpack_from('<I', data, 0x3C)[0]
    if data[pe_offset : pe_offset + 4] != b'PE\0\0':
        raise ValueError('not a PE file')
    coff = pe_offset + 4
    num_sections, optional_header_size = struct.unpack_from('<H12xH', data, coff + 2)
    optional_header = coff + 20
    magic = struct.unpack_from('<H', data, optional_header)[0]
    if magic == 0x10B:  # PE32
        data_directories = optional_header + 96
        thunk_format, ordinal_flag = '<I', 1 << 31
    elif magic == 0x20B:  # PE32+
        data_directories = optional_header + 112
        thunk_format, ordinal_flag = '<Q', 1 << 63
    else:
        raise ValueError(f'unknown optional header magic {magic:#x}')
    thunk_size = struct.calcsize(thunk_format)

    sections = []
    section_table = optional_header + optional_header_size
    for i in range(num_sections):
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
            '<IIII', data, section_table + i * 40 + 8
        )
        sections.append((virtual_address, max(virtual_size, raw_size), raw_offset))

    def offset(rva: int) -> int:
        for virtual_address, size, raw_offset in sections:
            if virtual_address <= rva < virtual_address + size:
                return rva - virtual_address + raw_offset
        raise ValueError(f'RVA {rva:#x} is not in any section')

    def c_string(rva: int) -> str:
        start = offset(rva)
        return data[start : data.index(b'\0', start)].decode('ascii')

    imports: dict[str, set[str]] = {}
    import_table_rva = struct.unpack_from('<I', data, data_directories + 8)[0]
    if not import_table_rva:
        return imports
    descriptor = offset(import_table_rva)
    while True:
        lookup_rva, _, _, name_rva, address_rva = struct.unpack_from(
            '<IIIII', data, descriptor
        )
        if not name_rva:
            break
        functions = imports.setdefault(c_string(name_rva).lower(), set())
        thunk = offset(lookup_rva or address_rva)
        while entry := struct.unpack_from(thunk_format, data, thunk)[0]:
            if entry & ordinal_flag:
                functions.add(f'#{entry & 0xFFFF}')
            else:
                # Skip the 2-byte hint before the name.
                functions.add(c_string((entry & 0x7FFFFFFF) + 2))
            thunk += thunk_size
        descriptor += 20
    return imports


def read_package(path: Path) -> dict[str, bytes]:
    """Returns {file name: contents} for every exe and DLL in a package .zip or folder."""
    if path.is_dir():
        files = {
            p.relative_to(path).as_posix(): p.read_bytes()
            for p in path.rglob('*')
            if p.is_file()
        }
    else:
        with zipfile.ZipFile(path) as z:
            files = {
                name: z.read(name) for name in z.namelist() if not name.endswith('/')
            }
    return {
        name: data
        for name, data in files.items()
        if name.lower().endswith(('.exe', '.dll'))
    }


def read_allowlist() -> set[tuple[str, str]]:
    allowed = set()
    for line in ALLOWLIST_PATH.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith('#'):
            dll, function = line.split()
            allowed.add((dll, function))
    return allowed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        'package', type=Path, help='package .zip, or a folder of its files'
    )
    args = parser.parse_args()

    files = read_package(args.package)
    if not files:
        print(f'no exe or DLL files in {args.package}')
        sys.exit(1)
    packaged_dlls = {Path(name).name.lower() for name in files}
    allowed = read_allowlist()

    # {(dll, function): [files importing it]}
    missing: dict[tuple[str, str], list[str]] = {}
    for name, data in sorted(files.items()):
        for dll, functions in read_imports(data).items():
            if dll in packaged_dlls or dll.startswith(CRT_PREFIXES):
                continue
            for function in functions:
                if (dll, function) not in allowed:
                    missing.setdefault((dll, function), []).append(name)

    if not missing:
        print(
            f'All OS imports of the {len(files)} exe and DLL files are in {ALLOWLIST_PATH.name}.'
        )
        return

    print(
        f'These OS imports are not in scripts/{ALLOWLIST_PATH.name}, so may not exist on Windows 7:\n'
    )
    for (dll, function), names in sorted(missing.items()):
        print(f'{dll} {function}    (imported by {", ".join(names)})')
    print(
        '\nWindows 7 refuses to start an exe that imports a function it does not have. Check the '
        '"Minimum supported client" in each function\'s Microsoft docs. If both the function and '
        'the DLL exist on Windows 7 (api-ms-win-core-*-l1-2-* DLLs are Windows 8+), add the line '
        f'above to scripts/{ALLOWLIST_PATH.name}. Otherwise, load it at runtime with '
        'GetProcAddress (see docs/windows_support.md).'
    )
    sys.exit(1)


if __name__ == '__main__':
    main()
