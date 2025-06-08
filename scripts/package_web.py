import argparse
import itertools
import os
import re
import shutil

from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', required=True)
parser.add_argument('--target')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
build_dir = Path(args.build_folder)
dist_dir = build_dir / 'packages/web'
root_dir = script_dir / '..'
web_dir = root_dir / 'web'

targets = [args.target] if args.target else ['zplayer', 'zeditor', 'playground']


def expand_braces(text, seen=None):
    """https://stackoverflow.com/a/71330479/2788187"""

    if seen is None:
        seen = set()

    spans = [m.span() for m in re.finditer(r'\{[^\{\}]*\}', text)][::-1]
    alts = [text[start + 1 : stop - 1].split(",") for start, stop in spans]

    if len(spans) == 0:
        if text not in seen:
            yield text
        seen.add(text)

    else:
        for combo in itertools.product(*alts):
            replaced = list(text)
            for (start, stop), replacement in zip(spans, combo):
                replaced[start:stop] = replacement

            yield from expand_braces("".join(replaced), seen)


def copy_from_build(pattern, dest):
    for file in expand_braces(os.path.join(build_dir, pattern)):
        shutil.copy2(file, dist_dir / dest)


def copy_from_build_if_exists(pattern, dest):
    for file in expand_braces(os.path.join(build_dir, pattern)):
        if os.path.exists(file):
            shutil.copy2(file, dist_dir / dest)


def copy_recursively(src, dst, symlinks=False, ignore=None):
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            shutil.copytree(s, d, symlinks, ignore)
        else:
            shutil.copy2(s, d)


def replace_in(file, pattern, replace_with):
    path = dist_dir / file
    string = path.read_text()
    if not re.search(pattern, string):
        raise Exception(f'did not find pattern <{pattern}> in file <{file}>')
    path.write_text(re.sub(pattern, replace_with, string))


if dist_dir.exists():
    for f in dist_dir.glob('*'):
        path = dist_dir / f
        if path.is_file() or path.is_symlink():
            path.unlink()
        else:
            shutil.rmtree(path)
else:
    dist_dir.mkdir(parents=True)

for f in dist_dir.rglob('*-e'):
    f.unlink()

code = os.system(
    f'cd {dist_dir} && {web_dir / "node_modules/.bin/workbox"} generateSW {web_dir / "workbox-config.cjs"}'
)
if code != 0:
    print('failed to make sw.js')
    exit(code)

copy_from_build('{main.js,main.js.map}', '')
copy_from_build('{zplayer.data,zplayer.data.js}', '')

if 'zplayer' in targets:
    (dist_dir / 'play').mkdir(exist_ok=True)
    shutil.copy2(build_dir / 'zplayer.html', dist_dir / 'play/index.html')
    copy_from_build('zplayer.{wasm,js}', '')
    replace_in(
        'zplayer.js',
        r'if ?\(SDL2.audio.scriptProcessorNode',
        'if(SDL2.audio?.scriptProcessorNode',
    )

if 'zeditor' in targets:
    (dist_dir / 'create').mkdir(exist_ok=True)
    shutil.copy2(build_dir / 'zeditor.html', dist_dir / 'create/index.html')
    copy_from_build('zeditor.{wasm,js}', '')
    copy_from_build('{zeditor.data,zeditor.data.js}', '')
    replace_in(
        'zeditor.js',
        r'if ?\(SDL2.audio.scriptProcessorNode',
        'if(SDL2.audio?.scriptProcessorNode',
    )

if 'playground' in targets:
    if (dist_dir / 'zscript').exists():
        shutil.rmtree(dist_dir / 'zscript')
    shutil.copytree(build_dir / 'playground', dist_dir / 'zscript')
    copy_from_build('{zscript-playground.data,zscript-playground.data.js}', 'zscript')


copy_from_build_if_exists('zplayer.wasm.debug.wasm', '')
copy_from_build_if_exists('zeditor.wasm.debug.wasm', '')
copy_from_build_if_exists('zscript.wasm.debug.wasm', '')
copy_from_build_if_exists('zscript.{wasm,mjs}', '')

copy_recursively(web_dir / 'icons', dist_dir)
shutil.copy2(web_dir / 'manifest.json', dist_dir)
shutil.copy2(web_dir / 'styles.css', dist_dir)
shutil.copytree(root_dir / 'timidity', dist_dir / 'timidity')
shutil.copytree(build_dir / 'packages/web_lazy_files', dist_dir / 'files')

(dist_dir / 'index.html').write_text(
    '<a href=create>create</a> <br> <a href=play>play</a> <br> <a href=zscript>zscript playground</a>'
)
