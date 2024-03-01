import functools
import logging
import os
import platform
import re

from pathlib import Path
from time import sleep


def read_last_contentful_line(file: Path):
    f = file.open('rb')
    try:  # catch OSError in case of a one line file
        f.seek(-2, os.SEEK_END)
        found_content = False
        while True:
            c = f.read(1)
            if not c.isspace():
                found_content = True
            if found_content and c == b'\n':
                if found_content:
                    break
            f.seek(-2, os.SEEK_CUR)
    except OSError:
        f.seek(0)
    return f.readline().decode()


@functools.cache
def read_replay_meta(path: Path):
    meta = {}
    with path.open('r', encoding='utf-8') as f:
        while True:
            line = f.readline()
            if not line.startswith('M'):
                break
            _, key, value = line.strip().split(' ', 2)
            meta[key] = value

    if not meta:
        raise Exception(f'invalid replay {path}')

    if 'frames' not in meta:
        # TODO: delete this when all replay tests have `M frames`
        last_step = read_last_contentful_line(path)
        if not last_step:
            raise Exception(f'no content found in {path}')
        if not re.match(r'^. \d+ ', last_step):
            raise Exception(
                f'unexpected content found in {path}:\n  {last_step}\nAre you sure this is a zplay file?'
            )
        frames = int(last_step.split(' ')[1])
        meta['frames'] = frames

    return meta


def parse_result_txt_file(path: Path):
    if platform.system() == 'Windows':
        # Windows has a tough time reading this file, sometimes resulting in a permission
        # denied error. I suspect MSVC's `std::filesystem::rename` is not atomic like it
        # claims to be. Or maybe the problem lies with Python's mtime.
        for _ in range(0, 10):
            try:
                lines = path.read_text().splitlines()
                if _ != 0:
                    logging.warning('finally was able to read it')
                break
            except:
                logging.exception(f'could not read {path}')
                sleep(0.1)
    else:
        lines = path.read_text().splitlines()

    result = {}
    for line in lines:
        key, value = line.split(': ', 1)
        if key == 'unexpected_gfx_frames':
            value = [int(x) for x in value.split(', ')]
        elif (
            key == 'unexpected_gfx_segments' or key == 'unexpected_gfx_segments_limited'
        ):
            segments = []
            for pair in value.split(' '):
                if '-' in pair:
                    start, end = pair.split('-')
                else:
                    start = int(pair)
                    end = start
                segments.append([int(start), int(end)])
            value = segments
        elif value == 'true':
            value = True
        elif value == 'false':
            value = False
        elif value.isdigit():
            value = int(value)
        else:
            try:
                value = float(value)
            except:
                pass

        result[key] = value

    # Check if file is only partially written.
    if 'stopped' not in result:
        return None

    return result
