#!/bin/sh
cat ../syslog | grep -a -E -w 'extracted|�����|quit the game' | tail -$1 > system.txt
