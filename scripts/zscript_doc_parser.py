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

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

git_ref = subprocess.check_output(
    ['git', 'rev-parse', 'HEAD'], encoding='utf-8'
).strip()

# TODO: make a class for the parser (get_doc_data) and remove these globals.
seen_warnings = []
types = {}


def reflink(symbol, label='🔗') -> str:
    return f':ref:`{label}<{symbol.loc.ref}>`'


class SymbolKind(Enum):
    File = 1
    Module = auto()
    Namespace = auto()
    Package = auto()
    Class = auto()
    Method = auto()
    Property = auto()
    Field = auto()
    Constructor = auto()
    Enum = auto()
    Interface = auto()
    Function = auto()
    Variable = auto()
    Constant = auto()
    String = auto()
    Number = auto()
    Boolean = auto()
    Array = auto()
    Object = auto()
    Key = auto()
    Null = auto()
    EnumMember = auto()
    Struct = auto()
    Event = auto()
    Operator = auto()
    TypeParameter = auto()


def warning(text: str):
    if text not in seen_warnings:
        print(text)
        seen_warnings.append(text)


@dataclass
class Location:
    ref: str
    file: str
    line: int

    def reflink(self):
        url = f'https://github.com/ZQuestClassic/ZQuestClassic/blob/{git_ref}/{self.file}#L{self.line}'
        return f'`Source <{url}>`__'


@dataclass
class Comment:
    text: str
    tags: list[tuple[str, str]]

    def has_tag(self, name: str) -> bool:
        return self.get_tag_single(name) != None

    def get_tag_single(self, name: str) -> Optional[str]:
        return next((t[1] for t in self.tags if t[0] == name), None)

    def get_tag_many(self, name: str) -> list[str]:
        return [t[1] for t in self.tags if t[0] == name]


@dataclass
class Type:
    name: str
    const: bool

    def is_array(self) -> bool:
        return '[]' in self.name

    def __str__(self):
        const_prefix = '|const| ' if self.const else ''
        id = self.name.replace('[]', '')
        if id in ['void', 'int', 'long', 'untyped', 'T', 'T1', 'T2']:
            return f'{const_prefix}|{id}|\\ {self.name.replace(id, "")}'
        if id in ['bool', 'char32', 'rgb']:
            return f'{const_prefix}{self.name}'

        symbol = types.get(id)
        if symbol:
            return f'{const_prefix}{reflink(symbol, self.name)}'

        warning(f'TODO: cant link {self.name}')
        return f'{const_prefix}{self.name}'


@dataclass
class Variable:
    symbol_id: int
    loc: Location
    comment: Comment
    name: str
    type: Type
    value: Optional[str]

    def link(self) -> str:
        return reflink(self, self.name)

    def deprecated(self) -> bool:
        return self.comment and self.comment.has_tag('deprecated')


@dataclass
class EnumMember:
    symbol_id: int
    loc: Location
    comment: Comment
    name: str
    value: str
    type: Type


@dataclass
class Enum:
    symbol_id: int
    loc: Location
    comment: Comment
    name: str
    prefix: str
    members: list[EnumMember]


@dataclass
class Parameters:
    variables: list[Variable]
    varargs: Optional[int]

    def __str__(self) -> str:
        parts = []
        for var in self.variables:
            def_str = ''
            if var.value != None:
                value_str = format_value(var.type, var.value)
                def_str = f' = ``{value_str}``'
            parts.append(f'{var.type} {var.name}{def_str}')
        if self.varargs != None:
            if self.variables and self.variables[-1].type.name == 'T':
                parts.append(f'{self.variables[-1].type} |varargs| ...')
            else:
                parts.append('untyped |varargs| ...')
        text = ', '.join(parts)
        return f'({text})'


@dataclass
class Function:
    symbol_id: int
    loc: Location
    comment: Comment
    name: str
    parameters: Parameters
    return_type: Type
    constructor: bool

    def link(self) -> str:
        return reflink(self, self.name)

    def deprecated(self) -> bool:
        return self.comment and self.comment.has_tag('deprecated')


@dataclass
class Class:
    symbol_id: int
    loc: Location
    comment: Comment
    name: str
    parent: Optional[Class]
    constructors: list[Function]
    functions: list[Function]
    variables: list[Variable]
    enums: list[Enum]

    def is_global(self) -> bool:
        return self.name[0].isupper()


@dataclass
class File:
    name: str
    enums: list[Enum]
    classes: list[Class]
    functions: list[Function]
    variables: list[Variable]


@dataclass
class Scope:
    name: str
    enums: list[Enum]
    classes: list[Class]
    functions: list[Function]
    variables: list[Variable]


def format_value(type: Type, value) -> str:
    if isinstance(value, str):
        return value
    if type.is_array() and value == 0:
        return 'null'
    if type.name == 'bool':
        return 'true' if value else 'false'
    if type.name == 'int' or type.name == 'char32' or type.name == 'rgb':
        return f'{value / 10000:g}'
    if type.name == 'long':
        return f'{value}L'
    return value


def parse_doc_type(type_str: str) -> Type:
    const = False
    if type_str.startswith('const '):
        type_str = type_str[6:]
        const = True
    if type_str == 'screendata':
        type_str = 'Screen'
    if type_str == 'hero':
        type_str = 'Hero'
    return Type(name=type_str, const=const)


def parse_doc_comment(x) -> Optional[Comment]:
    if x.get('comment'):
        text = x['comment']['text']
        tags = x['comment']['tags']

        SINGLE_VALUE_TAGS = [
            'delete',
            'deprecated_getter',
            'deprecated',
            'exit',
            'extends',
            'index',
            'length',
            'reassign_ptr',
            'value',
            'vargs',
            'zasm_ref',
            'zasm_var',
        ]
        for tag in SINGLE_VALUE_TAGS:
            count = len([t for t in tags if t[0] == tag])
            if count > 1:
                raise Exception(
                    f'@{tag} cannot have multiple values.\n\nComment: {text}\n\nTags: {tags}'
                )

        return Comment(text=text, tags=tags)

    return None


def parse_doc_parameters(x) -> Parameters:
    variables = []
    varargs = x.get('varargs', None)

    params = x['parameters']
    for i, param in enumerate(params):
        name = param['name']
        type = parse_doc_type(param['type'])
        value = param.get('default', None)
        variables.append(
            Variable(
                symbol_id=None,
                loc=None,
                comment=None,
                name=name,
                type=type,
                value=value,
            )
        )

    return Parameters(variables=variables, varargs=varargs)


def parse_doc_enum_member(x) -> EnumMember:
    symbol_id = x['id']
    loc = parse_location(x)
    comment = parse_doc_comment(x)
    name = x['name']
    value = x['value']
    type = parse_doc_type(x['type'])
    return EnumMember(
        symbol_id=symbol_id, loc=loc, comment=comment, name=name, value=value, type=type
    )


def parse_location(x) -> Location:
    file = Path(x['location']['file'])
    file = file.relative_to(root_dir).as_posix()
    return Location(ref='', file=file, line=x['location']['line'])


def parse_doc_symbol(x, parent=None) -> File:
    kind = SymbolKind(x['kind'])
    if kind == SymbolKind.Variable:
        symbol_id = x['id']
        loc = parse_location(x)
        comment = parse_doc_comment(x)
        name = x['name']
        type = parse_doc_type(x['type'])
        value = x.get('value', None)
        return Variable(
            symbol_id=symbol_id,
            loc=loc,
            comment=comment,
            name=name,
            type=type,
            value=value,
        )
    elif kind == SymbolKind.Enum:
        symbol_id = x['id']
        loc = parse_location(x)
        comment = parse_doc_comment(x)
        name = x['name']
        prefix = x.get('prefix', None)
        members = [parse_doc_enum_member(m) for m in x['children']]

        # For now, give unnamed enums a fake name.
        # TODO: Eventually, let's just update enums to have names.
        if not name:
            if prefix:
                name = prefix[0:-1]  # remove trailing _
            elif members:
                name = members[0].name
            else:
                name = 'UnknownEnum'

        return Enum(
            symbol_id=symbol_id,
            loc=loc,
            comment=comment,
            name=name,
            prefix=prefix,
            members=members,
        )
    elif kind in [SymbolKind.Function, SymbolKind.Constructor]:
        symbol_id = x['id']
        loc = parse_location(x)
        comment = parse_doc_comment(x)
        name = x['name']
        return_type = parse_doc_type(x['returnType'])
        parameters = parse_doc_parameters(x)
        return Function(
            symbol_id=symbol_id,
            loc=loc,
            comment=comment,
            name=name,
            parameters=parameters,
            return_type=return_type,
            constructor=kind == SymbolKind.Constructor,
        )
    elif kind == SymbolKind.Class:
        symbol_id = x['id']
        loc = parse_location(x)
        comment = parse_doc_comment(x)
        name = x['name']
        if name == 'screendata':
            name = 'Screen'
        if name == 'hero':
            name = 'Hero'
        parent = x.get('parent', None)  # 'str' for now, but set to Class later.
        constructors = []
        functions = []
        variables = []
        enums = []
        for symbol in x['children']:
            kind = SymbolKind(symbol['kind'])
            if kind == SymbolKind.Function:
                functions.append(parse_doc_symbol(symbol, name))
            elif kind == SymbolKind.Constructor:
                constructors.append(parse_doc_symbol(symbol, name))
            elif kind == SymbolKind.Variable:
                variables.append(parse_doc_symbol(symbol, name))
            elif kind == SymbolKind.Enum:
                enums.append(parse_doc_symbol(symbol, name))

        return Class(
            symbol_id=symbol_id,
            loc=loc,
            comment=comment,
            name=name,
            parent=parent,
            constructors=constructors,
            functions=functions,
            variables=variables,
            enums=enums,
        )


def parse_doc_file(x) -> File:
    file = File(name=x['name'], enums=[], classes=[], functions=[], variables=[])
    for symbol in x['symbols']:
        kind = SymbolKind(symbol['kind'])
        if kind == SymbolKind.Variable:
            file.variables.append(parse_doc_symbol(symbol))
        elif kind == SymbolKind.Enum:
            file.enums.append(parse_doc_symbol(symbol))
        elif kind == SymbolKind.Function:
            file.functions.append(parse_doc_symbol(symbol))
        elif kind == SymbolKind.Class:
            file.classes.append(parse_doc_symbol(symbol))
    return file


def get_doc_data(script_path: Path) -> list[File]:
    # Change include paths to use resources/ directly, instead of possibly-stale stuff inside a build folder.
    include_paths = [
        str(root_dir / 'resources/include'),
        str(root_dir / 'resources/headers'),
    ]
    args = [
        '-input',
        str(script_path),
        '-include',
        ';'.join(include_paths),
        '-unlinked',
        '-delay_cassert',
        '-doc',
    ]
    p = run_target.run('zscript', args, env={**os.environ, 'TEST_ZSCRIPT': '1'})
    if p.returncode:
        raise Exception(p.stderr)

    data = json.loads(p.stdout)
    # print(json.dumps(data,indent=2))
    # exit(0)
    files = []
    for file in data['files']:
        file = parse_doc_file(file)
        if file.variables or file.functions or file.classes or file.enums:
            files.append(file)
    files.sort(key=lambda x: x.name)

    # Set Class.parent
    classes = {}
    for file in files:

        def get_classes(symbol, parent):
            if isinstance(symbol, Class):
                classes[symbol.name] = symbol

        walk(file, get_classes)
    for symbol in classes.values():
        if symbol.parent:
            symbol.parent = classes[symbol.parent]

    return files


def walk(symbol, cb, parent=None):
    cb(symbol, parent)
    if isinstance(symbol, Class):
        for x in symbol.functions:
            walk(x, cb, symbol)
        for x in symbol.constructors:
            walk(x, cb, symbol)
        for x in symbol.variables:
            walk(x, cb, symbol)
    elif isinstance(symbol, File) or isinstance(symbol, Scope):
        for x in symbol.functions:
            walk(x, cb, symbol)
        for x in symbol.variables:
            walk(x, cb, symbol)
        for x in symbol.classes:
            walk(x, cb, symbol)
        for x in symbol.enums:
            walk(x, cb, symbol)
    elif isinstance(symbol, Enum):
        for x in symbol.members:
            walk(x, cb, symbol)
