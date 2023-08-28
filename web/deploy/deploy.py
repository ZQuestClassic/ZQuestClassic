import argparse
import os
import shutil
import re
from pathlib import Path
import itertools

parser = argparse.ArgumentParser()
parser.add_argument('--build_folder')
parser.add_argument('--prepare', action='store_true')
parser.add_argument('--deploy', action='store_true')
parser.add_argument('--prod', action='store_true')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
dist_dir = script_dir / 'dist'
root_dir = script_dir / '../..'


def expand_braces(text, seen=None):
    """ https://stackoverflow.com/a/71330479/2788187 """

    if seen is None:
        seen = set()

    spans = [m.span() for m in re.finditer("\{[^\{\}]*\}", text)][::-1]
    alts = [text[start + 1: stop - 1].split(",") for start, stop in spans]

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
    for file in expand_braces(os.path.join(args.build_folder, pattern)):
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
    if not pattern in string:
        raise Exception(f'did not find pattern <{pattern}> in file <{file}>')
    path.write_text(string.replace(pattern, replace_with))


if args.prepare:
    if not args.build_folder:
        raise Exception('missing --build_folder')

    if dist_dir.exists():
        shutil.rmtree(dist_dir)
    os.mkdir(dist_dir)
    os.mkdir(dist_dir / 'play')
    os.mkdir(dist_dir / 'create')

    copy_from_build('zelda.html', 'play/index.html')
    copy_from_build('zelda.js', 'play')
    copy_from_build('zplayer.{wasm,worker.js}', 'play')

    copy_from_build('zquest.html', 'create/index.html')
    copy_from_build('zquest.{wasm,js,worker.js}', 'create')
    copy_from_build('{zq.data,zq.data.js}', 'create')

    copy_from_build('zscript.{wasm,mjs,worker.js}', '')
    copy_from_build('{zc.data,zc.data.js}', '')
    copy_from_build('{main.js,main.js.map}', '')

    copy_recursively(script_dir / 'icons', dist_dir)
    shutil.copytree(root_dir / 'timidity', dist_dir / 'timidity')
    shutil.copy2(script_dir / 'manifest.json', dist_dir)

    for file in ['play/index.html', 'create/index.html']:
        for partial_file in ['meta.html', 'ga.html', 'manifest.html']:
            content = (script_dir / 'partials' / partial_file).read_text()
            replace_in(file, '</head>', f'{content}</head>')

        replace_in(file, "zelda.html", "/zc/play/")
        replace_in(file, "zquest.html", "/zc/create/")
        replace_in(file, "'timidity'", "'../timidity'")
        replace_in(file, "zc.data.js", "../zc.data.js")
        replace_in(file, "main.js", "../main.js")

    replace_in('zc.data.js', "REMOTE_PACKAGE_BASE = 'zc.data'",
               "REMOTE_PACKAGE_BASE = '../zc.data'")

    shutil.copy2(script_dir / 'workbox-config.js', dist_dir)
    os.chdir(dist_dir)
    os.system(
        f'{script_dir / "../node_modules/.bin/workbox"} generateSW workbox-config.js')

    (dist_dir / 'index.html').write_text('<a href=create>create</a> <br> <a href=play>play</a>')


if args.deploy:
    if not dist_dir.exists():
        raise Exception('you must run --prepare first')

    deploy_folder = 'zc' if args.prod else 'zc-debug'
    os.system(
        f'ssh root@hoten.cc "mkdir -p /var/www/hoten.cc/public_html/{deploy_folder}/"')
    os.system(
        f'rsync -ahvzL --delete {dist_dir}/ root@hoten.cc:/var/www/hoten.cc/public_html/{deploy_folder}/')


if not args.prepare and not args.deploy:
    raise Exception('missing --prepare or --deploy')
