#!/bin/bash

set -e

SERVER=api.zquestclassic.com
HASH=$(git rev-parse HEAD)

ssh root@$SERVER "cd /home/zc/ZQuestClassic && git fetch && git checkout $HASH"

scp configs/nginx/* root@$SERVER:/etc/nginx/conf.d/
scp configs/systemd/* root@$SERVER:/etc/systemd/system/
scp configs/crontab/* root@$SERVER:/etc/cron.d/

ssh root@$SERVER "nginx -t"
ssh root@$SERVER "systemctl daemon-reload"
ssh root@$SERVER "systemctl restart cron nginx zc"
