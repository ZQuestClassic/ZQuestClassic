#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

cd /home/zc/ZQuestClassic
git fetch
git checkout main
git reset --hard origin/main

if [ ! -d ".venv" ]; then
	python -m venv .venv
fi
source .venv/bin/activate

# Only install if requirements.txt changed.
REQ_HASH_FILE=".venv/requirements.txt.md5"
if [ ! -f "$REQ_HASH_FILE" ] || [ "$(md5sum requirements.txt)" != "$(cat $REQ_HASH_FILE)" ]; then
	pip install -r requirements.txt
	md5sum requirements.txt > "$REQ_HASH_FILE"
fi

python infra/scripts/update_docs.py
