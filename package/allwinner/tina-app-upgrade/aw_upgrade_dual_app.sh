#!/bin/sh

ota_package=/mnt/UDISK/app_ota.tar

update_img()
{
	img_to_ota=$1
	partition_to_ota=$2
	echo "[OTA]begin update $img_to_ota to /dev/by-name/$partition_to_ota"
	img_md5=$(tar -xf "$ota_package" "$img_to_ota.md5" -O)
	file_size=$(tar -tvf "$ota_package" "$img_to_ota" | awk '{print $3/512}')
	echo "[OTA]file_size:$file_size"

	#verify source img
	echo "[OTA]verifying $img_to_ota..."
	source_md5=$(tar -xf "$ota_package" "$img_to_ota" -O | md5sum | awk '{print $1}')
	echo "[OTA]source_md5:$source_md5"
	echo "[OTA]img_md5:$img_md5"
	[ x"$source_md5" != x"$img_md5" ] && {
		echo "[OTA]verify $img_to_ota md5 fail"
		exit -1
	}

	#clean the partition
	echo "[OTA]cleaning $partition_to_ota..."
	dd if=/dev/zero of="/dev/by-name/$partition_to_ota"  > /dev/null
	sync
	echo "[OTA]clean $partition_to_ota done"

	#update partitions
	echo "[OTA]updating $partition_to_ota..."
	tar -xf "$ota_package" "$img_to_ota" -O | dd of="/dev/by-name/$partition_to_ota"
	sync
	echo "[OTA]update $partition_to_ota done"

	#readout to verify
	echo "[OTA]verifying $partition_to_ota"
	verify_md5=$(dd if="/dev/by-name/$partition_to_ota" bs=512 count="$file_size" | md5sum | awk '{print $1}')
	echo "[OTA]verify_md5:$verify_md5"
	echo "[OTA]img_md5:$img_md5"
	[ x"$verify_md5" != x"$img_md5" ] && {
		echo "[OTA]verify md5 fail"
		exit -1
	}
	echo "[OTA]verifying $partition_to_ota done"

	#announce success
	echo "[OTA]update $img_to_ota to /dev/by-name/$partition_to_ota success"
}

[ ! -f $ota_package ] && echo "[OTA]$ota_package not exist" && exit -1

#check appAB
mkdir -p /var/lock
appAB=$(fw_printenv -n appAB)
echo "[OTA]appAB=$appAB"
if mount | grep -q "app_sub" ; then
	echo "[OTA]now is app_sub"
	app_partition_to_ota="app"
	next_app="A"
#TODO: maybe other partition's name contain app
elif mount | grep -q "app" ; then
	echo "[OTA]now is app"
	app_partition_to_ota="app_sub"
	next_app="B"
else
	echo "[OTA]not app or app_sub"
	#TODO: should we defaultly do update to app?
	return -1
fi

update_img app.fex $app_partition_to_ota

fw_setenv appAB $next_app

echo "[OTA]update app done!"

#reboot

