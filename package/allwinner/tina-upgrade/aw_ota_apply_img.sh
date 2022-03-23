#!/bin/sh
. /sbin/aw_ota_utils.sh
. /sbin/aw_ota_verify_img.sh

need_reboot=0

write_partition_by_dd(){
	busybox dd if="$1" of="$2"
	local ret=$?
	sync
	return $ret
}

write_partition(){
	local img=$1
	local partition_name=$2
	[ -e "$img" ] && {
		write_partition_by_dd "$img" "/dev/by-name/$partition_name"
		return $?
	}
	return 1
}

get_system()
{
	echo "$boot_partition"
}

reboot_2_recovery_system()
{
	local system=$(get_system)
	[ x"$system" != x"recovery" ] && {
		set_env boot_partition recovery
		do_reboot
	}
}

reboot_2_main_system()
{
	local system=$(get_system)
	[ x"$system" != x"boot" ] && {
		set_env boot_partition boot
		do_reboot
	}
}

update_img_prepare()
{
	local part=$1
	local verified_img=$2
	check_img "$part"
	[ $? -eq 1 ] && return 1
	return 0
}

update_uboot()
{
	local output=$(ota-burnuboot "$1")
	local result=$(echo "$output" | grep "Success")
	[ "$result" != "" ] &&  return 0
	return 1
}

update_boot0()
{
	local output=$(ota-burnboot0 "$1")
	local result=$(echo "$output" | grep "Success")
	[ "$result" != "" ] &&  return 0
	return 1
}

update_img()
{
	local part=$1
	local verified_img=$2
	echo "update_img $part"
	[ x"$part" = x"uboot" ] && {
		update_uboot "$verified_img"
		return $?
	}
	[ x"$part" = x"boot0" ] && {
		update_boot0 "$verified_img"
		return $?
	}
	write_partition "$verified_img" "$part"
	return $?
}



update_img_finish()
{
	local part=$1
	local verified_img=$2

	img=$(part_2_img "$part")
	rm -f "$LOCAL_OTA_DIR"/"$img".*

	return 0
}

try_update_img()
{
	local part=$1
	local img
	local verified_img
	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1
	verified_img=$LOCAL_OTA_DIR/$img.verified
	[ -e "$verified_img" ] || return 0

	update_img_prepare "$part" "$verified_img"
	[ $? -eq 1 ] && return 1

	update_img "$part" "$verified_img"
	[ $? -eq 1 ] && return 1

	update_img_finish "$part" "$verified_img"
	[ $? -eq 1 ] && return 1

	echo "try_update_img $part Success"
	return 0
}

[ x"$ota" = x"0" ] && return 0

echo "aw_ota_apply_img"
try_update_img recovery
try_update_img boot
try_update_img rootfs
try_update_img uboot
try_update_img boot0
echo "aw_ota_apply_img done"

[ x"$ota" = x"1" ] && set_ota_flag 0 && need_reboot=1

[ x"$boot_partition" != x"boot" ] && set_boot_partition boot && need_reboot=1

[ x"$need_reboot" = x"1" ] && do_reboot


