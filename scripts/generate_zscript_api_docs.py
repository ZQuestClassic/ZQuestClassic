from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import sys
import textwrap
import time

from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from typing import Optional

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
resources_dir = root_dir / 'resources'
sources_dir = root_dir / 'docs-www/source'
tutorials_dir = root_dir / 'docs-www/source/tutorials'
zscript_dir = root_dir / 'docs-www/source/zscript'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

from zscript_doc_parser import (
    Class,
    Enum,
    EnumMember,
    File,
    Function,
    Scope,
    Variable,
    format_value,
    get_doc_data,
    reflink,
    types,
    walk,
    warning,
)

libraries = ['std', 'ghost', 'tango', 'gui', 'EmilyMisc']
sections = {
    'Globals': [],
    'Classes': [],
    'Libraries': [],
}
git_ref = subprocess.check_output(
    ['git', 'rev-parse', 'HEAD'], encoding='utf-8'
).strip()


def indent(text: str, amount: int, ch=' ') -> str:
    return textwrap.indent(text, amount * ch)


tmp_file = root_dir / '.tmp/tmp.zs'
tmp_file.write_text(
    '\n'.join(
        [
            '#include "std.zh"',
            '#include "ghost.zh"',
            '#include "tango.zh"',
            '#include "EmilyMisc.zh"',
            '#include "gui.zh"',
        ]
    )
)

all_files = get_doc_data(tmp_file)
lines = []
symbols_by_id = {}

# Create refs.
seen_refs = []
for file in all_files:

    def cb(symbol, parent):
        if hasattr(symbol, 'symbol_id'):
            if symbol.symbol_id:
                symbols_by_id[symbol.symbol_id] = symbol

        if isinstance(symbol, Class) or isinstance(symbol, Enum):
            types[symbol.name] = symbol

        if not hasattr(symbol, 'loc'):
            return

        if 'bindings' in file.name:
            ref = 'globals_'
            if isinstance(parent, Class):
                ref = 'globals_' if parent.is_global() else 'classes_'
            elif isinstance(symbol, Class):
                ref = 'globals_' if symbol.is_global() else 'classes_'
        else:
            rel_file = Path(file.name).relative_to(resources_dir)
            lib = next((x for x in libraries if x in str(rel_file)), None)
            if lib:
                ref = f'libs_{lib}_'
            else:
                raise Exception(f'unknown file: {file.name}')

        if 'bindings' in file.name and isinstance(parent, Class):
            ref += f'{parent.name}_'

        if isinstance(symbol, Function):
            if symbol.constructor:
                ref += 'ctor_'
            else:
                ref += 'fun_'
        elif isinstance(symbol, Variable):
            ref += 'var_'
        elif isinstance(symbol, Enum):
            ref += 'enum_'
        elif isinstance(symbol, EnumMember):
            ref += 'enum_member_'
        ref += symbol.name

        # Sphinx refs are normalized to lowercase.
        ref = ref.lower()

        # Needed for function overloading.
        initial_ref = ref
        attempts = 1
        while ref in seen_refs:
            attempts += 1
            ref = f'{initial_ref}_{attempts}'

        symbol.loc.ref = ref
        seen_refs.append(ref)

    walk(file, cb)


def rst_title(text: str):
    lines.append(text)
    lines.append('=' * len(text))
    lines.append('')


def rst_h2(text: str):
    lines.append(text)
    lines.append('-' * len(text))
    lines.append('')


def doclink(target, label='🔗') -> str:
    return f':ref:`{label}<{target}>`'


def add(text: str):
    lines.append(text)


def add_table(rows: list[list[str]]):
    if not rows:
        return

    for i, row in enumerate(rows):
        rows[i] = [f' {v} ' for v in row]

    cols = len(rows[0])
    col_widths = []
    for i in range(cols):
        w = 0
        for row in rows:
            w = max(w, len(row[i]))
        col_widths.append(w)
    table_width = sum(col_widths)

    add('.. table::')
    add('   :widths: auto')
    add('')

    divider = '+'
    for x in col_widths:
        divider += '-' * x + '+'

    add('   ' + divider)
    for row in rows:
        cols = []
        for i, v in enumerate(row):
            cols.append(v.ljust(col_widths[i]))
        add('   ' + '|' + '|'.join(cols) + '|')
        add('   ' + divider)

    add('')


def rst_toc(caption: str, documents):
    add('.. toctree::')
    add('   :maxdepth: 1')
    add('   :class: toctree-grid')
    if caption:
        add(f'   :caption: {caption}')
    add('')
    for document in documents:

        add(f'   /{document}')
    add('')


def add_comment(symbol):
    def replace_symbol_link(match: re.Match):
        if '|' in match.group(1):
            symbol_id, label = match.group(1).split('|')
            matched_symbol = symbols_by_id.get(int(symbol_id))
        else:
            matched_symbol = symbols_by_id.get(int(match.group(1)))
            label = matched_symbol.name

        if not matched_symbol:
            raise Exception(
                f'could not resolve symbol link in comment: {symbol.comment.text}'
            )

        return reflink(matched_symbol, label)

    def replace_docs_link(match: re.Match):
        if '|' in match.group(1):
            docs_key, label = match.group(1).split('|')
        else:
            docs_key = match.group(1)
            label = docs_key
        return doclink(docs_key, label)

    monos = []

    def replace_monos(match: re.Match):
        monos.append(match.group(1))
        return '$MONO'

    def replace_monos_placeholder(match: re.Match):
        return f'\\ ``{monos.pop(0)}``'

    code_blocks = []

    def replace_code_block(match: re.Match):
        code = match.group(1).replace('\n', '\n   ')
        code_blocks.append(code)
        return '\n.. zscript::\n   $CODE'

    def replace_code_block_placeholder(match: re.Match):
        return code_blocks.pop(0)

    def sanitize(text: str) -> str:
        text = re.sub(r'```(.+?)```', replace_code_block, text, flags=re.DOTALL)
        text = re.sub(r'`(.+?)`', replace_monos, text)
        text = text.replace('_', '\\_').replace('*', '\\*').replace('`', '``')
        text = re.sub(r'\$CODE', replace_code_block_placeholder, text)
        text = re.sub(r'\$MONO', replace_monos_placeholder, text)
        text = re.sub(r'\[@(.+?)@\]', replace_symbol_link, text)
        text = re.sub(r'\[#(.+?)#\]', replace_docs_link, text)
        text = re.sub(r'^# (.+)', r':comment_header:`\1`', text, flags=re.MULTILINE)
        return text

    def format_comment(text: str) -> str:
        return indent(sanitize(text), 3)

    interesting_tags = [
        'alias',
        'deprecated_alias',
        'deprecated_future',
        'deprecated_getter',
        'index',
        'length',
        'param',
        'value',
        'versionadded',
        'versionchanged',
        'versionremoved',
    ]
    has_comment_or_interesting_tags = symbol.comment and (
        symbol.comment.text
        or next((t for t in symbol.comment.tags if t[0] in interesting_tags), None)
    )

    if has_comment_or_interesting_tags:
        add('')
        add('.. rst-class:: classref-comment')
        add('')
        for tag, value in symbol.comment.tags:
            if tag in [
                'versionadded',
                'versionchanged',
                'versionremoved',
            ]:
                data = value.split(' ', 1)
                add(format_comment(f'.. {tag}:: {data[0].strip()}'))
                if len(data) > 1:
                    add(format_comment(indent(f'{data[1].strip()}', 3)))
                add('')
                add('')
            elif tag in interesting_tags:
                add(format_comment(f'`{tag}` ' + value))
                add('')
                add('')
        add('')
        add(format_comment(symbol.comment.text))
        add('')

    if symbol.comment and symbol.comment.has_tag('deprecated'):
        notice = symbol.comment.get_tag_single('deprecated')
        if notice == True:
            notice = ''
        else:
            notice = notice.replace('\n', ' ')
        add('')
        add('.. deprecated::')
        add('')
        add(format_comment(notice))
        add('')


def handle_variable(symbol: Variable):
    add(f'.. _{symbol.loc.ref}:\n')

    parts = [
        str(symbol.type),
        f'**{symbol.name}**',
        (
            f'= ``{format_value(symbol.type, symbol.value)}``'
            if symbol.value != None
            else ''
        ),
        reflink(symbol),
    ]
    if symbol.loc:
        parts.append(symbol.loc.reflink())
    add(' '.join(parts))
    add_comment(symbol)

    add('')
    add('.. rst-class:: classref-item-separator')
    add('')
    add('----')
    add('')


def handle_enum(symbol: Enum):
    add(f'.. _{symbol.loc.ref}:\n')

    add(f'enum **{symbol.name}**: {reflink(symbol)}')
    if symbol.loc:
        add(symbol.loc.reflink())
    add('')
    add_comment(symbol)

    for member in symbol.members:
        add(f'.. _{member.loc.ref}:')
        add('')
        add('.. rst-class:: classref-enumeration-constant')
        add('')
        value_str = format_value(member.type, member.value)
        add(f'{reflink(symbol, symbol.name)} **{member.name}** = ``{value_str}``')
        add_comment(member)
        add('')

    add('')
    add('.. rst-class:: classref-item-separator')
    add('')
    add('----')
    add('')


def handle_function(symbol: Function):
    add(f'.. _{symbol.loc.ref}:\n')
    add('.. rst-class:: classref-method\n')

    parts = [
        str(symbol.return_type),
        f'**{symbol.name}**' + '\\ ' + str(symbol.parameters),
        reflink(symbol),
    ]
    if symbol.loc:
        parts.append(symbol.loc.reflink())
    add(' '.join(parts))
    add_comment(symbol)

    add('')
    add('.. rst-class:: classref-item-separator')
    add('')
    add('----')
    add('')


def handle_scope(symbol):
    constructors = []
    functions = []
    variables = []
    enums = []

    def cb(symbol, parent):
        if isinstance(symbol, Function):
            if symbol.constructor:
                constructors.append(symbol)
            else:
                functions.append(symbol)
        elif isinstance(symbol, Variable):
            variables.append(symbol)
        elif isinstance(symbol, Enum):
            enums.append(symbol)

    walk(symbol, cb)

    def deprecated(x):
        return ' **deprecated**' if x.deprecated() else ''

    if variables:
        rst_h2('Variables')
        add_table([[str(x.type), x.link() + deprecated(x)] for x in variables])

    if constructors:
        rst_h2('Constructors')
        add_table(
            [
                [
                    str(x.return_type),
                    (x.link() + '\\ ' + str(x.parameters) + deprecated(x)),
                ]
                for x in constructors
            ]
        )

    if functions:
        rst_h2('Functions')
        add_table(
            [
                [
                    str(x.return_type),
                    (x.link() + '\\ ' + str(x.parameters) + deprecated(x)),
                ]
                for x in functions
            ]
        )

    if enums:
        rst_h2('Enumerations')
        for enum in enums:
            handle_enum(enum)

    if variables:
        rst_h2('Variable descriptions')
        for variable in variables:
            handle_variable(variable)

    if constructors:
        rst_h2('Constructor descriptions')
        for constructor in constructors:
            handle_function(constructor)

    if functions:
        rst_h2('Function descriptions')
        for function in functions:
            handle_function(function)

    # Remove the last five lines (separator stuff), since there is nothing that follows.
    global lines
    if lines[-1] == '' and lines[-2] == '----':
        lines = lines[0:-4]


def process_bindings_class(file: File, symbol: Class):
    add(f'.. _{symbol.loc.ref}:\n')
    rst_title(symbol.name)

    if symbol.parent:
        parents = []
        cur = symbol.parent
        while cur:
            parents.append(cur)
            cur = cur.parent
        parts = [reflink(s, s.name) for s in parents]
        add(f'Inherits: {" < ".join(parts)}')
        add('')

    inherited_by = []
    for s in types.values():
        if isinstance(s, Class) and s.parent == symbol:
            inherited_by.append(s)
    if inherited_by:
        inherited_by.sort(key=lambda s: s.name)
        parts = [reflink(s, s.name) for s in inherited_by]
        add(f'Inherited by: {", ".join(parts)}')
        add('')

    add_comment(symbol)

    if symbol.comment and symbol.comment.has_tag('tutorial'):
        rst_h2('Tutorials')
        for tutorial in symbol.comment.get_tag_many('tutorial'):
            label = tutorial.replace('tutorials/', '').capitalize()
            add(f'* :doc:`{tutorial}`')
            add('')

    handle_scope(
        Scope(
            name='',
            enums=file.enums,
            classes=[],
            functions=[*symbol.constructors, *symbol.functions],
            variables=symbol.variables,
        )
    )


def process_lib(name: str, files):
    lib_documents = []

    for file in files:
        path = Path(file.name)
        fname = path.stem
        if path.name == 'std.cfg':
            fname = 'cfg'
        rst_title(path.name)

        handle_scope(file)

        doc = f'zscript/libs/{name}/{fname}'
        lib_documents.append(doc)
        write(doc)

    lib_documents.sort()

    document = f'zscript/libs/{name}/index'
    sections['Libraries'].append(document)
    rst_title(lib)
    add(f'.. _lib_{name}:')
    rst_toc(None, lib_documents)

    example = None
    docs = None
    if name == 'gui':
        example = 'gui_example.zs'
    elif name == 'tango':
        example = 'tango_example.zs'
        docs = 'The original documentation for tango can be found `here <../../../_static/old/tango/index.html>`__'
    elif name == 'ghost':
        docs = 'The original documentation for ghost can be found `here <../../../_static/old/ghost/index.html>`__'

    if docs:
        rst_h2('Docs')
        add(docs)
        add('')

    if example:
        rst_h2('Example')
        add('.. only:: html')
        add('')
        url = f'https://raw.githubusercontent.com/ZQuestClassic/ZQuestClassic/refs/heads/{git_ref}/resources/headers/examples/{example}'
        add('   .. zscript::')
        add(f'      :url: {url}')
        add(f'      :fname: {example}')
        add('')

        add('.. only:: not html')
        add('')
        add(f'   .. literalinclude :: /../../resources/headers/examples/{example}')
        add('')

    write(document)


def write(name: str):
    global lines

    out_file = sources_dir / f'{name}.rst'
    out_file.parent.mkdir(parents=True, exist_ok=True)
    text = '.. Auto-generated by generate_zscript_api_docs.py\n\n' + '\n'.join(lines)
    out_file.write_text(text, 'utf-8')
    lines = []


for folder in ['classes', 'globals', 'libs']:
    if (zscript_dir / folder).exists():
        shutil.rmtree(zscript_dir / folder)

# handle bindings
classes = []
functions = []
variables = []
enums = []
for file in [f for f in all_files if 'bindings' in f.name]:
    if not file.classes:
        functions.extend(file.functions)
        variables.extend(file.variables)
        enums.extend(file.enums)
        continue

    if len(file.classes) != 1:
        raise Exception('dunno how to handle multiple classes here')

    # Put the functions/variables into the global documentation page, but
    # keep the enums (see process_bindings_class).
    functions.extend(file.functions)
    variables.extend(file.variables)

    symbol = file.classes[0]
    is_global = symbol.is_global()
    section = sections['Globals'] if is_global else sections['Classes']
    prefix = 'globals' if is_global else 'classes'
    document = f'zscript/{prefix}/{symbol.name}'
    section.append(document)
    process_bindings_class(file, symbol)
    write(document)

document = 'zscript/globals/constants'
sections['Globals'].insert(0, document)
rst_title('Global constants')
handle_scope(
    Scope(name='constants', enums=enums, classes=[], functions=[], variables=variables)
)
write(document)

document = 'zscript/globals/functions'
sections['Globals'].insert(0, document)
rst_title('Global functions')
handle_scope(
    Scope(name='globals', enums=[], classes=[], functions=functions, variables=[])
)
write(document)

# handle libs
for lib in libraries:
    files = [f for f in all_files if lib in f.name]
    process_lib(lib, files)

# zscript/index.rst
rst_title('ZScript')

rst_toc(
    'Language',
    [
        'zscript/lang/introduction',
        'zscript/lang/comments',
        'zscript/lang/literals',
        'zscript/lang/keywords',
        'zscript/lang/declarations/index',
        'zscript/lang/types/index',
        'zscript/lang/control_flow/index',
        'zscript/lang/ranges',
        'zscript/lang/scripts',
        'zscript/lang/annotations',
        'zscript/lang/options',
        'zscript/lang/compiler_directives',
    ],
)

add('.. _zsdoc_index:')
for name, documents in sections.items():
    add(f'.. _zsdoc_{name.lower()}:')
    if name == 'Globals':
        rst_toc(name, documents[0:2])
        rst_toc(None, documents[2:])
        continue

    rst_toc(name, documents)
write('zscript/index')
