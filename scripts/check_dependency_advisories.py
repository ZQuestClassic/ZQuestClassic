#!/usr/bin/env python3

# Checks the native dependencies against the OSV database (https://osv.dev) for
# known security advisories. These are pinned by commit in places Dependabot
# does not read:
#
#   - FetchContent_Declare(...) in the CMake files
#   - third_party/*/update.sh, for the libraries vendored into third_party/
#
# Prints a markdown report, and exits with 1 if any advisory is found that is
# not listed in third_party/advisories_ignore.txt. Exits with 2 if the check
# itself failed.
#
# OSV is asked which advisories affect the pinned commit. That misses the many
# advisories recorded as "affects everything up to release X" with no fix: a
# pin that is newer than release X looks unaffected, though nothing fixed it.
# So for a pin that is not itself a release, the newest release it contains is
# checked too, and its advisories with no recorded fix are reported.
#
# A dependency with no advisories is not necessarily safe: OSV only knows about
# repositories whose commit history it has indexed. Forks are not covered, nor
# are the vendored libraries with no update.sh.

import argparse
import json
import re
import subprocess
import sys
import tempfile
import traceback
import urllib.request

from dataclasses import dataclass
from pathlib import Path

script_dir = Path(__file__).parent
root_dir = script_dir.parent
ignore_path = root_dir / 'third_party/advisories_ignore.txt'

OSV_API = 'https://api.osv.dev/v1'
SHA_RE = re.compile(r'^[0-9a-f]{40}$')


@dataclass
class Pin:
    name: str
    repo: str
    # A commit hash, or a tag name.
    ref: str
    source: str
    commit: str = ''
    # The newest release the pinned commit contains, if the pin is not one.
    release: str = ''
    release_commit: str = ''


def find_cmake_pins() -> list[Pin]:
    pins = []
    files = subprocess.check_output(
        ['git', 'ls-files', '*CMakeLists.txt', '*.cmake'], cwd=root_dir, text=True
    ).splitlines()
    for file in files:
        if file.startswith('third_party/'):
            continue

        text = (root_dir / file).read_text()
        text = re.sub(r'#.*', '', text)
        for m in re.finditer(r'FetchContent_Declare\(\s*(\S+)(.*?)\)', text, re.S):
            name, body = m.groups()
            repo = re.search(r'GIT_REPOSITORY\s+(\S+)', body)
            tag = re.search(r'GIT_TAG\s+(\S+)', body)
            if repo and tag:
                pins.append(Pin(name, repo[1].removesuffix('.git'), tag[1], file))
                continue

            # A release archive, with the version in a variable.
            url = re.search(
                r'URL\s+(https://github\.com/[^/\s]+/[^/\s]+)/releases/download/\$\{(\w+)\}',
                body,
            )
            if url:
                version = re.search(rf'set\({url[2]}\s+([^\s)]+)\)', text)
                if version:
                    pins.append(Pin(name, url[1], version[1], file))

    return pins


def find_vendored_pins() -> list[Pin]:
    pins = []
    for path in sorted((root_dir / 'third_party').glob('*/update.sh')):
        text = path.read_text()
        name = path.parent.name
        source = str(path.relative_to(root_dir))

        m = re.search(r'(https://github\.com/[^/\s]+/[^/\s]+)/archive/\$\{SHA\}', text)
        sha = re.search(r'^SHA=([0-9a-f]{40})', text, re.M)
        if m and sha:
            pins.append(Pin(name, m[1], sha[1], source))
            continue

        m = re.search(
            r'(https://github\.com/[^/\s]+/[^/\s]+)/releases/download/([^/\s]+)/', text
        )
        if m:
            pins.append(Pin(name, m[1], m[2], source))
            continue

        print(f'warning: could not find what {source} is pinned to', file=sys.stderr)

    return pins


def resolve_commit(pin: Pin) -> str:
    if SHA_RE.match(pin.ref):
        return pin.ref

    # For an annotated tag, the "^{}" line has the commit the tag points to.
    output = subprocess.check_output(
        [
            'git',
            'ls-remote',
            pin.repo,
            f'refs/tags/{pin.ref}',
            f'refs/tags/{pin.ref}^{{}}',
        ],
        text=True,
    )
    lines = output.splitlines()
    if not lines:
        raise Exception(
            f'{pin.name}: {pin.ref} is not a commit hash or a tag of {pin.repo}'
        )
    return lines[-1].split()[0]


def find_release(pin: Pin):
    # Only the commit graph is needed, so skip downloading any files.
    with tempfile.TemporaryDirectory() as dir:
        subprocess.check_call(
            ['git', 'clone', '--quiet', '--bare', '--filter=tree:0', pin.repo, dir]
        )
        result = subprocess.run(
            ['git', 'describe', '--tags', '--abbrev=0', pin.commit],
            cwd=dir,
            capture_output=True,
            text=True,
        )
        # Fails if the repository has no tags before this commit.
        if result.returncode != 0:
            return

        tag = result.stdout.strip()
        commit = subprocess.check_output(
            ['git', 'rev-list', '-n1', tag], cwd=dir, text=True
        ).strip()
        if commit != pin.commit:
            pin.release = tag
            pin.release_commit = commit


def osv_request(path: str, body=None):
    data = json.dumps(body).encode() if body is not None else None
    request = urllib.request.Request(
        f'{OSV_API}/{path}', data, {'Content-Type': 'application/json'}
    )
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def query_advisory_ids(commits: list[str]) -> list[list[str]]:
    ids = [[] for _ in commits]
    queries = {i: {'commit': commit} for i, commit in enumerate(commits)}
    while queries:
        indices = list(queries)
        body = {'queries': [queries[i] for i in indices]}
        results = osv_request('querybatch', body)['results']
        for i, result in zip(indices, results):
            ids[i] += [vuln['id'] for vuln in result.get('vulns', [])]
            if token := result.get('next_page_token'):
                queries[i]['page_token'] = token
            else:
                del queries[i]
    return ids


def has_fix(advisory: dict) -> bool:
    for affected in advisory.get('affected', []):
        for range in affected.get('ranges', []):
            if any('fixed' in event for event in range.get('events', [])):
                return True
    return False


# Returns for each pin its advisories, each with a note on why it is included.
def query_advisories(pins: list[Pin]) -> list[list[tuple[dict, str]]]:
    commits = [pin.commit for pin in pins]
    with_release = [pin for pin in pins if pin.release]
    commits += [pin.release_commit for pin in with_release]
    ids = query_advisory_ids(commits)
    release_ids = dict(zip([pin.name for pin in with_release], ids[len(pins) :]))

    results = []
    for pin, pin_ids in zip(pins, ids):
        advisories = [(osv_request(f'vulns/{id}'), '') for id in pin_ids]
        for id in release_ids.get(pin.name, []):
            if id in pin_ids:
                continue

            advisory = osv_request(f'vulns/{id}')
            if not has_fix(advisory):
                note = f'affects {pin.release}, and has no recorded fix'
                advisories.append((advisory, note))
        results.append(advisories)
    return results


# Each line is "<dependency> <advisory id> <reason>".
def load_ignores() -> dict[tuple[str, str], str]:
    ignores = {}
    for line in ignore_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue

        parts = line.split(maxsplit=2)
        if len(parts) != 3:
            raise Exception(f'{ignore_path.name}: missing a reason: {line}')
        ignores[(parts[0], parts[1])] = parts[2]
    return ignores


def main():
    parser = argparse.ArgumentParser(
        description='Checks the native dependencies for known security advisories'
    )
    parser.add_argument(
        '--output', type=Path, help='Write the report here instead of printing it'
    )
    args = parser.parse_args()

    pins = find_cmake_pins() + find_vendored_pins()
    for pin in pins:
        pin.commit = resolve_commit(pin)
        find_release(pin)

    ignores = load_ignores()
    unused_ignores = set(ignores)
    found = []
    ignored = []
    for pin, advisories in zip(pins, query_advisories(pins)):
        for advisory, note in advisories:
            ids = [advisory['id'], *advisory.get('aliases', [])]
            keys = [(pin.name, id) for id in ids if (pin.name, id) in ignores]
            unused_ignores.difference_update(keys)
            summary = advisory.get('summary') or advisory.get('details', '')
            summary = summary.strip().splitlines()[0][:150] if summary.strip() else ''
            if note:
                summary += f' ({note})'
            entry = (pin, ids, summary)
            if keys:
                ignored.append((*entry, ignores[keys[0]]))
            else:
                found.append(entry)

    lines = []
    if found:
        lines.append('## Advisories\n')
        lines.append(
            'Update the dependency, or - if the advisory does not apply - add it to'
            f' `{ignore_path.relative_to(root_dir)}`.\n'
        )
        for pin, ids, summary in found:
            url = f'https://osv.dev/vulnerability/{ids[0]}'
            lines.append(f'- **{pin.name}** [{", ".join(ids)}]({url}): {summary}')
        lines.append('')
    else:
        lines.append('No new advisories.\n')

    if ignored:
        lines.append('## Ignored\n')
        for pin, ids, summary, reason in ignored:
            lines.append(f'- **{pin.name}** {", ".join(ids)}: {reason}')
        lines.append('')

    if unused_ignores:
        lines.append('## Stale ignores\n')
        lines.append(
            f'These entries in `{ignore_path.relative_to(root_dir)}` no longer match'
            ' anything, and can be removed.\n'
        )
        for name, id in sorted(unused_ignores):
            lines.append(f'- {name} {id}')
        lines.append('')

    lines.append('## Checked\n')
    lines.append('| Dependency | Pinned to | Pinned in |')
    lines.append('|---|---|---|')
    for pin in pins:
        ref = (
            pin.commit[:12]
            if pin.ref == pin.commit
            else f'{pin.ref} ({pin.commit[:12]})'
        )
        if pin.release:
            ref += f', after {pin.release}'
        lines.append(f'| [{pin.name}]({pin.repo}) | {ref} | `{pin.source}` |')

    report = '\n'.join(lines) + '\n'
    if args.output:
        args.output.write_text(report)
    else:
        print(report)

    sys.exit(1 if found else 0)


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(2)
