import argparse
import re
import os
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--name', help='The name to reserve for (will be capitalized for you)')
parser.add_argument('--count', type = int, help = 'How many opcodes or registers to reserve')
parser.add_argument('--listfile', help = '''Specify a file to read names from each line, instead of a name/count.
    For '-r', the file should have a single term per line.
        The term should only contain capital letters, numbers, and '_'. (ex 'RESRVD_VAR_EMILY_1')
    For '-o','-u','-b', the file should have 2 terms per line, separated by whitespace.
        The first should only contain capital letters, numbers, and '_'. (ex 'RESRVD_OP_EMILY_1')
        The second should only contain letters, numbers, and '_'. (ex 'OReservedEmily_1')''')

mode_group = parser.add_argument_group('Mode','The operation mode (REQUIRED)')

exclgroup = mode_group.add_mutually_exclusive_group()

exclgroup.add_argument('--reg','-r', action='store_true',help='Add ZASM registers')
exclgroup.add_argument('--opcode','-o', action='store_true',help='Add ZASM opcodes')
exclgroup.add_argument('--unopcode','-u', action='store_true',help='Add Unary ZASM opcodes')
exclgroup.add_argument('--binopcode','-b', action='store_true',help='Add Binary ZASM opcodes')
exclgroup.add_argument('--ternopcode','-t', action='store_true',help='Add Ternary ZASM opcodes')

args = parser.parse_args()

opcodetype = 3 if args.ternopcode else (2 if args.binopcode else (1 if args.unopcode else (0 if args.opcode else -1)))

if not (args.reg or opcodetype > -1):
    print("Specify either '--reg' or '--opcode'/'--unopcode'/'--binopcode' to choose a mode!", file=sys.stderr)
    exit(1)

reglist = []
oplist_allcaps = []
oplist_propcase = []
file_bytecode_cpp = '../src/parser/ByteCode.cpp'
file_bytecode_h = '../src/parser/ByteCode.h'
file_ffscript_h = '../src/zc/ffscript.h'
file_zasm_table_cpp = '../src/zasm_table.cpp'
file_zasm_table_h = '../src/zasm_table.h'
        
def printlns(lns): # For debugging
    for line in lns:
        if line[-1] == '\n':
            line = line[:-1]
        print(line)
def write_file(name,lines):
    with open(name, 'w') as file:
        for q in range(len(lines)-1,-1,-1):
            if lines[q].strip() == '':
                lines.pop(q)
            else:
                lines.append('\n')
                break
        file.writelines(lines)
    return
def propercase(s:str) -> str:
    pat_excl = re.compile('[^a-zA-Z0-9]')
    pat_spec = re.compile('[0-9]')
    pat_wordbreak = re.compile('[^a-zA-Z]')
    return ''.join([x if pat_spec.match(x) else
        (x.upper() if ind < 1 or pat_wordbreak.match(s[ind-1]) else x.lower())
        for ind,x in enumerate(s)
        if not pat_excl.match(x)])
if args.listfile:
    if args.name or args.count:
        print("Cannot specify '--listfile' alongside '--name' or '--count'!", file=sys.stderr)
        exit(1)
    if not os.path.isfile(args.listfile):
        print(f"File '{args.listfile}' not found to read from!", file=sys.stderr)
        exit(1)
    with open(args.listfile, 'r') as file:
        tmplines = [x.strip() for x in file.readlines() if len(x.strip()) > 0]
        if opcodetype < 0:
            reglist = [x for x in tmplines if re.match('^[A-Z0-9_]+$',x)]
            args.count = len(reglist)
        else:
            oppat_1 = '^([A-Z0-9_]+)[ \t]+[A-Za-z0-9_]+$'
            oppat_2 = '^[A-Z0-9_]+[ \t]+([A-Za-z0-9_]+)$'
            oplist_allcaps = [re.match(oppat_1,x).groups()[0] for x in tmplines if re.match(oppat_1,x)]
            oplist_propcase = [re.match(oppat_2,x).groups()[0] for x in tmplines if re.match(oppat_2,x)]
            args.count = len(oplist_allcaps)
    if args.count < 1:
        print("Must specify non-empty file for '--listfile'!", file=sys.stderr)
        exit(1)
else:
    if not args.name:
        print("Must specify a name via '--name' to reserve with!", file=sys.stderr)
        exit(1)
    if not args.count or args.count < 1:
        print("Must specify at least 1 for '--count'!", file=sys.stderr)
        exit(1)
    args.name = re.sub(' ','_',args.name.upper())
    if args.reg:
        lines = []
        with open(file_bytecode_cpp, 'r') as file:
            lines = file.readlines()
        index = 0
        first_reserved = -1
        last_reserved = 0
        while lines[index].strip() != 'string ZScript::VarToString(int32_t ID)':
            index = index + 1
        while lines[index].strip() != 'default:':
            m = re.match(f'case RESRVD_VAR_{args.name}_?([0-9]+): return "RESRVD_VAR_{args.name}_?[0-9]+";',lines[index].strip())
            if m:
                if val > last_reserved:
                    last_reserved = val
                if first_reserved < 0 or val < first_reserved:
                    first_reserved = val
            index = index + 1
        # starting index number
        start = last_reserved if first_reserved < args.count else 0
        for q in range(args.count):
            reglist.append(f'RESRVD_VAR_{args.name}_{start+q+1}')
        #generated the register list
    else:
        name_prop = propercase(args.name)
        lines = []
        with open(file_bytecode_cpp, 'r') as file:
            lines = file.readlines()
        index = 0
        first_reserved = -1
        last_reserved = 0
        m = None
        while index < len(lines):
            m = re.match(f'string OReserved{name_prop}_?([0-9]+)::toString\\(\\) const',lines[index].strip())
            if m:
                val = int(m.groups()[0])
                if val > last_reserved:
                    last_reserved = val
                if first_reserved < 0 or val < first_reserved:
                    first_reserved = val
            index = index + 1
        index = index - 1
        while len(lines[index].strip()) == 0:
            index = index - 1
        index = index + 1
        # starting index number
        start = last_reserved if first_reserved < args.count else 0
        for q in range(args.count):
            oplist_allcaps.append(f'RESRVD_OP_{args.name}_{start+q+1}')
            oplist_propcase.append(f'OReserved{name_prop}_{start+q+1}')
        #generated the opcode list
if args.reg:
    # zasm_table.h
    with open(file_zasm_table_h, 'r') as file:
        lines = file.readlines()
    index = -1
    m = None
    while not m:
        index = index + 1
        m = re.match('#define NUMVARIABLES( +)(0x[0-9A-Fa-f]+)',lines[index].strip())
    ws,num = m.groups()
    num = int(num,16)
    newlines = []
    for reg in reglist:
        numspace = (12+len(ws))-len(reg)
        if numspace < 2:
            numspace = 2
        newlines.append(f'#define {reg}{" "*numspace}0x{num:04X}\n')
        num = num + 1
    newlines.append('\n')
    lines[index] = f'#define NUMVARIABLES{ws}0x{num:04X}\n'
    lines[index:index] = newlines
    write_file(file_zasm_table_h,lines)
    
    # zasm_table.cpp
    with open(file_zasm_table_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_variable[ \t]+variable_list[ \t]*\[\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]*){ " ", -1 }',lines[index])
    ws = m.groups()[0]
    newlines = []
    for reg in reglist:
        newlines.append(f'{ws}{{ "{reg}", {reg} }},\n')
    newlines.append('\n')
    lines[index:index] = newlines
    write_file(file_zasm_table_cpp,lines)
elif opcodetype > -1:
    # Bytecode.cpp
    lines = []
    with open(file_bytecode_cpp, 'r') as file:
        lines = file.readlines()
    index = len(lines) - 1
    while len(lines[index].strip()) == 0:
        index = index - 1
    index = index + 1
    # starting index number
    newlines = ['\n','\n']
    for q in range(args.count):
        newlines.append(f'int32_t {oplist_propcase[q]}::command() const\n')
        newlines.append('{\n')
        newlines.append(f'\treturn {oplist_allcaps[q]};\n')
        newlines.append('}\n')
    wslines = len(lines)-index-1
    if wslines < 2:
        for q in range(wslines,2):
            newlines.append('\n')
    else:
        lines[index:] = []
    
    lines[index:index] = newlines
    write_file(file_bytecode_cpp,lines)
    
    # Bytecode.h
    with open(file_bytecode_h, 'r') as file:
        lines = file.readlines()
    index = len(lines)-1
    m = None
    while not re.match('}',lines[index].strip()):
        index = index - 1
    newlines = ['\n','\n','\n']
    for op in oplist_propcase:
        match opcodetype:
            case 3:
                newlines.append(f'\tclass {op} : public TernaryOpcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\t{op}(Argument *A, Argument* B, Argument* C) : TernaryOpcode(A,B,C) {{}}\n')
                newlines.append(f'\t\tint32_t command() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {op}(a->clone(),b->clone(),c->clone());\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
            case 2:
                newlines.append(f'\tclass {op} : public BinaryOpcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\t{op}(Argument *A, Argument* B) : BinaryOpcode(A,B) {{}}\n')
                newlines.append(f'\t\tint32_t command() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {op}(a->clone(),b->clone());\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
            case 1:
                newlines.append(f'\tclass {op} : public UnaryOpcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\t{op}(Argument *A) : UnaryOpcode(A) {{}}\n')
                newlines.append(f'\t\tint32_t command() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {op}(a->clone());\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
            case _:
                newlines.append(f'\tclass {op} : public Opcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\tint32_t command() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {op}();\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
    newlines.pop(-1) # remove trailing newline
    lines[index:index] = newlines
    write_file(file_bytecode_h,lines)
    
    # zasm_table.h
    with open(file_zasm_table_h, 'r') as file:
        lines = file.readlines()
    index = 0
    while not lines[index].strip() == 'enum ASM_DEFINE':
        index = index + 1
    m = None
    num = 0
    while not m:
        index = index + 1
        if re.match('[^/]*,.*',lines[index].strip()):
            num = num + 1
        m = re.match('([ \t]*)NUMCOMMANDS[ \t]*//0x[0-9A-Fa-f]+',lines[index])
    ws = m.groups()[0]
    newlines = [f'{ws}\n']
    for op in oplist_allcaps:
        newlines.append(f'{ws}{op},\n')
        num = num + 1
    newlines.append('\n')
    lines[index] = f'{ws}NUMCOMMANDS  //0x{num:04X}\n'
    lines[index:index] = newlines
    write_file(file_zasm_table_h,lines)
    
    # zasm_table.cpp
    with open(file_zasm_table_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_command[ \t]+command_list[ \t]*\[[ \t]*NUMCOMMANDS[ \t]*\+[ \t]*1[ \t]*\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]){[ \t]*"",[ \t]*0,[ \t]*{},[ \t]*0[ \t]*,[ \t]*0[ \t]*}',lines[index])
    newlines = []
    argstr = ''
    if opcodetype > 0:
        argstr = ' ?' + (', ?'*(opcodetype-1)) + ' ';
    for op in oplist_allcaps:
        newlines.append(f'\t{{ "{op}", {opcodetype}, {{{argstr}}}, 0, 0 }},\n')
    newlines.append('\n')
    lines[index] = f'\t{{ "", 0, {{}}, 0, 0 }}\n'
    lines[index:index] = newlines
    write_file(file_zasm_table_cpp,lines)
else:
    exit(0)

