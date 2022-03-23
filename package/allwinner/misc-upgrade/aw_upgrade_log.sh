#!/bin/sh

count=0
while [ ! -e /dev/by-name/extend ] && [ ! -e /dev/by-name/recovery ] && [ $count -lt 10 ]
do
	let count+=1
	sleep 1
done

count=0
while [ $count -lt 10 ]
do
	if $(fgrep -sq '/dev/by-name/UDISK /mnt/UDISK' /proc/mounts) ;then
		echo "### mount /mnt/UDISK/ succeed ###"
		break
	else
		let count+=1
		sleep 1
		[ $count == 10 ] && echo "### wait mount /mnt/UDISK/ 10s timeout ###"
	fi
done

if [ -e /mnt/UDISK/misc-upgrade/ota.tar ]; then
    /sbin/aw_upgrade_lite.sh 1>/dev/console 2>/dev/console
fi

if [ -e /mnt/UDISK/misc-upgrade/recovery.img.patch ]; then
    /sbin/aw_upgrade_plus.sh 1>/dev/console 2>/dev/console
fi

if [ -e /dev/by-name/extend ];then
    /sbin/aw_upgrade_process.sh 1>/dev/console 2>/dev/console
else
    /sbin/aw_upgrade_normal.sh 1>/dev/console 2>/dev/console
fi
