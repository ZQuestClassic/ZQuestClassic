import argparse
import os
import subprocess
import shutil
import itertools
from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

tmp_folder = root_dir / '.tmp/fuzz/outputs'
if tmp_folder.exists():
    shutil.rmtree(tmp_folder)
tmp_folder.mkdir(parents=True)

state_path = root_dir / '.tmp/fuzz/state.txt'
if state_path.exists():
    state = state_path.read_text().splitlines()
else:
    state = []

parser = argparse.ArgumentParser(
    description='Runs each bad input found by AFL++ fuzzer, and saves stacktrace to disk')
parser.add_argument('--fuzz_results_folder', default='build_fuzz/results')
parser.add_argument('--build_folder', default='build_asan/Debug')

args = parser.parse_args()

build_dir = args.build_folder
results_dir = Path(args.fuzz_results_folder)

i = 0
for file in itertools.chain(results_dir.rglob('crashes/id:*'), results_dir.rglob('hangs/id:*')):
    file = file.absolute()
    print(file)
    if str(file) in state:
        print('KNOWN OK')
        continue

    timed_out = False
    try:
        output = subprocess.run(['./zelda', '-load-and-quit', file],
                                env={**os.environ, 'CI': '1'}, timeout=3,
                                cwd=build_dir, encoding='utf-8', stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        if output.returncode >= 0:
            print('OK')
            state.append(str(file))
            state_path.write_text('\n'.join(state))
            continue
    except subprocess.TimeoutExpired as e:
        timed_out = True

    command = f'{build_dir}/zelda -load-and-quit {file}'

    if timed_out:
        print('TIMEOUT!')
        text = f'{command}\n\nTIMEOUT'
    else:
        print('CRASH!')
        text = f'{command}\n\n{output.stderr}'

    i += 1
    out_path = tmp_folder / f'{i}.txt'
    out_path.write_text(text)
    print(f'!!! written to {out_path}')
