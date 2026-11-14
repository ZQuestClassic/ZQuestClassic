import datetime
import hashlib
import ipaddress
import json
import os
import shutil
import sys
import uuid
import zlib

from http import HTTPStatus
from pathlib import Path

import boto3

try:
    import fcntl
except ImportError:  # Windows (local dev only)
    fcntl = None

from flask import Flask, flash, redirect, request, url_for
from werkzeug.exceptions import HTTPException

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

sys.path.append(str((root_dir / 'scripts').absolute()))
import database

# An uploaded replay may not exceed this size as sent (before decompression).
MAX_UPLOAD_BYTES = 30_000_000

app = Flask(__name__)
# Rejects larger requests before reading them, rather than buffering them all in memory.
app.config['MAX_CONTENT_LENGTH'] = MAX_UPLOAD_BYTES
app.config.from_prefixed_env()

data_dir = Path(app.config['DATA_DIR'])
replays_dir = data_dir / 'replays'
replays_dir.mkdir(exist_ok=True)


# An uploaded replay may not exceed this size once decompressed.
MAX_REPLAY_BYTES = 300_000_000

# Uploads are refused (507) once the disk holding the replays has less free space than this.
# Can be set with the FLASK_MIN_FREE_DISK_BYTES env var.
MIN_FREE_DISK_BYTES = int(app.config.get('MIN_FREE_DISK_BYTES', 2_000_000_000))

# Per client IP (per /64 for IPv6, see quota_key), per UTC day (429 when exceeded). ZC uploads each changed replay once a week
# at most, one at a time, so a real player stays far below these.
MAX_UPLOADS_PER_IP_PER_DAY = 500
MAX_UPLOAD_BYTES_PER_IP_PER_DAY = 3_000_000_000

# Shared by all the uwsgi processes, so the counts live on disk.
quota_dir = data_dir / 'upload_quota'


def decompress_gzip_bounded(data: bytes, max_size: int) -> bytes:
    """Decompress gzip data, aborting if the output would exceed max_size.

    Unlike gzip.decompress(), this never buffers more than ~max_size bytes, so a
    small "gzip bomb" payload can't exhaust memory before the size check runs.
    """
    decompressor = zlib.decompressobj(16 + zlib.MAX_WBITS)
    out = decompressor.decompress(data, max_size + 1)
    if decompressor.unconsumed_tail or len(out) > max_size:
        raise ValueError('decompressed payload too large')
    # Concatenating copies `out`, which can be hundreds of MB, so only do it if needed.
    tail = decompressor.flush()
    if tail:
        if len(out) + len(tail) > max_size:
            raise ValueError('decompressed payload too large')
        out += tail
    return out


def quota_key(ip: str) -> str:
    """The address uploads are counted under. An IPv6 address counts as its whole /64, which
    is what a single customer usually gets, so switching addresses within it doesn't help.
    """
    try:
        addr = ipaddress.ip_address(ip)
    except ValueError:
        return ip
    if addr.version == 6 and addr.ipv4_mapped:
        return str(addr.ipv4_mapped)
    if addr.version == 6:
        return str(ipaddress.ip_network(f'{addr}/64', strict=False))
    return str(addr)


class IpQuota:
    """Counts uploads and bytes per client IP for the current UTC day, in a file locked
    against the other server processes for the duration of the `with` block."""

    def __init__(self, ip: str):
        today = datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%d')
        day_dir = quota_dir / today
        if not day_dir.exists():
            day_dir.mkdir(parents=True, exist_ok=True)
            for old in quota_dir.iterdir():
                if old.name != today:
                    shutil.rmtree(old, ignore_errors=True)
        # Not the IP itself: no need to keep those around.
        name = hashlib.sha256(quota_key(ip).encode()).hexdigest()[:32]
        self.path = day_dir / name
        self.uploads = 0
        self.bytes = 0

    def __enter__(self):
        self.file = open(self.path, 'a+')
        # Not waiting for the lock: one IP with several uploads in flight shouldn't be able
        # to tie up every server process. ZC uploads one at a time.
        self.busy = False
        if fcntl:
            try:
                fcntl.flock(self.file, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                self.busy = True
                return self
        self.file.seek(0)
        parts = self.file.read().split()
        if len(parts) == 2 and all(p.isdigit() for p in parts):
            self.uploads, self.bytes = int(parts[0]), int(parts[1])
        return self

    def exceeded(self) -> bool:
        return (
            self.uploads >= MAX_UPLOADS_PER_IP_PER_DAY
            or self.bytes >= MAX_UPLOAD_BYTES_PER_IP_PER_DAY
        )

    def add(self, num_bytes: int):
        self.uploads += 1
        self.bytes += num_bytes
        self.file.seek(0)
        self.file.truncate()
        self.file.write(f'{self.uploads} {self.bytes}')
        self.file.flush()

    def __exit__(self, *args):
        # Closing releases the lock.
        self.file.close()


def is_valid_uuid(val):
    # Only the canonical form (what ZC writes), else the same replay could be uploaded under
    # many keys (uuid.UUID also accepts braces, "urn:uuid:", no hyphens, upper case...).
    try:
        return str(uuid.UUID(str(val))) == val
    except ValueError:
        return False


def connect_s3():
    session = boto3.session.Session()
    return session.client(
        's3',
        region_name=app.config['S3_REGION'],
        endpoint_url=app.config['S3_URL'],
        aws_access_key_id=app.config['S3_ACCESS_KEY'],
        aws_secret_access_key=app.config['S3_SECRET_ACCESS_KEY'],
    )


def parse_replay(replay_text: str) -> tuple[dict[str, str], list[str]]:
    """Returns tuple: meta, steps. Raises ValueError if malformed."""
    meta = {}
    steps = []
    done_with_meta = False

    for line in replay_text.splitlines():
        if not line:
            continue
        if not line.startswith('M'):
            done_with_meta = True
        if done_with_meta:
            steps.append(line)
        else:
            # Not stripped: a meta line may have an empty value ("M qst_title ").
            parts = line.split(' ', 2)
            if len(parts) != 3:
                raise ValueError(f'invalid meta line: {line[:100]}')
            meta[parts[1]] = parts[2]

    return meta, steps


s3 = connect_s3() if app.config.get('PRODUCTION') else None
db = database.Database(data_dir / 'database')

qst_keys_by_hash = db.get_qst_keys_by_hash()
quest_list = []
for qst_hash in qst_keys_by_hash.keys():
    quest_list.append({'hash': qst_hash})
print(f'found {len(quest_list)} quests')

replay_guid_to_path = {}
for path in replays_dir.rglob('*.zplay'):
    replay_guid_to_path[path.stem] = path
print(f'found {len(replay_guid_to_path)} replays')


@app.route('/api/v1/quests', methods=['GET'])
def quests():
    return quest_list


@app.route('/api/v1/replays/<uuid>', methods=['PUT'])
def replays(uuid):
    # Old clients auto-uploaded replays without user consent (fixed in
    # 8940060885); they predate this header, so requiring it locks them all out.
    if not request.headers.get('ZC-Version'):
        return {
            'error': 'missing ZC-Version header (please update ZC)'
        }, HTTPStatus.BAD_REQUEST

    # Log the attempted upload size up front (and flushed), so that if a large
    # payload starves the box of memory we can still correlate it in the logs.
    content_encoding = request.headers.get('Content-Encoding', '').lower()
    print(
        f'replay upload attempt: uuid={uuid} ip={request.remote_addr} '
        f'content_length={request.content_length} '
        f'encoding={content_encoding or "none"}',
        flush=True,
    )

    if shutil.disk_usage(replays_dir).free < MIN_FREE_DISK_BYTES:
        print('replay upload refused: disk is full', flush=True)
        return {
            'error': 'server is out of space for replays, try again later'
        }, HTTPStatus.INSUFFICIENT_STORAGE

    with IpQuota(request.remote_addr or '') as quota:
        if quota.busy:
            return {
                'error': 'another upload from this address is in progress, try again later'
            }, HTTPStatus.TOO_MANY_REQUESTS
        if quota.exceeded():
            print(
                f'replay upload refused: quota exceeded ip={request.remote_addr}',
                flush=True,
            )
            return {
                'error': 'too many uploads today, try again later'
            }, HTTPStatus.TOO_MANY_REQUESTS

        body, status, *stored = handle_replay_upload(uuid, content_encoding)
        if stored:
            quota.add(stored[0])
        return body, status


def handle_replay_upload(uuid, content_encoding):
    """Returns (body, status, bytes stored)."""
    # Larger payloads are rejected by MAX_CONTENT_LENGTH.
    raw_data = request.data

    if content_encoding == 'gzip':
        try:
            raw_data = decompress_gzip_bounded(raw_data, MAX_REPLAY_BYTES)
        except Exception as e:
            return {
                'error': f'Failed to decompress gzip payload: {str(e)}'
            }, HTTPStatus.BAD_REQUEST
    elif content_encoding:
        return {
            'error': f'Unsupported content encoding payload: {content_encoding}'
        }, HTTPStatus.BAD_REQUEST

    print(
        f'replay upload decoded: uuid={uuid} bytes={len(raw_data)}',
        flush=True,
    )

    try:
        data = raw_data.decode('utf-8')
    except UnicodeDecodeError:
        return {'error': 'Payload is not valid utf-8 text'}, HTTPStatus.BAD_REQUEST
    # Only one copy of a large replay at a time.
    del raw_data

    if not data.startswith('M'):
        return {'error': 'invalid replay'}, HTTPStatus.BAD_REQUEST

    # No larger than 300 MB.
    if len(data) > MAX_REPLAY_BYTES:
        return {
            'error': 'too large, max is 300 MB'
        }, HTTPStatus.REQUEST_ENTITY_TOO_LARGE

    try:
        meta, steps = parse_replay(data)
    except ValueError as e:
        return {'error': f'invalid replay: {e}'}, HTTPStatus.BAD_REQUEST

    qst_hash = meta.get('qst_hash')
    uuid_from_replay = meta.get('uuid')

    if not qst_hash or not uuid_from_replay or 'length' not in meta:
        return {'error': 'replay is too old'}, HTTPStatus.BAD_REQUEST

    if uuid_from_replay != uuid:
        return {
            'error': 'replay uuid does not match uuid from url'
        }, HTTPStatus.BAD_REQUEST

    if not qst_hash.isalnum():
        return {'error': 'invalid qst_hash'}, HTTPStatus.BAD_REQUEST

    if not is_valid_uuid(uuid):
        return {'error': 'invalid uuid'}, HTTPStatus.BAD_REQUEST

    # Split-replay segments (from scripts/split_replay.py) start from a .sav
    # snapshot that never gets uploaded alongside the replay, so they can
    # never actually be replayed.
    if 'sav' in meta:
        return {
            'error': 'replay requires a save file to run (split replay segment)'
        }, HTTPStatus.BAD_REQUEST

    try:
        length = int(meta['length'])
    except ValueError:
        return {'error': 'invalid length'}, HTTPStatus.BAD_REQUEST
    if length != len(steps):
        return {'error': 'invalid length'}, HTTPStatus.BAD_REQUEST

    if qst_hash not in qst_keys_by_hash:
        return {'error': 'unknown qst'}, HTTPStatus.BAD_REQUEST

    key = f'{qst_hash}/{uuid}.zplay'
    path = replays_dir / key
    status = HTTPStatus.CREATED
    if path.exists():
        status = HTTPStatus.OK
        existing_meta, existing_steps = parse_replay(path.read_text('utf-8'))
        if int(existing_meta['length']) > length:
            return {'error': 'replay is older than current'}, HTTPStatus.CONFLICT
        # Replays are append-only.
        if steps[: len(existing_steps)] != existing_steps:
            return {'error': 'replay is different than current'}, HTTPStatus.CONFLICT

    path.parent.mkdir(exist_ok=True)
    path.write_text(data)
    if s3:
        s3.upload_file(path, app.config['S3_BUCKET'], key)

    return {'key': key}, status, len(data)


@app.errorhandler(HTTPException)
def handle_exception(e):
    """Return JSON instead of HTML for HTTP errors."""
    response = e.get_response()
    response.data = json.dumps(
        {
            'code': e.code,
            'name': e.name,
            'description': e.description,
            # What the client logs.
            'error': e.description,
        }
    )
    response.content_type = 'application/json'
    return response


if __name__ == '__main__':
    app.run(host='0.0.0.0')
