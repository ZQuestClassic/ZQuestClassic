# Splits a replay into individual replays at every point where the original hit init_game.
#   python scripts/split_replay.py --replay tests/replays/nes_remastered.zplay --output-folder tests/replays/nes_remastered
#
# This helps break up really long replays which can be hard to debug when an assert fails really deep into it.
# Until the day where we can generate arbitrary save states (probably at specific boundraries - like in loadscr),
# splitting at init_game is the best we can do.
#
# If splitting a replay test, be sure to delete the original and the qst file (which was copied into the output folder).

import argparse
import os
import re
import shutil

from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

import run_target

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir / '..'
tmp_dir = root_dir / '.tmp/split_replay'


@dataclass
class ReplayStep:
    type: str
    frame: int
    data: str


@dataclass
class ReplayPart:
    steps: list[ReplayStep] = field(default_factory=list)
    absolute_start_frame: int = 0
    save_file: Optional[Path] = None
    last_key_step: Optional[ReplayStep] = None

    def duration(self) -> int:
        if not steps:
            return 0

        return steps[-1].frame - steps[0].frame


def split_replay(
    replay_path: Path,
    output_folder: Path,
    skip_save_file_generation: bool,
    split_threshold: int,
):
    meta = []
    replay_parts = [ReplayPart()]
    current_part = replay_parts[0]
    previous_part = None
    qst_path = Path()

    with replay_path.open('r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line.startswith('M'):
                meta.append(line)
                _, key, value = line.split(' ', 2)
                if key == 'qst':
                    qst_path = replay_path.parent / value
                continue

            type, frame, data = line.split(' ', 2)
            step = ReplayStep(type, int(frame), data)

            if type == 'K':
                current_part.last_key_step = step
                if previous_part and previous_part.last_key_step.data == step.data:
                    continue

            # We split strictly at init_game.
            # We track the absolute start frame so we can find the matching .sav file later.
            if type == 'C' and data == 'init_game' and step.frame != 0:
                if current_part.duration() > split_threshold:
                    previous_part = current_part
                    current_part = ReplayPart(absolute_start_frame=step.frame)
                    replay_parts.append(current_part)

            current_part.steps.append(step)

    total = len(replay_parts)
    if total == 1:
        raise Exception(
            'Nothing to split. Either the replay file has no "init_game" commands, or they did not meet the split threshold.'
        )
    print(f'will split into {total} replays')

    build_folder = run_target.get_build_folder()
    saves_folder = build_folder / 'saves/current_replay'
    if not skip_save_file_generation:
        if saves_folder.exists():
            shutil.rmtree(saves_folder)
        print('running replay w/ -replay-save-games, this may take a few minutes ...')
        run_target.run(
            'zplayer',
            [
                '-headless',
                '-v0',
                '-replay-exit-when-done',
                '-replay-save-games',
                '-replay',
                replay_path.absolute(),
            ],
            build_folder,
        )

    # Find all .sav files and extract their frame numbers from the filename (e.g. zc_frame_28709.sav).
    save_files_map = []
    for p in saves_folder.rglob('*.sav'):
        match = re.search(r'_frame_(\d+)', p.name)
        if match:
            frame = int(match.group(1))
            save_files_map.append((frame, p))
        else:
            raise Exception(f'Could not parse frame number from save file: {p.name}')

    # Sort saves by frame ascending to ensure we always grab the latest valid one.
    save_files_map.sort(key=lambda x: x[0])

    # Assign the correct save file to each part.
    for i, part in enumerate(replay_parts):
        if i == 0:
            continue  # Part 0 uses no save file.

        # Find the most recent save file generated before (or exactly at) this part's start frame.
        assigned_save = None
        for save_frame, save_path in save_files_map:
            if save_frame <= part.absolute_start_frame:
                assigned_save = save_path
            else:
                break  # Since it's sorted, we can stop checking once save_frame exceeds our start frame.

        part.save_file = assigned_save
        if not part.save_file:
            raise Exception(
                f'No save file found for segment starting at frame {part.absolute_start_frame}'
            )

    maxcol1 = len(str(total))
    for i, part in enumerate(replay_parts):
        col1 = str(i + 1).rjust(maxcol1, ' ')
        save_name = part.save_file.name if part.save_file else 'None'
        print(f'[{col1}] length: {len(part.duration())} | save: {save_name}')

    output_folder.mkdir(exist_ok=True)
    if qst_path.exists():
        shutil.copy(qst_path, output_folder)

    most_recent_key_step = None
    for i, part in enumerate(replay_parts):
        is_first = i == 0
        is_last = i == len(replay_parts) - 1
        digits_len = len(str(total))
        number_part = f'{(i + 1):0{digits_len}}_of_{total}'
        output_replay = output_folder / f'{replay_path.stem}_{number_part}.zplay'

        first_frame = part.steps[0].frame
        for step in part.steps:
            step.frame -= first_frame

        # The hero position is only emitted if the previous frame had a different value.
        # When splitting a replay, the previous part's last hero position should be added
        # on the first frame.
        if not is_first:
            # If the first frame does have a hero position comment, then we are expecting this
            # new value. Do nothing.
            has_frame_zero_position = False
            for step in part.steps:
                if step.frame != 0:
                    break

                if step.type == 'C' and step.data.startswith('h '):
                    has_frame_zero_position = True
                    break

            last_hero_position = None
            for step in reversed(replay_parts[i - 1].steps):
                if step.type == 'C' and step.data.startswith('h '):
                    last_hero_position = step.data.replace('h ', '')
                    break

            if not has_frame_zero_position and last_hero_position:
                inserted = False
                for j, step in enumerate(part.steps):
                    if step.frame != 0:
                        break
                    if step.type == 'C' and step.data.startswith('scr='):
                        part.steps.insert(
                            j + 1, ReplayStep('C', 0, f'h {last_hero_position}')
                        )
                        inserted = True
                        break

                if not inserted:
                    raise Exception(
                        f'[{output_replay.name}] could not find where to insert hero position'
                    )

        with output_replay.open('w', newline='\n') as f:
            for line in meta:
                if line.startswith('M frames'):
                    f.write(f'M frames {part.steps[-1].frame}')
                    f.write('\n')
                    continue
                if line.startswith('M length'):
                    f.write(f'M length {len(part.steps)}')
                    f.write('\n')
                    continue

                f.write(line)
                f.write('\n')
                if line.startswith('M qst '):
                    if not is_first and part.save_file:
                        save_file_name = f'{output_replay.stem}.sav'
                        f.write('M sav ')
                        f.write(save_file_name)
                        f.write('\n')
                        shutil.copy(part.save_file, output_folder / save_file_name)

                    # Not used yet, but perhaps would be useful to have a feature to continue playing the next replay
                    # in a series.
                    if not is_last:
                        f.write('M next ')
                        number_part = f'{(i + 2):0{digits_len}}_of_{total}'
                        f.write(f'{replay_path.stem}_{number_part}.zplay')
                        f.write('\n')

                    if first_frame:
                        f.write('M parent_replay_frame_offset ')
                        f.write(str(first_frame))
                        f.write('\n')

            if most_recent_key_step:
                f.write(
                    ' '.join(
                        [most_recent_key_step.type, '0', most_recent_key_step.data]
                    )
                )
                f.write('\n')

            for step in part.steps:
                if (
                    step.frame == 0
                    and step.type == 'K'
                    and most_recent_key_step
                    and step.data == most_recent_key_step.data
                ):
                    continue
                f.write(' '.join([step.type, str(step.frame), step.data]))
                f.write('\n')

        if part.last_key_step:
            most_recent_key_step = part.last_key_step

        print(f'\t{output_replay.name}: {len(part.duration())}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--replay', required=True)
    parser.add_argument('--output-folder', required=True)
    parser.add_argument(
        '--skip-save-file-generation',
        action='store_true',
        help='Only use this skip if already generated in output folder!',
    )
    parser.add_argument(
        '--split-threshold',
        default=50_000,
        type=int,
        help='If current replay part length at a save point is less than this threshold, the replay will not be cut. It may be long enough on the next save.',
    )

    args = parser.parse_args()
    split_replay(
        Path(args.replay),
        Path(args.output_folder),
        skip_save_file_generation=args.skip_save_file_generation,
        split_threshold=args.split_threshold,
    )
