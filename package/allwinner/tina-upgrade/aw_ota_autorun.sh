#!/bin/sh

#wait for /dev/by-name ready
count=0
while [ ! -e /dev/by-name/recovery ] && [ $count -lt 10 ]
do
	let count+=1
	sleep 1
done

#wait for /mnt/UDISK/ota
count=0
while [ ! -e /mnt/UDISK/ota ] && [ $count -lt 10 ]
do
       let count+=1
       sleep 1
done

/sbin/aw_auto_connect_wifi.sh & 1>/dev/console 2>/dev/console
/sbin/aw_ota_apply_img.sh 1>/dev/console 2>/dev/console
