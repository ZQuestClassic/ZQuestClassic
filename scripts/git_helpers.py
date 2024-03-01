from joblib import Memory
from pathlib import Path
import os
import subprocess

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
memory = Memory(root_dir / '.tmp/memory_git_helpers', verbose=0)


@memory.cache
def rev_parse(commitish: str):
    return subprocess.check_output(
        ['git', 'rev-parse', commitish], encoding='utf-8'
    ).strip()


@memory.cache
def is_ancestor(maybe_ancestor_sha: str, sha: str):
    return (
        subprocess.call(
            ['git', 'merge-base', '--is-ancestor', maybe_ancestor_sha, sha],
            encoding='utf-8',
        )
        == 0
    )


@memory.cache
def tag_to_sha_on_main(tag: str):
    args = f'git rev-list {tag}..main --ancestry-path --merges --reverse'.split(' ')
    out = subprocess.check_output(args, encoding='utf-8')
    lines = out.splitlines()
    if not lines:
        return rev_parse(tag)
    return lines[0]
