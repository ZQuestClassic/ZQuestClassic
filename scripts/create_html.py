import argparse
import json
import os
import re

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
web_dir = root_dir / 'web'

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder')
parser.add_argument('--target')
args = parser.parse_args()

html_template = (web_dir / 'index.html').read_text()
CI = 'CI' in os.environ
out_file = Path(args.build_folder) / f'{args.target}.html'


def replace(string, pattern, replace_with):
    if not hasattr(pattern, 'match'):
        if not pattern in string:
            raise Exception(f'did not find substring <{pattern}>')
        return string.replace(pattern, replace_with)

    if not re.search(pattern, string):
        raise Exception(f'did not find pattern <{pattern}>')
    return re.sub(pattern, replace_with, string)


html = html_template
html = replace(html, '__TARGET__', args.target)
html = replace(html, '/*__INLINECSS__*/', (web_dir / 'styles.css').read_text())
html = replace(html, '__IS_CI__', 'true' if CI else 'false')

scripts = [
    f'<script src="../{args.target}.js"></script>',
]
if args.target == 'zplayer':
    scripts.append('<script src="../zplayer.data.js"></script>')
elif args.target == 'zeditor':
    scripts.extend(
        [
            '<script src="../zplayer.data.js"></script>',
            '<script src="../zeditor.data.js"></script>',
        ]
    )
html = replace(html, '__SCRIPTS__', '\n'.join(scripts))

lazy_files_dir = Path(args.build_folder) / 'packages/web_lazy_files'
lazy_files = [
    f'/{p.relative_to(lazy_files_dir)}'
    for p in lazy_files_dir.rglob('*')
    if os.path.isfile(p)
]
html = replace(html, '__FILES__', json.dumps(lazy_files))

out_file.write_text(html)
