#!/bin/sh

cd /var/
mkdir tmp
cd /var/tmp/

touch runaslog
chown root:root ./runaslog
chmod 644 ./runaslog