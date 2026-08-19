import functools
import os
import subprocess

from pathlib import Path

from joblib import Memory

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
memory = Memory(root_dir / '.tmp/memory_git_helpers', verbose=0)


# Cached only within a single run: what a ref points to can change between
# runs (tags get re-pushed, branches move), so this must never go in a
# persistent cache.
@functools.cache
def rev_parse(commitish: str):
    return subprocess.check_output(
        ['git', 'rev-parse', commitish], encoding='utf-8'
    ).strip()


@memory.cache
def _is_ancestor(ancestor_sha: str, sha: str):
    # Keyed by resolved shas, so the cached relation is immutable.
    p = subprocess.run(['git', 'merge-base', '--is-ancestor', ancestor_sha, sha])
    if p.returncode == 0:
        return True
    if p.returncode == 1:
        return False
    # Ex: one of the commits doesn't exist locally. Raising keeps this
    # transient failure out of the cache.
    raise Exception(f'git merge-base failed with code {p.returncode}')


def is_ancestor(maybe_ancestor: str, commitish: str):
    return _is_ancestor(rev_parse(maybe_ancestor), rev_parse(commitish))


@memory.cache
def _first_merge_into_branch(sha: str, branch: str):
    # Keyed by resolved sha; a found merge never changes as the branch
    # advances. The "no merge yet" answer can change, so it raises and is
    # not cached.
    args = f'git rev-list {sha}..{branch} --ancestry-path --merges --reverse'.split(' ')
    lines = subprocess.check_output(args, encoding='utf-8').splitlines()
    if not lines:
        raise LookupError(f'no merge of {sha} into {branch}')
    return lines[0]


def tag_to_sha_on_branch(tag: str, branch: str):
    sha = rev_parse(tag)
    try:
        return _first_merge_into_branch(sha, branch)
    except LookupError:
        return sha
