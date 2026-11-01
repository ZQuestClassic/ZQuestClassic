#!/usr/bin/env python3

# Regenerates the animations of trigger_example.qst rooms shown in the combo triggers guide
# (docs-www/source/tutorials/combo_triggers.rst) by playing replays and capturing frames.
#
# Run it after changing the example quest or its replays:
#
#   python scripts/update_trigger_example_animations.py --build_folder build/Release
#
# The animations are lossless animated WebP at the game's full 60 fps. That is much smaller
# than GIF, which also can't time frames finer than 1/100 s.

import argparse
import subprocess
import tempfile

from pathlib import Path

from PIL import Image

root_dir = Path(__file__).resolve().parent.parent
replays_dir = root_dir / 'tests/replays'
out_dir = root_dir / 'docs-www/source/tutorials/combo_triggers'

# (animation name, replay relative to tests/replays, first frame, last frame). The rooms that
# misc/triggers2.zplay already plays through are cut from it; the others have a replay of
# their own in tests/replays/trigger_example.
#
# Keep transitions out of the clips: the test-mode replays open with a wipe that is done by
# frame 67, and a screen takes 64 frames to scroll in sideways, 44 up or down. To find the
# frames, the replays' "C <frame> scr=" comments say when the screen changes.
CLIPS = [
    ('room_00', 'misc/triggers2.zplay', 60, 1790),
    ('room_01', 'misc/triggers2.zplay', 1858, 2800),
    ('room_02', 'misc/triggers2.zplay', 2871, 3450),
    ('room_03', 'misc/triggers2.zplay', 3518, 4395),
    ('room_04', 'trigger_example/room_04.zplay', 70, 325),
    ('room_10', 'misc/triggers2.zplay', 5094, 5845),
    ('room_12', 'misc/triggers2.zplay', 11152, 12450),
    ('room_13', 'trigger_example/room_13.zplay', 70, 715),
    ('room_14', 'trigger_example/room_14.zplay', 70, 755),
    ('room_15', 'trigger_example/room_25.zplay', 979, 1295),
    ('room_16', 'trigger_example/room_16.zplay', 70, 275),
    ('room_17', 'trigger_example/room_17.zplay', 70, 545),
    ('room_18', 'trigger_example/room_18.zplay', 70, 715),
    ('room_20', 'misc/triggers2.zplay', 7185, 7838),
    ('room_25', 'trigger_example/room_25.zplay', 70, 930),
    ('room_26', 'trigger_example/room_26.zplay', 70, 235),
]

# The animation at the top of the guide: four rooms playing at once in a 2x2 grid. Each tile
# is (replay, first frame), and all of them play for HEADER_LENGTH frames.
HEADER_TILES = [
    ('trigger_example/room_04.zplay', 140),
    ('trigger_example/room_13.zplay', 130),
    ('trigger_example/room_16.zplay', 90),
    ('trigger_example/room_18.zplay', 160),
]
# Room 16's replay ends at frame 280.
HEADER_LENGTH = 180
# Transparent space between the tiles, so the grid suits a light or dark page.
HEADER_GAP = 4

# The playfield, without the 56 pixel subscreen at the top.
CROP = (0, 56, 256, 232)
# WebP frame durations are whole milliseconds, so 1/60 s alternates 17, 17 and 16 ms to
# stay at the game's real speed.
FRAME_DURATIONS = [17, 17, 16]


def capture(zplayer: Path, replay: str, frames: str, work_dir: Path):
    # stdout only lists the saved snapshots; errors go to stderr.
    subprocess.run(
        [
            str(zplayer),
            '-headless',
            '-replay',
            str(replays_dir / replay),
            '-replay-exit-when-done',
            '-replay-output-dir',
            str(work_dir),
            '-snapshot',
            frames,
        ],
        cwd=zplayer.parent,
        check=True,
        stdout=subprocess.DEVNULL,
    )


def load_frames(png_prefix: Path, first: int, last: int, name: str) -> list[Image.Image]:
    pngs = [
        png_prefix.with_name(f'{png_prefix.name}.{f}.png')
        for f in range(first, last + 1)
    ]
    missing = [p.name for p in pngs if not p.exists()]
    if missing:
        raise SystemExit(
            f'{name}: replay did not produce {missing[0]} (did it end early?)'
        )
    return [Image.open(p).convert('RGBA').crop(CROP) for p in pngs]


def save_webp(images: list[Image.Image], webp: Path):
    durations = [FRAME_DURATIONS[i % len(FRAME_DURATIONS)] for i in range(len(images))]
    # These are libwebp's own defaults (as used by its img2webp tool), which compress these
    # frames best; Pillow's defaults differ. Identical frames in a row are merged.
    images[0].save(
        webp,
        save_all=True,
        append_images=images[1:],
        duration=durations,
        loop=0,
        lossless=True,
        quality=75,
        method=4,
        minimize_size=True,
    )


def make_webp(png_prefix: Path, first: int, last: int, webp: Path):
    images = load_frames(png_prefix, first, last, webp.name)
    save_webp([im.convert('RGB') for im in images], webp)


def make_header(png_prefixes: dict[str, Path], webp: Path):
    tiles = [
        load_frames(png_prefixes[replay], first, first + HEADER_LENGTH - 1, webp.name)
        for replay, first in HEADER_TILES
    ]
    w = CROP[2] - CROP[0]
    h = CROP[3] - CROP[1]
    size = (w * 2 + HEADER_GAP, h * 2 + HEADER_GAP)
    images = []
    for i in range(HEADER_LENGTH):
        frame = Image.new('RGBA', size, (0, 0, 0, 0))
        for t, tile in enumerate(tiles):
            frame.paste(tile[i], ((t % 2) * (w + HEADER_GAP), (t // 2) * (h + HEADER_GAP)))
        images.append(frame)
    save_webp(images, webp)


def main():
    parser = argparse.ArgumentParser(
        description='Regenerate the trigger_example.qst animations for the docs.'
    )
    parser.add_argument(
        '--build_folder',
        default='build/Release',
        help='Folder with the zplayer executable.',
    )
    parser.add_argument(
        '--only',
        action='append',
        choices=[c[0] for c in CLIPS] + ['header'],
        help='Only make these animations.',
    )
    args = parser.parse_args()

    zplayer = (root_dir / args.build_folder / 'zplayer').resolve()
    if not zplayer.exists():
        zplayer = zplayer.with_suffix('.exe')
    if not zplayer.exists():
        raise SystemExit(f'zplayer not found in {args.build_folder}')

    clips = [c for c in CLIPS if not args.only or c[0] in args.only]
    header = not args.only or 'header' in args.only
    out_dir.mkdir(parents=True, exist_ok=True)

    # Play each replay once, capturing every frame any of the animations needs.
    by_replay = {}
    for name, replay, first, last in clips:
        by_replay.setdefault(replay, []).append((first, last))
    if header:
        for replay, first in HEADER_TILES:
            by_replay.setdefault(replay, []).append((first, first + HEADER_LENGTH - 1))

    with tempfile.TemporaryDirectory() as tmp:
        png_prefixes = {}
        for i, (replay, ranges) in enumerate(by_replay.items()):
            work_dir = Path(tmp) / str(i)
            work_dir.mkdir()
            print(f'playing {replay}')
            capture(
                zplayer,
                replay,
                ' '.join(f'{first}-{last}' for first, last in ranges),
                work_dir,
            )
            png_prefixes[replay] = work_dir / Path(replay).name

        webps = []
        for name, replay, first, last in clips:
            webp = out_dir / f'{name}.webp'
            make_webp(png_prefixes[replay], first, last, webp)
            webps.append(webp)
        if header:
            webp = out_dir / 'header.webp'
            make_header(png_prefixes, webp)
            webps.append(webp)
        for webp in webps:
            print(f'  {webp.relative_to(root_dir)} ({webp.stat().st_size // 1024} KB)')

if __name__ == '__main__':
    main()
