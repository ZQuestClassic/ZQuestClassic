# Builds the seed corpora for the zplayer fuzz targets, under .tmp/fuzz/seeds/<target>.
# Run build_fuzzer.sh first. See docs/fuzzing.md.

import argparse
import os
import shutil
import subprocess

from pathlib import Path

root = Path(__file__).parent.parent.parent
parser = argparse.ArgumentParser()
parser.add_argument('--build_folder', default=root / 'build_fuzz', type=Path)
parser.add_argument('--out', default=root / '.tmp/fuzz/seeds', type=Path)
parser.add_argument(
    '--quest_database',
    default=root / '.tmp/database/quests',
    type=Path,
    help='Optional. Music files from quest folders here (see zc-database) seed the music target.',
)
args = parser.parse_args()

zplayer = args.build_folder / 'zplayer'
if not zplayer.exists():
    raise SystemExit(
        f'{zplayer} does not exist - run scripts/fuzzing/build_fuzzer.sh first'
    )

# Big inputs make every run slow, and mutations rarely reach past the first few MB anyway.
MAX_UNPACKED_SIZE = 8 * 1024 * 1024


def reset_dir(name: str) -> Path:
    path = args.out / name
    shutil.rmtree(path, ignore_errors=True)
    path.mkdir(parents=True)
    return path


def unpack(src: Path, dest: Path) -> bool:
    env = {**os.environ, 'ZC_FUZZ': 'unpack'}
    p = subprocess.run(
        [zplayer, src, dest], env=env, cwd=args.build_folder, capture_output=True
    )
    if p.returncode != 0 or not dest.exists():
        print(f'  could not unpack {src}')
        return False
    if dest.stat().st_size > MAX_UNPACKED_SIZE:
        dest.unlink()
        return False
    return True


def tests_files(pattern: str) -> list[Path]:
    files = [p for p in (root / 'tests').rglob(pattern) if '.backups' not in str(p)]
    return sorted(files, key=lambda p: p.stat().st_size)


# qst: every test quest, uncompressed. They span 1.9x through 3.0, so the old-version
# branches of the loader get seeds too.
out = reset_dir('qst')
for qst in tests_files('*.qst'):
    unpack(qst, out / f'{qst.stem}.qst')
print(f'qst: {len(list(out.iterdir()))} seeds')

# sav: one save per quest is plenty; the rest of a series differ only in progress.
out = reset_dir('sav')
seen = set()
for sav in tests_files('*.sav'):
    series = sav.stem.split('_of_')[0].rsplit('_', 1)[0]
    if series in seen:
        continue
    seen.add(series)
    unpack(sav, out / f'{sav.stem}.sav')
print(f'sav: {len(list(out.iterdir()))} seeds')

# zplay: the smallest replays, cut short. Every step type shows up early in a replay.
out = reset_dir('zplay')
for zplay in tests_files('*.zplay')[:150]:
    data = zplay.read_bytes()[: 64 * 1024]
    (out / zplay.name).write_bytes(data)
print(f'zplay: {len(list(out.iterdir()))} seeds')

# music: first byte picks the decoder (see fuzz_music in src/zc/fuzz.cpp).
out = reset_dir('music')
kinds = {
    '.ogg': 0,
    '.mp3': 1,
    '.nsf': 2,
    '.spc': 2,
    '.gbs': 2,
    '.vgm': 2,
    '.gym': 2,
    '.mid': 3,
}
music_files = list((root / 'resources').rglob('*'))
if args.quest_database.exists():
    music_files += list(args.quest_database.rglob('music/*'))
counts = {}
for f in music_files:
    ext = f.suffix.lower()
    kind = kinds.get(ext)
    if kind is None or not f.is_file():
        continue
    # A few of each kind is enough to seed it.
    if counts.get(ext, 0) >= 15:
        continue
    # Decoders read streams front to back, so a cut-off file still exercises them.
    data = f.read_bytes()[: 256 * 1024]
    (out / f'{sum(counts.values())}{f.suffix}').write_bytes(bytes([kind]) + data)
    counts[ext] = counts.get(ext, 0) + 1
print(f'music: {counts}')
