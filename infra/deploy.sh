#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"

set -e

SERVER=api.zquestclassic.com

# The checkout belongs to zc (which runs the api server and the docs cron), so keep root
# from creating files in it. The docs cron used to run as root, so first hand back anything
# root made there, and give zc the docs web root and the cron's log.
ssh root@$SERVER "chown -R zc: /home/zc/ZQuestClassic && mkdir -p /var/www/docs.zquestclassic.com && chown -R zc: /var/www/docs.zquestclassic.com && touch /var/log/zc-docs-update.log && chown zc: /var/log/zc-docs-update.log"
ssh root@$SERVER "cd /home/zc/ZQuestClassic && sudo -u zc -H git fetch && sudo -u zc -H git checkout origin/main"

scp nginx/* root@$SERVER:/etc/nginx/conf.d/
scp systemd/* root@$SERVER:/etc/systemd/system/
scp crontab/* root@$SERVER:/etc/cron.d/

ssh root@$SERVER "nginx -t"
ssh root@$SERVER "systemctl daemon-reload"
ssh root@$SERVER "systemctl restart cron nginx zc.api_server"
