#!/bin/sh
TOKEN="telegram_token" 
ID="telegram_id"
URL="https://api.telegram.org/bot$TOKEN/sendMessage"

curl -s -X POST $URL -d chat_id=$ID -d text="$1" > tg.log
