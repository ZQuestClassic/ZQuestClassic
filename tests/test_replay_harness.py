# python -m pytest tests/test_replay_harness.py

import os
import sys

from pathlib import Path

import pytest

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
sys.path.insert(0, str(script_dir))
sys.path.insert(0, str((root_dir / 'scripts').absolute()))

import run_target

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
    load_replays,
    make_replay_batches,
    time_format,
)

playground_dir = script_dir / 'replays/playground'
playground_qst = playground_dir / 'playground.qst'

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
# load_replays
# ---------------------------------------------------------------------------


def _write_replay(path: Path, qst: str = 'test.qst', frames: int = 10):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(f'M qst {qst}\n' f'M frames {frames}\n' 'C 0 step\n')


def test_load_replays_resolves_relative_path(tmp_path, monkeypatch):
    # A relative path must come back absolute, so the cwd it's later used from
    # (the zplayer subprocess runs with cwd=build_folder) doesn't matter.
    replays_dir = tmp_path / 'replays'
    _write_replay(replays_dir / 'a.zplay')
    monkeypatch.chdir(tmp_path)

    replays = load_replays([Path('replays/a.zplay')], Path('replays'))

    assert len(replays) == 1
    assert replays[0].path.is_absolute()
    assert replays[0].path == (replays_dir / 'a.zplay').resolve()


def test_load_replays_name_relative_to_resolved_root(tmp_path, monkeypatch):
    # Run names stay relative to root even when both inputs are relative paths.
    replays_dir = tmp_path / 'replays'
    _write_replay(replays_dir / 'subdir' / 'b.zplay')
    monkeypatch.chdir(tmp_path)

    replays = load_replays([Path('replays/subdir/b.zplay')], Path('replays'))

    assert replays[0].name == 'subdir/b.zplay'


def test_load_replays_missing_file_raises(tmp_path):
    with pytest.raises(Exception, match='file does not exist'):
        load_replays([tmp_path / 'nope.zplay'], tmp_path)


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


# ---------------------------------------------------------------------------
# make_replay_batches (seeded batching for --batch)
# ---------------------------------------------------------------------------


def _batch_replays(n: int, prefix='r') -> list[Replay]:
    return [make_replay(f'{prefix}{i}.zplay') for i in range(n)]


def _batch_names(batches):
    return [[r.name for r in g] for g in batches]


def test_make_replay_batches_is_deterministic_for_same_seed():
    replays = _batch_replays(20)
    a = make_replay_batches(replays, seed=42, batch_size=5)
    b = make_replay_batches(replays, seed=42, batch_size=5)
    assert _batch_names(a) == _batch_names(b)


def test_make_replay_batches_different_seed_changes_order():
    replays = _batch_replays(20)
    a = make_replay_batches(replays, seed=1, batch_size=5)
    b = make_replay_batches(replays, seed=2, batch_size=5)
    # Two distinct seeds producing an identical order over 20 replays is astronomically
    # unlikely; this guards against the seed being ignored.
    assert _batch_names(a) != _batch_names(b)


def test_make_replay_batches_respects_size_and_covers_all():
    replays = _batch_replays(23)
    batches = make_replay_batches(replays, seed=7, batch_size=5)
    assert all(len(g) <= 5 for g in batches)
    got = sorted(r.name for g in batches for r in g)
    assert got == sorted(r.name for r in replays)


def test_make_replay_batches_unique_basenames_per_batch():
    # Batched replays share an output dir and .result.txt files are keyed by filename, so
    # two replays with the same basename must never land in the same batch.
    replays = [
        make_replay('a/dup.zplay'),
        make_replay('b/dup.zplay'),
        make_replay('c/other.zplay'),
    ]
    batches = make_replay_batches(replays, seed=3, batch_size=8)
    for g in batches:
        basenames = [r.path.name for r in g]
        assert len(basenames) == len(set(basenames))


# ---------------------------------------------------------------------------
# -replay-batch engine integration (requires a build)
# ---------------------------------------------------------------------------

ASSERT_FAILED_EXIT_CODE = 120

# Small playground replays (66 frames each) used as known-good batch members.
GOOD_REPLAYS = [
    'auto_init_scripts',
    'auto_bug_convert_rgb',
    'auto_bug_empty_constructor',
]


@pytest.fixture
def build_folder():
    try:
        return run_target.get_build_folder()
    except Exception as e:
        pytest.skip(f'no build available: {e}')


def _write_batch_file(path: Path, entries):
    """entries: a path, or a (path, frame) tuple for a per-replay frame limit."""
    lines = []
    for entry in entries:
        if isinstance(entry, tuple):
            lines.append(f'{entry[0]} {entry[1]}')
        else:
            lines.append(str(entry))
    path.write_text('\n'.join(lines) + '\n')


def _run_batch(batch_file: Path, output_dir: Path, build_folder, extra=None):
    args = [
        '-replay-batch',
        str(batch_file),
        '-replay-output-dir',
        str(output_dir),
        '-headless',
        '-s',
        '-v0',
    ]
    if extra:
        args += extra
    # The timeout turns a regression that hangs the batch (e.g. a modal dialog) into a
    # failed test rather than a hung run.
    return run_target.run('zplayer', args, build_folder=build_folder, timeout=180)


def _batch_result(output_dir: Path, name: str):
    return parse_result_txt_file(output_dir / f'{name}.zplay.result.txt')


def _make_corrupt_copy(name: str, dst: Path) -> Path:
    """Copy a playground replay, repoint its quest to an absolute path (so it loads from any
    directory) and append a bogus trailing step. The extra step makes the replay log longer
    than what the engine records, a deterministic assert failure."""
    lines = (playground_dir / f'{name}.zplay').read_text().splitlines()
    out_lines = []
    last_frame = 0
    for line in lines:
        if line.startswith('M qst '):
            line = f'M qst {playground_qst.absolute()}'
        out_lines.append(line)
        parts = line.split(' ')
        if (
            len(parts) > 1
            and parts[0] in 'CRKDUSVQXY'
            and parts[1].lstrip('-').isdigit()
        ):
            last_frame = max(last_frame, int(parts[1]))
    out_lines.append(f'C {last_frame + 1} bogus_step')
    dst.write_text('\n'.join(out_lines) + '\n')
    return dst


def test_batch_runs_all_replays_in_one_process(tmp_path, build_folder):
    out = tmp_path / 'out'
    out.mkdir()
    batch_file = tmp_path / 'list.txt'
    _write_batch_file(batch_file, [playground_dir / f'{n}.zplay' for n in GOOD_REPLAYS])

    p = _run_batch(batch_file, out, build_folder)

    assert p.returncode == 0, p.stdout + p.stderr
    for name in GOOD_REPLAYS:
        result = _batch_result(out, name)
        assert result['stopped'], name
        assert result['success'], name


def test_batch_continues_and_reports_after_a_failure(tmp_path, build_folder):
    bad = _make_corrupt_copy('auto_bug_convert_rgb', tmp_path / 'bad.zplay')
    out = tmp_path / 'out'
    out.mkdir()
    batch_file = tmp_path / 'list.txt'
    _write_batch_file(
        batch_file,
        [
            playground_dir / 'auto_init_scripts.zplay',
            bad,
            playground_dir / 'auto_bug_empty_constructor.zplay',
        ],
    )

    p = _run_batch(batch_file, out, build_folder)

    # The batch had a failure, so it exits with the assert-failed code...
    assert p.returncode == ASSERT_FAILED_EXIT_CODE, p.stdout + p.stderr
    # ...but the replays before and after the bad one still ran and passed.
    assert _batch_result(out, 'auto_init_scripts')['success']
    assert _batch_result(out, 'auto_bug_empty_constructor')['success']
    assert _batch_result(out, 'bad')['success'] is False


def test_batch_skips_missing_replay_without_hanging(tmp_path, build_folder):
    out = tmp_path / 'out'
    out.mkdir()
    batch_file = tmp_path / 'list.txt'
    _write_batch_file(
        batch_file,
        [
            playground_dir / 'auto_init_scripts.zplay',
            tmp_path / 'does_not_exist.zplay',
            playground_dir / 'auto_bug_empty_constructor.zplay',
        ],
    )

    p = _run_batch(batch_file, out, build_folder)

    assert p.returncode == ASSERT_FAILED_EXIT_CODE, p.stdout + p.stderr
    # The missing entry produces no result file, but the others still ran.
    assert not (out / 'does_not_exist.zplay.result.txt').exists()
    assert _batch_result(out, 'auto_init_scripts')['success']
    assert _batch_result(out, 'auto_bug_empty_constructor')['success']


def test_batch_honors_per_replay_frame_limit(tmp_path, build_folder):
    out = tmp_path / 'out'
    out.mkdir()
    batch_file = tmp_path / 'list.txt'
    # auto_init_scripts is 66 frames; cap it at 20.
    _write_batch_file(batch_file, [(playground_dir / 'auto_init_scripts.zplay', 20)])

    p = _run_batch(batch_file, out, build_folder)

    assert p.returncode == 0, p.stdout + p.stderr
    result = _batch_result(out, 'auto_init_scripts')
    assert result['success']
    assert result['frame'] <= 20
