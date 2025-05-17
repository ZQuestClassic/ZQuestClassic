import json
import os
import sys
import uuid

from http import HTTPStatus
from pathlib import Path
from typing import Dict, List, Tuple

import boto3

from flask import Flask, flash, redirect, request, url_for
from werkzeug.exceptions import HTTPException

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

sys.path.append(str((root_dir / 'scripts').absolute()))
import database

app = Flask(__name__)
app.config.from_prefixed_env()

data_dir = Path(app.config['DATA_DIR'])
replays_dir = data_dir / 'replays'
replays_dir.mkdir(exist_ok=True)


def is_valid_uuid(val):
    try:
        uuid.UUID(str(val))
        return True
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


def parse_meta(replay_text: str) -> Dict[str, str]:
    meta = {}

    for line in replay_text.splitlines():
        if not line.startswith('M'):
            break

        _, key, value = line.strip().split(' ', 2)
        meta[key] = value

    return meta


def parse_replay(replay_text: str) -> Tuple[Dict[str, str], List[str]]:
    """Returns tuple: meta, steps"""
    meta = {}
    steps = []
    done_with_meta = False

    for line in replay_text.splitlines():
        if not line.startswith('M'):
            done_with_meta = True
        if done_with_meta and line:
            steps.append(line)
        else:
            _, key, value = line.strip().split(' ', 2)
            meta[key] = value

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
    # No larger than 30 MB.
    if len(request.data) > 30e6:
        return {'error': 'too large, max is 30 MB'}, HTTPStatus.REQUEST_ENTITY_TOO_LARGE

    data = request.data.decode('utf-8')
    if not data.startswith('M'):
        return {'error': 'invalid replay'}, HTTPStatus.BAD_REQUEST

    meta, steps = parse_replay(data)
    qst_hash = meta.get('qst_hash')
    uuid_from_replay = meta.get('uuid')

    if not qst_hash or not uuid:
        return {'error': 'replay is too old'}, HTTPStatus.BAD_REQUEST

    if uuid_from_replay != uuid:
        return {'error': 'replay uuid does not match uuid from url'}, HTTPStatus.BAD_REQUEST

    if not qst_hash.isalnum():
        return {'error': 'invalid qst_hash'}, HTTPStatus.BAD_REQUEST

    if not is_valid_uuid(uuid):
        return {'error': 'invalid uuid'}, HTTPStatus.BAD_REQUEST

    if int(meta.get('length')) != len(steps):
        return {'error': 'invalid length'}, HTTPStatus.BAD_REQUEST

    if qst_hash not in qst_keys_by_hash:
        return {'error': 'unknown qst'}, HTTPStatus.BAD_REQUEST

    key = f'{qst_hash}/{uuid}.zplay'
    path = replays_dir / key
    status = HTTPStatus.CREATED
    if path.exists():
        status = HTTPStatus.OK
        existing_meta, existing_steps = parse_replay(path.read_text('utf-8'))
        if int(existing_meta['length']) > int(meta['length']):
            return {'error': 'replay is older than current'}, HTTPStatus.CONFLICT
        # Replays are append-only.
        for i, existing_step in enumerate(existing_steps):
            if existing_step != steps[i]:
                return {
                    'error': 'replay is different than current'
                }, HTTPStatus.CONFLICT

    path.parent.mkdir(exist_ok=True)
    path.write_text(data)
    if s3:
        s3.upload_file(path, app.config['S3_BUCKET'], key)

    return {'key': key}, status


@app.errorhandler(HTTPException)
def handle_exception(e):
    """Return JSON instead of HTML for HTTP errors."""
    response = e.get_response()
    response.data = json.dumps(
        {
            'code': e.code,
            'name': e.name,
            'description': e.description,
        }
    )
    response.content_type = 'application/json'
    return response


if __name__ == '__main__':
    app.run(host='0.0.0.0')
