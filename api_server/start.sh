#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."

set -e

if [ ! -d ../venv ] ; then
	python -m venv ../venv
fi

source ../venv/bin/activate
pip install -r ../requirements.txt

export FLASK_APP=server
export FLASK_S3_BUCKET=zc-replays
export FLASK_S3_URL=https://nyc3.digitaloceanspaces.com
export FLASK_S3_REGION=ny3
export FLASK_DATA_DIR="${FLASK_DATA_DIR:=../.tmp/api_server}"

CONFIG_PATH="$PWD/secrets.sh"
if [ -f "$CONFIG_PATH" ] ; then
	source "$CONFIG_PATH"
	if [[ -z "${FLASK_S3_ACCESS_KEY}" ]]; then
		echo "missing FLASK_S3_ACCESS_KEY"
		exit 1
	fi
	if [[ -z "${FLASK_S3_SECRET_ACCESS_KEY}" ]]; then
		echo "missing FLASK_S3_SECRET_ACCESS_KEY"
		exit 1
	fi
else
	echo "No config file at $CONFIG_PATH"
fi

mkdir -p "$FLASK_DATA_DIR/replays"

if [[ -z "${FLASK_PRODUCTION}" ]]; then
	# Local dev.
	mkdir -p "$ROOT_DIR/.tmp/database"
	ln -sfh "$ROOT_DIR/.tmp/database" "$FLASK_DATA_DIR/database"

	flask run -p 5000
else
	# Prod.
	mkdir -p "$FLASK_DATA_DIR/database"

	echo "syncing from s3://$FLASK_S3_BUCKET/ ..."
	s3cmd sync --no-preserve --acl-public "s3://$FLASK_S3_BUCKET/" "$FLASK_DATA_DIR/replays/"
	echo "done syncing from s3"

	uwsgi --ini server.ini
fi
