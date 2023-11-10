import sys
import re
from pathlib import Path
from common import valid_types, valid_scopes

commit_msg_path = Path(sys.argv[1])
commit_msg = commit_msg_path.read_text()
is_valid = True

log = ''

if commit_msg.startswith('fixup!'):
    exit(0)

def add_log(*args):
    global log
    message = ' '.join(args)
    log += message
    log += '\n'


# Prefix is required, scope is optional.
first_line = commit_msg.splitlines()[0]
match = re.match(r'(\w+)\((\w+)\)[!<]?: (.+)',
                 first_line) or re.match(r'(\w+)[!<]?:( )(.+)', first_line)
if not match:
    add_log('commit message must match expected pattern, using an expected type and optional scope.')
    add_log('examples:\n\tfix: fix the thing\n\tfix(zc): fix the thing\n')
    add_log(f'valid types (the first word of the commit) are:',
            ', '.join(valid_types))
    add_log(f'valid scopes (the optional text in parentheses) are:',
            ', '.join(valid_scopes))
    add_log('A "!" may be added before the ":" to remove the commit from the changelog.')
    add_log('A "<" may be added before the ":" to squash the commit into the previous in the changelog.')
    add_log()
    is_valid = False
else:
    type, scope, oneliner = match.groups()
    scope = scope.strip()

    is_valid = True
    if type not in valid_types:
        add_log(
            f'invalid type: {type}\nMust be one of: {", ".join(valid_types)}')
        is_valid = False
    if scope and scope not in valid_scopes:
        add_log(
            f'invalid scope: {scope}\nMust be one of: {", ".join(valid_scopes)}')
        is_valid = False

if not is_valid:
    print(commit_msg)
    print('============== FAILED TO COMMIT ==================')
    print(log)
    print('commit message is not valid, please rewrite it')
    print('to skip this validation, commit again with the --no-verify flag')
    print('for reference, the commit message is above')
    exit(1)
