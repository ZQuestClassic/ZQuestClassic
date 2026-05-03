#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"

set -e

SERVER=api.zquestclassic.com

ssh root@$SERVER "cd /home/zc/ZQuestClassic && git fetch && git checkout origin/main"

scp nginx/* root@$SERVER:/etc/nginx/conf.d/
scp systemd/* root@$SERVER:/etc/systemd/system/
scp crontab/* root@$SERVER:/etc/cron.d/

ssh root@$SERVER "nginx -t"
ssh root@$SERVER "systemctl daemon-reload"
ssh root@$SERVER "systemctl restart cron nginx zc.api_server"
