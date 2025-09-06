import argparse
import os
import run_target
import sys

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
resource_dir = script_dir / '..' /  'resources'
dest_file = resource_dir / 'include' / 'bindings' / 'qrs.zh'

sys.path.append(str((root_dir / 'scripts').absolute()))

parser = argparse.ArgumentParser()
parser.add_argument(
    '--build_folder', help='The location of the build folder. ex: build/Release'
)
parser.add_argument(
    '--output', help='The path to output the file', default=dest_file
)
args = parser.parse_args()
if args.output:
    dest_file = args.output
build_folder = None
if args.build_folder:
    build_folder = Path(args.build_folder).absolute()

def output_qr_enum_file():
    run_target.check_run('zeditor', [
        '-dev-qrs-zscript',
        dest_file,
        '-q',
        '-headless',
    ], build_folder)

if __name__ == '__main__':
    output_qr_enum_file()
