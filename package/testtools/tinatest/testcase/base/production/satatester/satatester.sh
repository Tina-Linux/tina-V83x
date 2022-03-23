#!/bin/sh

count=0
mount_path="/tmp/sata"
can_format=`mjson_fetch /base/production/satatester/format`

rw_test()
{
	if rwcheck -p 1 -t 1 -s 10M -d ${mount_path}; then
		echo "rwcheck pass"
        umount ${mount_path}
        exit 0
    else
		echo "rwcheck failed"
        exit 1
    fi
}

device_format_mount_rw()
{
	mkfs.ext4 -F $1 \
		 && mount $1 ${mount_path} \
		 && rw_test
}

while true; do
	blk=`ls /sys/devices/soc/sata/ata1/host0/*/*/block 2>/dev/null | head -n 1`
	if [ -b "/dev/$blk" ]; then
		mkdir -p ${mount_path}
		for device in $(ls /dev/$blk[1-9]* 2&>/dev/null | sort -r)
		do
			mount ${device} ${mount_path} &>/dev/null
			if [ $? == 0 ]; then
				rw_test
			else
				device_part=${device}
			fi
		done

		mount /dev/$blk ${mount_path} &>/dev/null && rw_test

		[ "${can_format}" = "true" ] || {
			echo "sata mounted fail and formatting is not allowed!!!"
			exit 0
		}

		[ "x${device_part}" != "x" ] && device_format_mount_rw ${device_part} \
		|| device_format_mount_rw /dev/$blk
	fi
done
