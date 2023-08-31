import argparse
import re
import os
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--name', required = True, help='The name to reserve for (will be capitalized for you)')
parser.add_argument('--count', required = True, type = int, help = 'How many opcodes or registers to reserve')

mode_group = parser.add_argument_group('Mode','The operation mode (REQUIRED)')

exclgroup = mode_group.add_mutually_exclusive_group()

exclgroup.add_argument('--reg','-r', action='store_true',help='Add ZASM registers')
exclgroup.add_argument('--opcode','-o', action='store_true',help='Add ZASM opcodes')
exclgroup.add_argument('--unopcode','-u', action='store_true',help='Add Unary ZASM opcodes')
exclgroup.add_argument('--binopcode','-b', action='store_true',help='Add Binary ZASM opcodes')

args = parser.parse_args()

opcodetype = 2 if args.binopcode else (1 if args.unopcode else (0 if args.opcode else -1))

if not args.name:
    print("Must specify a name via '--name' to reserve with!")
    exit(0)
if not args.count or args.count < 1:
    print("Must specify at least 1 for '--count'!")
    exit(0)
if not (args.reg or opcodetype > -1):
    print("Specify either '--reg' or '--opcode'/'--unopcode'/'--binopcode' to choose a mode!")
    exit(0)

args.name = re.sub(' ','_',args.name.upper())

def printlns(lns):
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

file_bytecode_cpp = '../src/parser/ByteCode.cpp'
file_bytecode_h = '../src/parser/ByteCode.h'
file_ffscript_h = '../src/zc/ffscript.h'
file_ffscript_cpp = '../src/zc/ffscript.cpp'
file_ffasm_cpp = '../src/zq/ffasm.cpp'
name = args.name
if args.reg:
    # Bytecode.cpp
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
            val = int(m.groups()[0])
            if val > last_reserved:
                last_reserved = val
            if first_reserved < 0 or val < first_reserved:
                first_reserved = val
        index = index + 1
    # whitespace for line starts
    ws = re.match('([\t ]*).*',lines[index]).groups()[0]
    # starting index number
    start = last_reserved if first_reserved < args.count else 0
    newlines = []
    for q in range(args.count):
        newlines.append(f'{ws}case RESRVD_VAR_{args.name}_{start+q+1}: return "RESRVD_VAR_{args.name}_{start+q+1}";\n')
    newlines.append(f'{ws}\n')
    
    lines[index:index] = newlines
    write_file(file_bytecode_cpp,lines)
    
    # Bytecode.h
    with open(file_bytecode_h, 'r') as file:
        lines = file.readlines()
    index = -1
    m = None
    while not m:
        index = index + 1
        m = re.match('#define LAST_BYTECODE( +)([0-9]+)',lines[index].strip())
    ws,num = m.groups()
    num = int(num)
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_VAR_{args.name}_{start+q+1}'
        numspace = (13+len(ws))-len(linename)
        if numspace < 2:
            numspace = 2
        newlines.append(f'#define {linename}{" "*numspace}{num}\n')
        num = num + 1
    newlines.append('\n')
    lines[index] = f'#define LAST_BYTECODE{ws}{num}\n'
    lines[index:index] = newlines
    write_file(file_bytecode_h,lines)
    
    # ffscript.h
    with open(file_ffscript_h, 'r') as file:
        lines = file.readlines()
    index = -1
    m = None
    while not m:
        index = index + 1
        m = re.match('#define NUMVARIABLES( +)(0x[0-9A-Fa-f]+)',lines[index].strip())
    ws,num = m.groups()
    num = int(num,16)
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_VAR_{args.name}_{start+q+1}'
        numspace = (12+len(ws))-len(linename)
        if numspace < 2:
            numspace = 2
        newlines.append(f'#define {linename}{" "*numspace}0x{num:04X}\n')
        num = num + 1
    newlines.append('\n')
    lines[index] = f'#define NUMVARIABLES{ws}0x{num:04X}\n'
    lines[index:index] = newlines
    write_file(file_ffscript_h,lines)
    
    # ffscript.cpp
    with open(file_ffscript_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_variable[ \t]+ZASMVars[ \t]*\[\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]*){ " ", -1, 0, 0 }',lines[index])
    ws = m.groups()[0]
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_VAR_{args.name}_{start+q+1}'
        newlines.append(f'{ws}{{ "{linename}", {linename}, 0, 0 }},\n')
    newlines.append('\n')
    lines[index:index] = newlines
    write_file(file_ffscript_cpp,lines)
    
    # ffasm.cpp
    with open(file_ffasm_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_variable[ \t]+variable_list[ \t]*\[\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]*){ " ", -1, 0, 0 }',lines[index])
    ws = m.groups()[0]
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_VAR_{args.name}_{start+q+1}'
        newlines.append(f'{ws}{{ "{linename}", {linename}, 0, 0 }},\n')
    newlines.append('\n')
    lines[index:index] = newlines
    write_file(file_ffasm_cpp,lines)
elif opcodetype > -1:
    name_prop = propercase(name)
    # Bytecode.cpp
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
    newlines = ['\n','\n']
    for q in range(args.count):
        newlines.append(f'string OReserved{name_prop}_{start+q+1}::toString() const\n')
        newlines.append('{\n')
        match opcodetype:
            case 2:
                newlines.append(f'\treturn "RESRVD_OP_{name}_{start+q+1} " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();\n')
            case 1:
                newlines.append(f'\treturn "RESRVD_OP_{name}_{start+q+1} " + getArgument()->toString();\n')
            case _:
                newlines.append(f'\treturn "RESRVD_OP_{name}_{start+q+1}";\n')
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
    for q in range(args.count):
        ln = f'OReserved{name_prop}_{start+q+1}'
        match opcodetype:
            case 2:
                newlines.append(f'\tclass {ln} : public BinaryOpcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\t{ln}(Argument *A, Argument* B) : BinaryOpcode(A,B) {{}}\n')
                newlines.append(f'\t\tstd::string toString() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {ln}(a->clone(),b->clone());\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
            case 1:
                newlines.append(f'\tclass {ln} : public UnaryOpcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\t{ln}(Argument *A) : UnaryOpcode(A) {{}}\n')
                newlines.append(f'\t\tstd::string toString() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {ln}(a->clone());\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
            case _:
                newlines.append(f'\tclass {ln} : public Opcode\n')
                newlines.append(f'\t{{\n')
                newlines.append(f'\tpublic:\n')
                newlines.append(f'\t\tstd::string toString() const;\n')
                newlines.append(f'\t\tOpcode* clone() const\n')
                newlines.append(f'\t\t{{\n')
                newlines.append(f'\t\t\treturn new {ln}();\n')
                newlines.append(f'\t\t}}\n')
                newlines.append(f'\t}};\n')
                newlines.append(f'\n')
    newlines.pop(-1) # remove trailing newline
    lines[index:index] = newlines
    write_file(file_bytecode_h,lines)
    
    # ffscript.h
    with open(file_ffscript_h, 'r') as file:
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
    for q in range(args.count):
        newlines.append(f'{ws}RESRVD_OP_{name}_{start+q+1},\n')
        num = num + 1
    newlines.append('\n')
    lines[index] = f'{ws}NUMCOMMANDS  //0x{num:04X}\n'
    lines[index:index] = newlines
    write_file(file_ffscript_h,lines)
    
    # ffscript.cpp
    with open(file_ffscript_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_command[ \t]+ZASMcommands[ \t]*\[[ \t]*NUMCOMMANDS[ \t]*\+[ \t]*1[ \t]*\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]*){ *"", *0, *0, *0, *0 *}',lines[index])
    ws = m.groups()[0]
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_OP_{name}_{start+q+1}'
        newlines.append(f'{ws}{{ "{linename}", 0, 0, 0, 0 }},\n')
    newlines.append('\n')
    lines[index] = f'{ws}{{ "", 0, 0, 0, 0 }}\n'
    lines[index:index] = newlines
    write_file(file_ffscript_cpp,lines)
    
    # ffasm.cpp
    with open(file_ffasm_cpp, 'r') as file:
        lines = file.readlines()
    index = 0
    while not re.match('script_command[ \t]+command_list[ \t]*\[[ \t]*NUMCOMMANDS[ \t]*\+[ \t]*1[ \t]*\][ \t]*=.*',lines[index]):
        index = index + 1
    m = None
    while not m:
        index = index + 1
        m = re.match('([ \t]*){ *"", *0, *0, *0, *0 *}',lines[index])
    ws = m.groups()[0]
    newlines = []
    for q in range(args.count):
        linename = f'RESRVD_OP_{name}_{start+q+1}'
        newlines.append(f'{ws}{{ "{linename}", 0, 0, 0, 0 }},\n')
    newlines.append('\n')
    lines[index] = f'{ws}{{ "", 0, 0, 0, 0 }}\n'
    lines[index:index] = newlines
    write_file(file_ffasm_cpp,lines)
else:
    exit(0)

