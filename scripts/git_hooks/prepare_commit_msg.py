import sys
import subprocess
from pathlib import Path
from common import valid_types, valid_scopes

if len(sys.argv) != 2:
    exit(0)


def get_comment_char():
    try:
        return subprocess.check_output(
            'git config core.commentchar', shell=True, encoding='utf-8'
        ).strip()
    except:
        return '#'


commit_msg_path = Path(sys.argv[1])
commit_msg = commit_msg_path.read_text()

comment_char = get_comment_char()
commit_msg += f'{comment_char} example commit - type(scope): details'
commit_msg += '\n'
commit_msg += f'{comment_char} valid types    - '
commit_msg += ', '.join(valid_types)
commit_msg += '\n'
commit_msg += f'{comment_char} valid scopes   - '
commit_msg += ', '.join(valid_scopes)

changed_files = subprocess.check_output(
    'git diff --cached --name-only --diff-filter=ACM', shell=True, encoding='utf-8'
).splitlines()
changed_files = [f.lower() for f in changed_files]
suggested_types = []
suggested_scopes = []

if any(f.startswith('tests') for f in changed_files):
    suggested_types.append('test')
if any(f.startswith('.github') for f in changed_files):
    suggested_types.append('ci')
if any(f.startswith('docs') for f in changed_files):
    suggested_types.append('docs')
if any(f.startswith('cmake') for f in changed_files):
    suggested_types.append('build')

if all(f.startswith('src/zc') for f in changed_files):
    suggested_scopes.append('zc')
if all(f.startswith('src/zq') for f in changed_files):
    suggested_scopes.append('zq')
if all(f.startswith('src/launcher') for f in changed_files):
    suggested_scopes.append('launcher')
if all(f.startswith('src/parser') for f in changed_files):
    suggested_scopes.append('zscript')
if all(f.startswith('src/zc/ffscript') for f in changed_files):
    suggested_scopes.append('zscript')
if all(f.startswith('src/zc/zconsole') for f in changed_files):
    suggested_scopes.append('zconsole')
if all(f.startswith('vscode-extension') for f in changed_files):
    suggested_scopes.append('vscode')

note = 'The above generated commit prefix is just a suggestion, change as needed'
if len(suggested_types) == 1 and len(suggested_scopes) == 1:
    prefix = f'{suggested_types[0]}({suggested_scopes[0]}): '
    commit_msg = f'{prefix}\n{comment_char} {note}\n{comment_char}{commit_msg}'
elif len(suggested_types) == 1:
    prefix = f'{suggested_types[0]}: '
    note = f'{note}\n{comment_char} no scope could be suggested. If one applies, please add it!'
    commit_msg = f'{prefix}\n{comment_char} {note}\n{comment_char}{commit_msg}'

if suggested_types:
    commit_msg += f'\n{comment_char}'
    commit_msg += f'\n{comment_char} suggested types: '
    commit_msg += ', '.join(suggested_types)

if suggested_scopes:
    commit_msg += f'\n{comment_char}'
    commit_msg += f'\n{comment_char} suggested scopes: '
    commit_msg += ', '.join(suggested_scopes)

commit_msg += '\n'
commit_msg_path.write_text(commit_msg)
