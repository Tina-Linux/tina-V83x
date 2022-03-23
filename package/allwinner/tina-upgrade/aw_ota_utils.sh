LOCAL_OTA_DIR=/mnt/UDISK/ota/imgs
OTA_SERVER_URL=http://192.168.43.208
USE_SIGNATURE=1
VERIFY_METHOR=md5
[ $USE_SIGNATURE = 1 ] && VERIFY_METHOR=signature

UPGRADE_SETTING_PATH=/mnt/UDISK/.misc-upgrade


#err code
ERR_ILLEGAL_ARGS=2
ERR_NETWORK_FAILED=3
ERR_NOT_ENOUGH_SPACE=4
ERR_VENDOR_HOOK_NOT_SUPPORT=5
ERR_MD5_CHECK_FAILED=10

set_system_flag()
{
	# $1 flag string
	write_misc -c $1
	sync
	#read_misc command
}

get_system_flag()
{
	read_misc command
}

set_system_version()
{
	# $1 version string
	write_misc -v $1
	sync
}

get_system_version()
{
	read_misc version
}

is_nand()
{
	[ x"$boot_type" = x"0" ] && echo 1 && return
	echo 0
}

is_mmc()
{
	[ x"$boot_type" = x"1" -o  x"$boot_type" = x"2" -o x"$boot_type" = x"4" ] && echo 1 && return
	echo 0
}

part_2_img()
{
	local part=$1
	local img

	[ x"$part" = x"" ] && return 1
	[ x"$part" = x"boot" ] && img=boot.img
	[ x"$part" = x"rootfs" ] && img=rootfs.img
	[ x"$part" = x"recovery" ] && img=recovery.img
	[ x"$part" = x"uboot" ] && img=boot_package.img
	[ x"$part" = x"boot0" ] && {
		[ x"$(is_nand)" = x"1" ] && img=boot0_nand.img
		[ x"$(is_mmc)" = x"1" ] && img=boot0_sdcard.img
	}
	echo $img
	[ x"$img" = x"" ] && echo "no img for part $1"
	return 0
}

in_initramfs()
{
	local current_device
	current_device=$(stat -c %04D /)
	[ x"$current_device" = x"0001" ] && return 1
	return 0;
}

in_recovery()
{
	[ x"$boot_partition" = x"recovery" ] && return 1
	return 0;
}

reboot_now()
{
	echo "reboot now"
	reboot -f
}


reboot_after_5_seconds_if_no_input()
{
	local user_input
	if read -t 5 -p "Reboot after 5 seconds...Press Enter to abort" user_input  < /dev/console
	then
		echo "reboot aborted"
		return 0
	fi

	reboot_now
}


do_reboot()
{
	#for debug
	reboot_after_5_seconds_if_no_input

	#for release
	#reboot_now
}

set_env()
{
	fw_setenv "$1" "$2"
}

set_ota_flag()
{
	set_env ota "$1"
}

set_boot_partition()
{
	set_env boot_partition "$1"
}

get_args(){
	ota_source=0
	[ -f "$UPGRADE_SETTING_PATH"/.image_url ] && export OTA_SERVER_URL=$(cat "$UPGRADE_SETTING_PATH"/.image_url) && ota_source=1 && echo ota_source:$ota_source
	[ -f "$UPGRADE_SETTING_PATH"/.ota_source_local_path ] && export OTA_SOURCE_LOCAL_PATH=$(cat "$UPGRADE_SETTING_PATH"/.ota_source_local_path) && ota_source=2 && echo ota_source:$ota_source

	echo setting args URL:               "$OTA_SERVER_URL"
	echo setting args LOCAL PATH:        "$OTA_SOURCE_LOCAL_PATH"
}


#LOCAL_VERSION=`get_system_version`
