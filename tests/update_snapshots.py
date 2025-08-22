import os
import subprocess
import sys

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))


def run(path: Path):
    print(f'running {path.name} --update')
    run_args = [
        sys.executable,
        path,
        '--update',
    ]
    subprocess.check_call(run_args)


# run(script_dir / 'test_jit.py')
run(script_dir / 'test_optimize_zasm.py')
run(script_dir / 'test_optimize_zasm_unit.py')
run(script_dir / 'test_zscript.py')
run(script_dir / 'test_zscript_database.py')
