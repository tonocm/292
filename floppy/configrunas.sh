#!/bin/sh

echo 'a' > pwdNobody
echo 'b' > pwdFTP

cd /etc/

echo 'root:nobody:a' > runas
echo 'root:ftp:b' >> runas

chown root:root ./runas
chmod 644 ./runas