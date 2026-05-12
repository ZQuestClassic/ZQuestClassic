"""
ZScript Version Linter
======================

This script identifies ZScript symbols (functions, variables, classes, enums) that:
1. Were added in ZC 3.0 but are missing the required `@versionadded 3.0` tag.
2. Are incorrectly marked with `@versionadded 3.0` but actually existed in ZC 2.55.

How it works:
------------
1.  **Extract 2.55 Symbols:**
    -   **Built-ins:** Parses `AccessorTable` arrays from 2.55 C++ source files (on the `releases/2.55` branch).
        It maps legacy internal names (like `getX`, `setX`, `_getAllocateBitmap`) to 3.0 property names.
    -   **Headers:** Scans all `.zh` files in 2.55 `resources/include` and `resources/headers` for
        global constants, variables, and functions.
2.  **Extract 3.0 Symbols:**
    -   Uses `zscript_doc_parser.py` (the same engine as the API doc generator) to get a full tree
        of current symbols and their documentation tags.
3.  **Comparison Logic:**
    -   **Arity-Aware:** Functions are differentiated by their parameter counts. A function like
        `WarpEx(int[])` (arity 1) will match 2.55, but `WarpEx(WarpType, ...)` (arity 9) will be
        flagged as new.
    -   **Inheritance:** Base class members (like `sprite->X`) are correctly attributed even when
        accessed through derived classes (like `Hero->X`).
    -   **Shorthand Mapping:** Correctly handles `_get`, `_set`, `const`, and `[]` internal naming conventions.
4.  **Suppression Rules:**
    -   **Parent Inheritance:** If a Class or Enum is tagged with `@versionadded 3.0`, all its members
        are assumed to inherit that version and will not be flagged individually.
    -   **`sprite` Exception:** Members of the `sprite` class are ALWAYS checked individually, even if
        the class is tagged, due to the major architectural shift it represents.
    -   **Enum Types:** The enum type itself (e.g., `AimType`) is never flagged; only its members are checked.
    -   **Collapsing:** If every single member of a class or named enum is new and missing a tag, only the class/enum
        itself is flagged to reduce noise.
5.  **Auto-Fix Mode (`--fix`):**
    -   Automatically adds missing `@versionadded 3.0` tags.
    -   Correctly formats comments: moves same-line comments to a block above and ensures proper spacing.
    -   Preserves indentation (tabs/spaces) and ensures tags appear before other ZASM-related doc tags.

Usage:
------
    # Check for versioning issues
    python3 scripts/lint_zscript_versions.py

    # Automatically fix missing tags
    python3 scripts/lint_zscript_versions.py --fix
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from dataclasses import dataclass, field

# Add current scripts directory to sys.path to import zscript_doc_parser
script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
sys.path.append(str(script_dir))

import zscript_doc_parser as parser


import urllib.request


@dataclass
class VersionInfo:
    added: str = None
    changed: str = None


class ContentProvider:
    def list_files(self, dir_path):
        raise NotImplementedError()

    def get_file(self, file_path):
        raise NotImplementedError()


class GitProvider(ContentProvider):
    def __init__(self, ref):
        self.ref = ref

    def list_files(self, dir_path):
        try:
            output = subprocess.check_output(
                ['git', 'ls-tree', '-r', '--name-only', self.ref, dir_path],
                encoding='utf-8',
                stderr=subprocess.DEVNULL,
            )
            return output.splitlines()
        except subprocess.CalledProcessError:
            return []

    def get_file(self, file_path):
        try:
            return subprocess.check_output(
                ['git', 'show', f'{self.ref}:{file_path}'],
                encoding='utf-8',
                stderr=subprocess.DEVNULL,
            )
        except subprocess.CalledProcessError:
            return None


class HttpProvider(ContentProvider):
    def __init__(self, branch):
        self.branch = branch
        self.base_url = f"https://raw.githubusercontent.com/ZQuestClassic/ZQuestClassic/refs/heads/{branch}"
        self.tree = None

    def _get_tree(self):
        if self.tree is not None:
            return self.tree
        api_url = f"https://api.github.com/repos/ZQuestClassic/ZQuestClassic/git/trees/{self.branch}?recursive=1"
        try:
            with urllib.request.urlopen(api_url) as response:
                data = json.loads(response.read().decode())
                self.tree = [item['path'] for item in data.get('tree', [])]
        except Exception as e:
            print(f"Warning: Failed to fetch tree from GitHub API: {e}")
            self.tree = []
        return self.tree

    def list_files(self, dir_path):
        tree = self._get_tree()
        # Ensure dir_path ends with / for correct matching
        prefix = dir_path if dir_path.endswith('/') else dir_path + '/'
        return [f for f in tree if f.startswith(prefix)]

    def get_file(self, file_path):
        url = f"{self.base_url}/{file_path}"
        try:
            with urllib.request.urlopen(url) as response:
                return response.read().decode('utf-8')
        except Exception:
            return None


def get_content_provider():
    # Check for remote or local git branches.
    potential_refs = ['upstream/releases/2.55', 'origin/releases/2.55', 'releases/2.55']
    for ref in potential_refs:
        try:
            subprocess.check_output(
                ['git', 'rev-parse', '--verify', ref], stderr=subprocess.DEVNULL
            )
            # If it's a remote ref, try to fetch it to ensure we have it locally
            if ref.startswith('origin/') or ref.startswith('upstream/'):
                remote, branch = ref.split('/', 1)
                print(f"Fetching {branch} from {remote}...")
                subprocess.run(
                    ['git', 'fetch', remote, branch],
                    stderr=subprocess.DEVNULL,
                    stdout=subprocess.DEVNULL,
                )
            return GitProvider(ref)
        except subprocess.CalledProcessError:
            continue

    print("No git branch found for 2.55, falling back to HTTP...")
    return HttpProvider('releases/2.55')


def parse_255_builtins(provider):
    """
    Parses built-in symbols from 2.55 C++ source files.
    Returns a dict mapping class name to a dict of name -> list of (min_arity, max_arity, is_varg).
    Properties are stored as arity-less names.
    """
    # Mapping of ZTID constant to class name
    ztid_to_name = {
        'ZTID_AUDIO': 'Audio',
        'ZTID_BITMAP': 'bitmap',
        'ZTID_BOTTLESHOP': 'bottleshopdata',
        'ZTID_BOTTLETYPE': 'bottledata',
        'ZTID_CHEATS': 'cheats',
        'ZTID_COMBOS': 'combodata',
        'ZTID_COMBOTRIGGER': 'combotrigger',
        'ZTID_DEBUG': 'Debug',
        'ZTID_DIRECTORY': 'directory',
        'ZTID_DMAPDATA': 'dmapdata',
        'ZTID_DROPSET': 'dropsetdata',
        'ZTID_EWPN': 'eweapon',
        'ZTID_FFC': 'ffc',
        'ZTID_FILE': 'file',
        'ZTID_FILESYSTEM': 'FileSystem',
        'ZTID_GAME': 'Game',
        'ZTID_GAMEDATA': 'gamedata',
        'ZTID_GENERICDATA': 'genericdata',
        'ZTID_GRAPHICS': 'Graphics',
        'ZTID_INPUT': 'Input',
        'ZTID_ITEM': 'itemsprite',
        'ZTID_ITEMCLASS': 'itemdata',
        'ZTID_LWPN': 'lweapon',
        'ZTID_MAPDATA': 'mapdata',
        'ZTID_NPC': 'npc',
        'ZTID_NPCDATA': 'npcdata',
        'ZTID_PALCYCLE': 'palcycle',
        'ZTID_PALDATA': 'paldata',
        'ZTID_PLAYER': 'Hero',
        'ZTID_PORTAL': 'portal',
        'ZTID_REGION': 'Region',
        'ZTID_RNG': 'randgen',
        'ZTID_SAVPORTAL': 'savedportal',
        'ZTID_SCREEN': 'Screen',
        'ZTID_SHOPDATA': 'shopdata',
        'ZTID_SPRITEDATA': 'spritedata',
        'ZTID_STACK': 'stack',
        'ZTID_SUBSCREENDATA': 'subscreendata',
        'ZTID_SUBSCREENPAGE': 'subscreenpage',
        'ZTID_SUBSCREENWIDGET': 'subscreenwidget',
        'ZTID_TEXT': 'Text',
        'ZTID_VIEWPORT': 'Viewport',
        'ZTID_ZINFO': 'zinfo',
        'ZTID_ZMESSAGE': 'messagedata',
    }

    class_symbols = {}

    # Get all .cpp files in 2.55 symbols directory
    symbol_files = provider.list_files('src/parser/symbols')
    symbol_files = [f for f in symbol_files if f.endswith('.cpp')]

    for filepath in symbol_files:
        filename = filepath.split('/')[-1]
        content = provider.get_file(filepath)
        if not content:
            continue

        # Accessor table entries:
        # { "name", tag, rettype, var, flags, {params}, {opts} }
        pattern = r'\{ "([^"]*)",\s*[^,]+,\s*[^,]+,\s*[^,]+,\s*([^,]+),\s*\{([^}]*)\},\s*\{([^}]*)\}'
        matches = re.findall(pattern, content)

        is_global_file = filename == 'GlobalSymbols.cpp'

        for name, flags, params_str, opts_str in matches:
            if not name:
                continue

            params = [p.strip() for p in params_str.split(',') if p.strip()]
            opts = [p.strip() for p in opts_str.split(',') if p.strip()]

            # Determine class name from first parameter (the object reference)
            if is_global_file:
                class_name = 'Global'
                prefix = 0
            elif params:
                ztid = params[0]
                class_name = ztid_to_name.get(ztid, 'Unknown')
                prefix = 1
            else:
                class_name = 'Unknown'
                prefix = 0

            if class_name not in class_symbols:
                class_symbols[class_name] = {}

            real_params_count = max(0, len(params) - prefix)
            max_arity = real_params_count
            min_arity = max(0, real_params_count - len(opts))
            is_varg = 'FL_VARG' in flags

            if name not in class_symbols[class_name]:
                class_symbols[class_name][name] = []

            class_symbols[class_name][name].append((min_arity, max_arity, is_varg))

            # Special case: 2.55 source has a bug in GameSymbols.cpp where
            # LoadTempScreenForComboPos is listed as arity 1 in the table, but
            # the handler code pops twice and comments say it takes 2 args.
            if class_name == 'Game' and name == 'LoadTempScreenForComboPos':
                class_symbols[class_name][name].append((2, 2, False))

            # Handle property mappings (getX -> X, _getX -> X, getX[] -> X)
            clean_name = name
            if clean_name.endswith('[]'):
                clean_name = clean_name[:-2]

            prop_name = None
            if (
                clean_name.startswith('_get')
                and len(clean_name) > 4
                and clean_name[4].isupper()
            ):
                prop_name = clean_name[4:]
            elif (
                clean_name.startswith('_set')
                and len(clean_name) > 4
                and clean_name[4].isupper()
            ):
                prop_name = clean_name[4:]
            elif (
                clean_name.startswith('get')
                and len(clean_name) > 3
                and clean_name[3].isupper()
            ):
                prop_name = clean_name[3:]
            elif (
                clean_name.startswith('set')
                and len(clean_name) > 3
                and clean_name[3].isupper()
            ):
                prop_name = clean_name[3:]
            elif (
                clean_name.startswith('const')
                and len(clean_name) > 5
                and clean_name[5].isupper()
            ):
                prop_name = clean_name[5:]

            if prop_name:
                if prop_name not in class_symbols[class_name]:
                    class_symbols[class_name][prop_name] = []
                # Both arity-less and with current arity
                class_symbols[class_name][prop_name].append(None)
                class_symbols[class_name][prop_name].append(
                    (min_arity, max_arity, is_varg)
                )

            if name.endswith('[]'):
                alt_name = name[:-2]
                if alt_name not in class_symbols[class_name]:
                    class_symbols[class_name][alt_name] = []
                class_symbols[class_name][alt_name].append(None)

    # Implicit globals
    if 'Global' not in class_symbols:
        class_symbols['Global'] = {}
    for g in [
        'Audio',
        'bitmap',
        'bottledata',
        'bottleshopdata',
        'combodata',
        'Debug',
        'directory',
        'dmapdata',
        'dropsetdata',
        'eweapon',
        'ffc',
        'file',
        'FileSystem',
        'Game',
        'genericdata',
        'Graphics',
        'Hero',
        'Input',
        'itemdata',
        'itemsprite',
        'Link',
        'lweapon',
        'mapdata',
        'messagedata',
        'Module',
        'npc',
        'npcdata',
        'paldata',
        'Player',
        'portal',
        'randgen',
        'RandGen',
        'Region',
        'savedportal',
        'Screen',
        'shopdata',
        'spritedata',
        'stack',
        'subscreendata',
        'subscreenpage',
        'subscreenwidget',
        'Tango',
        'Text',
        'Viewport',
        'ZInfo',
    ]:
        if g not in class_symbols['Global']:
            class_symbols['Global'][g] = []
        class_symbols['Global'][g].append(None)

    return class_symbols


def parse_255_headers(provider):
    """
    Parses global symbols from all 2.55 header files.
    Returns a dict of name -> set of arities.
    """
    symbols = {}

    headers = provider.list_files('resources/include') + provider.list_files(
        'resources/headers'
    )
    headers = [
        h
        for h in headers
        if h.endswith('.zh') or h.endswith('.zs') or h.endswith('.cfg')
    ]

    for h in headers:
        content = provider.get_file(h)
        if not content:
            continue

        # Strip all comments first
        content = re.sub(r'//.*', '', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)

        # Strip function bodies to avoid matching local variables/params
        # But KEEP namespace and enum blocks
        def strip_bodies(text):
            res = []
            start = 0
            i = 0
            while i < len(text):
                if text[i] == '{':
                    # Check if it's a function (follows ')')
                    pre = text[max(0, i - 50) : i].strip()
                    if pre.endswith(')'):
                        res.append(text[start:i])
                        # Find matching }
                        stack = 1
                        i += 1
                        while i < len(text) and stack > 0:
                            if text[i] == '{':
                                stack += 1
                            elif text[i] == '}':
                                stack -= 1
                            i += 1
                        start = i
                        continue
                i += 1
            res.append(text[start:])
            return ''.join(res)

        # First, find enum blocks and extract their members
        enum_blocks = re.findall(r'enum\b[^{]*\{([^}]*?)\}', content, re.DOTALL)
        for block in enum_blocks:
            for part in re.split(r'[,\n;]', block):
                part = part.strip()
                if not part:
                    continue
                m = re.match(r'\b(\w+)\b', part)
                if m:
                    name = m.group(1)
                    if not name[0].isdigit():
                        if name not in symbols:
                            symbols[name] = set()
                        symbols[name].add(None)

        content = strip_bodies(content)

        # Constants - handle 'const type A = 1, B = 2;' or 'define/DEFINE A = 1, B = 2;'
        # We find the prefix, then all assignments until the semicolon
        const_lines = re.findall(
            r'(?:const\s+\w+|define|DEFINE[A-Z]?|CONFIG[A-Z]?)\s+([^;]+);', content
        )
        for line in const_lines:
            for m in re.findall(r'(\w+)\s*=', line):
                if m not in symbols:
                    symbols[m] = set()
                symbols[m].add(None)

        # Global Functions
        functions = re.findall(r'(?:\w+)\s+(\w+)\s*\(([^)]*)\)', content)
        for name, params_str in functions:
            if name in [
                'if',
                'while',
                'for',
                'switch',
                'return',
                'enum',
                'const',
                'typedef',
                'import',
                'using',
            ]:
                continue
            params = [p.strip() for p in params_str.split(',') if p.strip()]
            max_arity = len(params)
            min_arity = len([p for p in params if '=' not in p])
            if name not in symbols:
                symbols[name] = set()
            for a in range(min_arity, max_arity + 1):
                symbols[name].add(a)

        # Global variables
        variables = re.findall(r'(?:\w+)\s+(\w+)\s*;', content)
        for v in variables:
            if v in [
                'return',
                'else',
                'while',
                'if',
                'for',
                'switch',
                'break',
                'continue',
            ]:
                continue
            if v not in symbols:
                symbols[v] = set()
            symbols[v].add(None)

    return symbols


def build_255_globals_map(headers_255, builtins_255):
    all_global_names_255_lower = {}
    for name, arities in headers_255.items():
        ln = name.lower()
        if ln not in all_global_names_255_lower:
            all_global_names_255_lower[ln] = set()
        all_global_names_255_lower[ln].update(arities)

    for name, entries in builtins_255.get('Global', {}).items():
        ln = name.lower()
        if ln not in all_global_names_255_lower:
            all_global_names_255_lower[ln] = set()
        for entry in entries:
            if entry is None:
                all_global_names_255_lower[ln].add(None)
            else:
                # Store arity range start
                all_global_names_255_lower[ln].add(entry[0])
    return all_global_names_255_lower


def get_symbol_status(
    symbol,
    class_obj,
    builtins_255,
    headers_255,
    all_global_names_255_lower,
):
    name = symbol.name
    if name.startswith('__'):
        return None

    if isinstance(symbol, parser.Function):
        num_params = len(symbol.parameters.variables)
        if symbol.parameters.varargs:
            min_arity = num_params - 1
            max_arity = 999
        else:
            min_arity = len([v for v in symbol.parameters.variables if v.value is None])
            max_arity = num_params
        symbol_arity_range = (min_arity, max_arity, symbol.parameters.varargs)
    else:
        symbol_arity_range = (None, None, False)

    def existed_in_255(cname, name, arity_range):
        if cname in builtins_255 and name in builtins_255[cname]:
            for entry in builtins_255[cname][name]:
                if entry is None:
                    if arity_range[0] is None or arity_range[0] == 0:
                        return True
                    continue
                min_a, max_a, is_v = entry
                max_a_eff = 999 if is_v else max_a
                if arity_range[0] is not None:
                    if max(arity_range[0], min_a) <= min(arity_range[1], max_a_eff):
                        return True
                elif min_a == 0:
                    return True
        if not cname or cname == 'Global':
            if name in headers_255:
                arities = headers_255[name]
                if None in arities and arity_range[0] is None:
                    return True
                if arity_range[0] is not None:
                    for a in range(arity_range[0], min(arity_range[1], 50) + 1):
                        if a in arities:
                            return True
        return False

    found_in_255 = False
    if class_obj:
        curr = class_obj
        while curr:
            cnames = [curr.name]
            if curr.name == 'Hero':
                cnames.append('hero')
            for cname in cnames:
                if existed_in_255(cname, name, symbol_arity_range):
                    found_in_255 = True
                    break
            if found_in_255:
                break
            if len(name) > 6 and existed_in_255('Global', name, symbol_arity_range):
                found_in_255 = True
                break
            curr = curr.parent if hasattr(curr, 'parent') else None
    else:
        if existed_in_255('Global', name, symbol_arity_range):
            found_in_255 = True

    if not found_in_255:
        ln = name.lower()
        if ln in all_global_names_255_lower:
            arities = all_global_names_255_lower[ln]
            if not class_obj or len(name) > 8:
                if (None in arities and symbol_arity_range[0] is None) or (
                    symbol_arity_range[0] in arities
                ):
                    found_in_255 = True

    has_version = False
    version_added = None
    if symbol.comment:
        if symbol.comment.has_tag('versionadded') or symbol.comment.has_tag(
            'versionchanged'
        ):
            has_version = True
        version_added = symbol.comment.get_tag_single('versionadded')

    return {
        'name': name,
        'class_name': class_obj.name if class_obj else None,
        'is_new': not found_in_255,
        'has_version': has_version,
        'version_added': version_added,
        'loc': symbol.loc,
        'found': found_in_255,
    }


def find_version_tag_issues(
    all_files, builtins_255, headers_255, all_global_names_255_lower
):
    missing_tags = []
    incorrect_tags = []

    def get_status(symbol, class_obj=None):
        return get_symbol_status(
            symbol,
            class_obj,
            builtins_255,
            headers_255,
            all_global_names_255_lower,
        )

    def walk_scope(scope, class_obj=None, ns_prefix=""):
        for ns in getattr(scope, 'namespaces', []):
            new_prefix = f"{ns_prefix}{ns.name}::" if ns.name else ns_prefix
            walk_scope(ns, class_obj, new_prefix)

        for cls in getattr(scope, 'classes', []):
            status = get_status(cls, class_obj)
            if not status:
                continue

            if status['found'] and status['version_added'] == '3.0':
                incorrect_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )

            if status['is_new'] and not status['has_version']:
                if cls.name == 'sprite':
                    missing_tags.append(
                        (
                            class_obj.name if class_obj else None,
                            f"{ns_prefix}{status['name']}",
                            status['loc'],
                        )
                    )
                    walk_scope(cls, cls, ns_prefix)
                    continue

                def collect_members(s, c):
                    res = []
                    for fn in getattr(s, 'functions', []):
                        st = get_status(fn, c)
                        res.append(st) if st else None
                    for var in getattr(s, 'variables', []):
                        st = get_status(var, c)
                        res.append(st) if st else None
                    for enm in getattr(s, 'enums', []):
                        for m in enm.members:
                            st = get_status(m, c)
                            res.append(st) if st else None
                    return res

                member_statuses = collect_members(cls, cls)
                all_missing = all(
                    ms.get('is_new', False) and not ms.get('has_version', False)
                    for ms in member_statuses
                )
                missing_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )
                if not (member_statuses and all_missing):
                    walk_scope(cls, cls, ns_prefix)
            else:
                if not (status['is_new'] and status['has_version']):
                    walk_scope(cls, cls, ns_prefix)

        for fn in getattr(scope, 'functions', []):
            status = get_status(fn, class_obj)
            if not status:
                continue
            if status['found'] and status['version_added'] == '3.0':
                incorrect_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )
            if status.get('is_new') and not status.get('has_version'):
                missing_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )

        for var in getattr(scope, 'variables', []):
            status = get_status(var, class_obj)
            if not status:
                continue
            if status['found'] and status['version_added'] == '3.0':
                incorrect_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )
            if status.get('is_new') and not status.get('has_version'):
                missing_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )

        for enm in getattr(scope, 'enums', []):
            status = get_status(enm, class_obj)
            if status and status['found'] and status['version_added'] == '3.0':
                incorrect_tags.append(
                    (
                        class_obj.name if class_obj else None,
                        f"{ns_prefix}{status['name']}",
                        status['loc'],
                    )
                )

            member_statuses = [get_status(m, class_obj) for m in enm.members]
            member_statuses = [ms for ms in member_statuses if ms]

            for ms in member_statuses:
                if ms['found'] and ms['version_added'] == '3.0':
                    incorrect_tags.append(
                        (
                            class_obj.name if class_obj else None,
                            f"{ns_prefix}{ms['name']}",
                            ms['loc'],
                        )
                    )

            all_missing = all(
                ms.get('is_new', False) and not ms.get('has_version', False)
                for ms in member_statuses
            )
            if not (status and status.get('has_version')):
                if member_statuses and all_missing and not enm.is_anonymous():
                    missing_tags.append(
                        (
                            class_obj.name if class_obj else None,
                            f"{ns_prefix}{status['name']}",
                            status['loc'],
                        )
                    )
                else:
                    for member in enm.members:
                        st = get_status(member, class_obj)
                        if st and st.get('is_new') and not st.get('has_version'):
                            missing_tags.append(
                                (
                                    class_obj.name if class_obj else None,
                                    f"{ns_prefix}{st['name']}",
                                    st['loc'],
                                )
                            )

    for file in all_files:
        walk_scope(file)

    return missing_tags, incorrect_tags


def apply_fixes(missing_tags):
    print(f"Applying fixes to {len(missing_tags)} symbols...")
    # Sort by file and then line (descending) to avoid line number shifts
    missing_tags.sort(key=lambda x: (x[2].file, -x[2].line))

    current_file = None
    lines = []

    for cls_name, sym_name, loc in missing_tags:
        abs_path = root_dir / loc.file
        if str(abs_path) != current_file:
            if current_file:
                Path(current_file).write_text('\n'.join(lines) + '\n')
            current_file = str(abs_path)
            if not abs_path.exists():
                print(f"Warning: File {loc.file} not found for fix.")
                current_file = None
                continue
            lines = abs_path.read_text().splitlines()

        line_idx = loc.line - 1
        if line_idx >= len(lines):
            continue

        # If there's an attribute line (@...) immediately above, we want to insert above it.
        # Also move above ZASM-related doc tags and their indented blocks.
        insert_idx = line_idx
        while insert_idx > 0:
            prev_line = lines[insert_idx - 1].strip()
            # Attribute lines like @Bitflags
            if prev_line.startswith('@'):
                insert_idx -= 1
                continue
            # Doc tags that should come after @versionadded
            if re.match(r'^//\s*@(zasm|scripts/)', prev_line):
                insert_idx -= 1
                continue
            # Indented lines that are likely part of a multi-line tag like @zasm
            if re.match(r'^//\s{2,}', prev_line):
                temp_idx = insert_idx - 1
                found_tag_start = False
                while temp_idx > 0:
                    t_line = lines[temp_idx - 1].strip()
                    if re.match(r'^//\s*@(zasm|scripts/)', t_line):
                        found_tag_start = True
                        break
                    if not re.match(r'^//\s{2,}', t_line):
                        break
                    temp_idx -= 1
                if found_tag_start:
                    insert_idx = temp_idx
                    continue
            break

        line = lines[line_idx]
        indent_match = re.match(r'^\s*', line)
        indent = indent_match.group(0) if indent_match else ''

        # If the line immediately above the tags is a description line, add a blank line
        needs_blank = False
        if insert_idx > 0:
            prev_line = lines[insert_idx - 1]
            # If it's a comment line that has content and isn't a tag
            if re.match(r'^\s*//\s*\S+', prev_line) and not re.search(
                r'//\s*@', prev_line
            ):
                needs_blank = True

        # Look for EOL comment
        eol_comment_match = re.search(r'^(.*?)\s*//\s*(.*)$', line)

        if eol_comment_match:
            content = eol_comment_match.group(1).rstrip()
            comment_text = eol_comment_match.group(2).strip()

            if sym_name in content:
                # Remove the comment from the original line if we are moving it
                lines[line_idx] = content
                new_lines = []
                if needs_blank:
                    new_lines.append(f'{indent}//')
                new_lines.extend(
                    [
                        f'{indent}// {comment_text}',
                        f'{indent}//',
                        f'{indent}// @versionadded 3.0',
                    ]
                )
                lines[insert_idx:insert_idx] = new_lines
            else:
                if needs_blank:
                    lines.insert(insert_idx, f'{indent}//')
                    insert_idx += 1
                lines.insert(insert_idx, f'{indent}// @versionadded 3.0')
        else:
            if needs_blank:
                lines.insert(insert_idx, f'{indent}//')
                insert_idx += 1
            lines.insert(insert_idx, f'{indent}// @versionadded 3.0')

    if current_file:
        Path(current_file).write_text('\n'.join(lines) + '\n')

    print("Done fixing.")


def print_report(missing_tags, incorrect_tags):
    if missing_tags:
        print(f"\nFound {len(missing_tags)} symbols missing @versionadded 3.0 tags:\n")
        missing_tags.sort(key=lambda x: (x[2].file, x[2].line, x[0] or "", x[1]))
        current_file = None
        for cls, name, loc in missing_tags:
            if current_file and loc.file != current_file:
                print()
            current_file = loc.file
            prefix = f"{cls}->" if cls else ""
            print(f"{prefix}{name} at {loc.file}:{loc.line}")

    if incorrect_tags:
        print(
            f"\nFound {len(incorrect_tags)} symbols incorrectly marked @versionadded 3.0 (existed in 2.55):\n"
        )
        incorrect_tags.sort(key=lambda x: (x[2].file, x[2].line, x[0] or "", x[1]))
        current_file = None
        for cls, name, loc in incorrect_tags:
            if current_file and loc.file != current_file:
                print()
            current_file = loc.file
            prefix = f"{cls}->" if cls else ""
            print(f"{prefix}{name} at {loc.file}:{loc.line}")

    if not missing_tags and not incorrect_tags:
        print("\nAll symbols have correct version tags!")
    else:
        sys.exit(1)


def main():
    arg_parser = argparse.ArgumentParser(description='ZScript Version Linter')
    arg_parser.add_argument(
        '--fix', action='store_true', help='Auto-fix missing version tags'
    )
    args = arg_parser.parse_args()

    print("Fetching 2.55 symbols...")
    provider = get_content_provider()
    builtins_255 = parse_255_builtins(provider)
    headers_255 = parse_255_headers(provider)
    all_global_names_255_lower = build_255_globals_map(headers_255, builtins_255)

    print("Fetching current 3.0 symbols...")
    tmp_file = root_dir / '.tmp/lint_tmp.zs'
    tmp_file.write_text(
        '\n'.join(
            [
                '#include "std.zh"',
            ]
        )
    )

    all_files = parser.get_doc_data(tmp_file)

    missing_tags, incorrect_tags = find_version_tag_issues(
        all_files, builtins_255, headers_255, all_global_names_255_lower
    )

    if args.fix and missing_tags:
        apply_fixes(missing_tags)
        return

    print_report(missing_tags, incorrect_tags)


if __name__ == '__main__':
    main()
