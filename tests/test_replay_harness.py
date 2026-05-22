# python -m pytest tests/test_replay_harness.py

import os
import sys
import pytest

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
sys.path.insert(0, str(script_dir))

from lib.replay_helpers import (
    parse_result_txt_file,
    read_last_contentful_line,
    read_replay_meta,
)
from replays import (
    Replay,
    ReplayTestResults,
    RunResult,
    _is_known_failure_test,
    apply_test_filter,
    get_replay_name,
    get_shards,
    group_arg,
    time_format,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def make_replay(name: str, frames: int = 100, qst: str = 'test.qst') -> Replay:
    return Replay(
        name=name, path=Path(f'/fake/{name}'), meta={'qst': qst}, frames=frames
    )


def make_test_env(tmp_path: Path, names: list[str]):
    """Create fake .zplay files and return (tests, replays_dir)."""
    replays_dir = tmp_path / 'replays'
    replays_dir.mkdir()
    tests = []
    for name in names:
        path = replays_dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.touch()
        tests.append(path)
    return tests, replays_dir


# ---------------------------------------------------------------------------
# read_last_contentful_line
# ---------------------------------------------------------------------------


def test_read_last_contentful_line_basic(tmp_path):
    f = tmp_path / 'f.txt'
    f.write_text('line one\nline two\n')
    assert read_last_contentful_line(f) == 'line two\n'


def test_read_last_contentful_line_no_trailing_newline(tmp_path):
    f = tmp_path / 'f.txt'
    f.write_bytes(b'first\nsecond')
    assert read_last_contentful_line(f) == 'second'


def test_read_last_contentful_line_single_line(tmp_path):
    f = tmp_path / 'f.txt'
    f.write_text('only line\n')
    assert read_last_contentful_line(f) == 'only line\n'


# ---------------------------------------------------------------------------
# read_replay_meta
# ---------------------------------------------------------------------------


@pytest.fixture(autouse=True)
def clear_replay_meta_cache():
    read_replay_meta.cache_clear()
    yield
    read_replay_meta.cache_clear()


def test_read_replay_meta_with_frames_field(tmp_path):
    f = tmp_path / 'test.zplay'
    f.write_text(
        'M author TestUser\n' 'M qst classic.qst\n' 'M frames 500\n' 'C 0 something\n'
    )
    meta = read_replay_meta(f)
    assert meta['author'] == 'TestUser'
    assert meta['qst'] == 'classic.qst'
    assert meta['frames'] == 500


def test_read_replay_meta_frames_from_last_line(tmp_path):
    f = tmp_path / 'test.zplay'
    f.write_text('M qst classic.qst\n' 'C 0 step\n' 'C 999 step\n')
    meta = read_replay_meta(f)
    assert meta['frames'] == 999


def test_read_replay_meta_raises_on_empty(tmp_path):
    f = tmp_path / 'empty.zplay'
    f.write_text('C 0 step\n')
    with pytest.raises(Exception, match='invalid replay'):
        read_replay_meta(f)


def test_read_replay_meta_multi_word_value(tmp_path):
    f = tmp_path / 'test.zplay'
    f.write_text('M qst_title My Quest Title\n' 'M qst test.qst\n' 'M frames 10\n')
    meta = read_replay_meta(f)
    assert meta['qst_title'] == 'My Quest Title'


# ---------------------------------------------------------------------------
# parse_result_txt_file
# ---------------------------------------------------------------------------


def write_result(tmp_path: Path, content: str) -> Path:
    p = tmp_path / 'result.txt'
    p.write_text(content)
    return p


def test_parse_result_txt_returns_none_when_no_stopped(tmp_path):
    p = write_result(tmp_path, 'frame: 10\nfps: 60\n')
    assert parse_result_txt_file(p) is None


def test_parse_result_txt_bool_values(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nsuccess: false\n')
    result = parse_result_txt_file(p)
    assert result['stopped'] is True
    assert result['success'] is False


def test_parse_result_txt_int_values(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nframe: 42\nfps: 60\n')
    result = parse_result_txt_file(p)
    assert result['frame'] == 42
    assert result['fps'] == 60


def test_parse_result_txt_float_values(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nduration: 3.14\n')
    result = parse_result_txt_file(p)
    assert result['duration'] == pytest.approx(3.14)


def test_parse_result_txt_string_values(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nreplay: /path/to/file.zplay\n')
    result = parse_result_txt_file(p)
    assert result['replay'] == '/path/to/file.zplay'


def test_parse_result_txt_unexpected_gfx_frames(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nunexpected_gfx_frames: 10, 20, 30\n')
    result = parse_result_txt_file(p)
    assert result['unexpected_gfx_frames'] == [10, 20, 30]


def test_parse_result_txt_unexpected_gfx_segments(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nunexpected_gfx_segments: 10-20 30-40\n')
    result = parse_result_txt_file(p)
    assert result['unexpected_gfx_segments'] == [[10, 20], [30, 40]]


def test_parse_result_txt_single_frame_segment(tmp_path):
    p = write_result(tmp_path, 'stopped: true\nunexpected_gfx_segments: 55\n')
    result = parse_result_txt_file(p)
    assert result['unexpected_gfx_segments'] == [[55, 55]]


# ---------------------------------------------------------------------------
# time_format
# ---------------------------------------------------------------------------


def test_time_format_none():
    assert time_format(None) == '-'


def test_time_format_seconds():
    assert time_format(5000) == '5s'
    assert time_format(0) == '0s'


def test_time_format_minutes():
    assert time_format(90000) == '1m 30s'
    assert time_format(120000) == '2m 00s'


# ---------------------------------------------------------------------------
# get_replay_name
# ---------------------------------------------------------------------------


def test_get_replay_name_relative(tmp_path):
    replays_dir = tmp_path / 'replays'
    replay_file = replays_dir / 'subdir' / 'test.zplay'
    assert get_replay_name(replay_file, replays_dir) == 'subdir/test.zplay'


def test_get_replay_name_not_relative(tmp_path):
    replays_dir = tmp_path / 'replays'
    replay_file = Path('/elsewhere/test.zplay')
    assert get_replay_name(replay_file, replays_dir) == 'test.zplay'


# ---------------------------------------------------------------------------
# apply_test_filter
# ---------------------------------------------------------------------------


def test_apply_test_filter_exact_absolute(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['foo.zplay', 'bar.zplay'])
    result = apply_test_filter(tests, replays_dir, str(tests[0].absolute()))
    assert result == [tests[0]]


def test_apply_test_filter_exact_relative_to_replays_dir(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['foo.zplay', 'bar.zplay'])
    result = apply_test_filter(tests, replays_dir, 'foo.zplay')
    assert result == [tests[0]]


def test_apply_test_filter_substring(tmp_path):
    tests, replays_dir = make_test_env(
        tmp_path,
        ['classic/classic_1st.zplay', 'classic/classic_2nd.zplay', 'other.zplay'],
    )
    result = apply_test_filter(tests, replays_dir, 'classic')
    assert set(result) == {tests[0], tests[1]}


def test_apply_test_filter_no_match(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['foo.zplay'])
    result = apply_test_filter(tests, replays_dir, 'nonexistent')
    assert result == []


# ---------------------------------------------------------------------------
# group_arg
# ---------------------------------------------------------------------------


def test_group_arg_empty(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    assert group_arg([], tests, replays_dir) == ({}, None)


def test_group_arg_default_value(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    assert group_arg(['42'], tests, replays_dir) == ({}, '42')


def test_group_arg_two_defaults_raises(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    with pytest.raises(Exception):
        group_arg(['1', '2'], tests, replays_dir)


def test_group_arg_replay_value(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay', 'b.zplay'])
    result, default = group_arg(['a.zplay=10'], tests, replays_dir)
    assert result == {replays_dir / 'a.zplay': '10'}
    assert default is None


def test_group_arg_loose_filename_match(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['sub/deep.zplay'])
    # 'deep.zplay' doesn't exist at root, but loose match finds sub/deep.zplay
    result, _ = group_arg(['deep.zplay=5'], tests, replays_dir)
    assert result == {replays_dir / 'sub' / 'deep.zplay': '5'}


def test_group_arg_unknown_replay_raises(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    with pytest.raises(Exception, match='unknown test'):
        group_arg(['no_exist.zplay=10'], tests, replays_dir)


def test_group_arg_duplicate_raises(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    with pytest.raises(Exception):
        group_arg(['a.zplay=10', 'a.zplay=20'], tests, replays_dir)


def test_group_arg_duplicate_allow_concat(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay'])
    result, _ = group_arg(
        ['a.zplay=10', 'a.zplay=20'], tests, replays_dir, allow_concat=True
    )
    assert result == {replays_dir / 'a.zplay': '10 20'}


def test_group_arg_default_and_replay(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay', 'b.zplay'])
    result, default = group_arg(['99', 'a.zplay=10'], tests, replays_dir)
    assert default == '99'
    assert result == {replays_dir / 'a.zplay': '10'}


def test_group_arg_multiple_replays(tmp_path):
    tests, replays_dir = make_test_env(tmp_path, ['a.zplay', 'b.zplay'])
    result, default = group_arg(['3', 'a.zplay=10', 'b.zplay=20'], tests, replays_dir)
    assert default == '3'
    assert result == {
        replays_dir / 'a.zplay': '10',
        replays_dir / 'b.zplay': '20',
    }


# ---------------------------------------------------------------------------
# get_shards
# ---------------------------------------------------------------------------


def test_get_shards_even_split():
    replays = [make_replay(f'{i}.zplay', frames=100) for i in range(4)]
    durations = {r.name: 100.0 for r in replays}
    shards = get_shards(replays, durations, 2)
    assert len(shards) == 2
    assert len(shards[0]) == 2
    assert len(shards[1]) == 2


def test_get_shards_all_in_one():
    replays = [make_replay(f'{i}.zplay') for i in range(3)]
    durations = {r.name: 10.0 for r in replays}
    shards = get_shards(replays, durations, 1)
    assert len(shards) == 1
    assert len(shards[0]) == 3


def test_get_shards_covers_all_replays():
    replays = [make_replay(f'{i}.zplay', frames=i * 10 + 1) for i in range(6)]
    durations = {r.name: float(r.frames) for r in replays}
    shards = get_shards(replays, durations, 3)
    all_in_shards = [r for shard in shards for r in shard]
    assert set(r.name for r in all_in_shards) == set(r.name for r in replays)


def test_get_shards_balanced():
    # One heavy replay and many light ones — heavy should be alone in its shard.
    heavy = make_replay('heavy.zplay', frames=1000)
    lights = [make_replay(f'light{i}.zplay', frames=100) for i in range(4)]
    replays = [heavy] + lights
    durations = {r.name: float(r.frames) for r in replays}
    shards = get_shards(replays, durations, 2)
    shard_names = [set(r.name for r in s) for s in shards]
    assert {'heavy.zplay'} in shard_names


# ---------------------------------------------------------------------------
# _is_known_failure_test
# ---------------------------------------------------------------------------


def test_is_known_failure_success_always_false():
    run = RunResult(name='anything.zplay', directory='', path='', success=True)
    assert _is_known_failure_test(run, 'x64') is False


def test_is_known_failure_unrecognized_failure():
    run = RunResult(
        name='some_other.zplay',
        directory='',
        path='',
        success=False,
        failing_frame=100,
    )
    assert not _is_known_failure_test(run, 'x64')


# ---------------------------------------------------------------------------
# ReplayTestResults deserialization
# ---------------------------------------------------------------------------


def test_replay_test_results_deserializes_dicts():
    raw = {
        'runs_on': 'ubuntu-22.04',
        'arch': 'x64',
        'ci': False,
        'workflow_run_id': None,
        'git_ref': None,
        'zc_version': '3.0.0',
        'time': '2026-01-01T00:00:00+00:00',
        'runs': [
            [
                {
                    'name': 'test.zplay',
                    'directory': '0/test',
                    'path': '/path/test.zplay',
                    'success': True,
                    'stopped': True,
                    'rng_desync': False,
                    'exit_code': 0,
                    'duration': None,
                    'fps': None,
                    'frame': None,
                    'num_frames': None,
                    'failing_frame': None,
                    'unexpected_gfx_frames': None,
                    'unexpected_gfx_segments': None,
                    'unexpected_gfx_segments_limited': None,
                    'exceptions': [],
                    'snapshots': None,
                }
            ]
        ],
    }
    results = ReplayTestResults(**raw)
    assert len(results.runs) == 1
    assert isinstance(results.runs[0][0], RunResult)
    assert results.runs[0][0].name == 'test.zplay'
    assert results.runs[0][0].success is True


def test_replay_test_results_strips_old_diff_field():
    raw = {
        'runs_on': 'ubuntu-22.04',
        'arch': 'x64',
        'ci': False,
        'workflow_run_id': None,
        'git_ref': None,
        'zc_version': '3.0.0',
        'time': '2026-01-01T00:00:00+00:00',
        'runs': [
            [
                {
                    'name': 'test.zplay',
                    'directory': '0/test',
                    'path': '',
                    'success': False,
                    'stopped': True,
                    'rng_desync': None,
                    'exit_code': None,
                    'duration': None,
                    'fps': None,
                    'frame': None,
                    'num_frames': None,
                    'failing_frame': None,
                    'unexpected_gfx_frames': None,
                    'unexpected_gfx_segments': None,
                    'unexpected_gfx_segments_limited': None,
                    'exceptions': [],
                    'snapshots': None,
                    'diff': 'old field that should be removed',
                }
            ]
        ],
    }
    results = ReplayTestResults(**raw)
    assert isinstance(results.runs[0][0], RunResult)


def test_replay_test_results_missing_path_defaults_empty():
    raw = {
        'runs_on': 'ubuntu-22.04',
        'arch': 'x64',
        'ci': False,
        'workflow_run_id': None,
        'git_ref': None,
        'zc_version': '3.0.0',
        'time': '2026-01-01T00:00:00+00:00',
        'runs': [
            [
                {
                    'name': 'test.zplay',
                    'directory': '0/test',
                    'success': False,
                    'stopped': True,
                    'rng_desync': None,
                    'exit_code': None,
                    'duration': None,
                    'fps': None,
                    'frame': None,
                    'num_frames': None,
                    'failing_frame': None,
                    'unexpected_gfx_frames': None,
                    'unexpected_gfx_segments': None,
                    'unexpected_gfx_segments_limited': None,
                    'exceptions': [],
                    'snapshots': None,
                }
            ]
        ],
    }
    results = ReplayTestResults(**raw)
    assert results.runs[0][0].path == ''
