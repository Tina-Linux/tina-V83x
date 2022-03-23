#!/bin/sh
#set -x

. aw_ota_utils.sh
. aw_ota_verify_img.sh

apply_patch()
{
	local old_file=$1
	local new_file=$2
	local patch_file=$3
	echo "apply patch:" "$old_file" "$new_file" "$patch_file"
	bspatch "$old_file" "$new_file" "$patch_file"
}

aw_get_self_patch()
{
	local part=$1
	local img
	local remote_path=$OTA_SERVER_URL/ota/imgs

	echo "get patch for $part"
	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img.patch" /tmp/"$img.md5" /tmp/"$img.signature"

	wget "$remote_path"/"$img.patch" -P  /tmp/
	[ $? != 0 ] && return 1
	wget "$remote_path"/"$img.md5"   -P  /tmp/
	[ $? != 0 ] && return 1
	[ x"$USE_SIGNATURE" = x"1" ] && {
		wget "$remote_path"/"$img.signature"   -P  /tmp/
		[ $? != 0 ] && return 1
	}

	return 0
}

aw_clean()
{
	local part=$1
	local img

	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img"*

}

aw_get_patch_from_url()
{
	local part=$1
	local img
	local remote_path=$OTA_SERVER_URL/ota/patchs
	echo "get patch from $remote_path"
	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img.patch" /tmp/"$img.md5" /tmp/"$img.signature"

	wget "$remote_path"/"$img.patch" -P  /tmp/
	[ $? != 0 ] && return 1
	wget "$remote_path"/"$img.md5"   -P  /tmp/
	[ $? != 0 ] && return 1
	[ x"$USE_SIGNATURE" = x"1" ] && {
		wget "$remote_path"/"$img.signature"   -P  /tmp/
		[ $? != 0 ] && return 1
	}
	echo "get patch from $remote_path success"
	return 0
}

aw_get_patch_from_local_path()
{
	local part=$1
	local img
	local local_path=$OTA_SOURCE_LOCAL_PATH/ota/patchs
	echo "get patch from $local_path"
	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img.patch" /tmp/"$img.md5" /tmp/"$img.signature"

	cp "$local_path"/"$img.patch" /tmp/
	[ $? != 0 ] && return 1
	cp "$local_path"/"$img.md5"  /tmp/
	[ $? != 0 ] && return 1
	[ x"$USE_SIGNATURE" = x"1" ] && {
		cp "$local_path"/"$img.signature"   -P  /tmp/
		[ $? != 0 ] && return 1
	}
	echo "get patch from $local_path success"
	return 0
}



aw_get_img_from_url()
{
	local part=$1
	local img
	local remote_path=$OTA_SERVER_URL/ota/imgs
	echo "get img from $remote_path"

	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img" /tmp/"$img.md5" /tmp/"$img.signature"

	wget "$remote_path"/"$img"	-P  /tmp/
	[ $? != 0 ] && return 1
	wget "$remote_path"/"$img.md5"	-P  /tmp/
	[ $? != 0 ] && return 1
	[ x"$USE_SIGNATURE" = x"1" ] && {
		wget "$remote_path"/"$img.signature"   -P  /tmp/
		[ $? != 0 ] && return 1
	}

	echo "get img from $remote_path success"
	return 0
}

aw_get_img_from_local_path()
{
	local part=$1
	local img
	local local_path=$OTA_SOURCE_LOCAL_PATH/ota/imgs

	echo "get img from $local_path"
	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1

	rm -f /tmp/"$img" /tmp/"$img.md5" /tmp/"$img.signature"

	cp "$local_path"/"$img"	/tmp/
	[ $? != 0 ] && return 1
	cp "$local_path"/"$img.md5"	/tmp/
	[ $? != 0 ] && return 1
	[ x"$USE_SIGNATURE" = x"1" ] && {
		cp "$local_path"/"$img.signature"	/tmp/
		[ $? != 0 ] && return 1
	}

	echo "get img from $local_path success"
	return 0
}

aw_get_patch()
{
	echo "get patch for $@"
	[ x"$ota_source" = x"1" ] && {
		echo "get patch from web"
		aw_get_patch_from_url "$@"
		return $?
	}

	[ x"$ota_source" = x"2" ] && {
		echo "get patch from local path"
		aw_get_patch_from_local_path "$@"
		return $?
	}
}

aw_get_img()
{
	echo "aw get img for $@"
	[ x"$ota_source" = x"1" ] && {
		echo "get img from web"
		aw_get_img_from_url "$@"
		return $?
	}

	[ x"$ota_source" = x"2" ] && {
		echo "get img from local path"
		echo "aw_get_img_from_local_path"
		aw_get_img_from_local_path "$@"
		return $?
	}
}


aw_pre_img()
{
	local part=$1
	echo "prepare img for $part"
	aw_get_patch "$part"
	[ $? = 0 ] && {
		echo "verifying patch for $part"
		aw_verify_patch "$part"
		[ $? = 0 ] && {
			echo "prepare img from patch success"
			return 0;
		}
		echo "patch not match existing img"
	}

	aw_get_img "$part"
	[ $? = 0 ] && {
		echo "verifying img for $part"
		aw_verify_img "$part"
		[ $? = 0 ] && {
			echo "prepared img from img success"
			return 0;
		}
	}
	echo "prepare img for $part fail"

	return 1;
}

check_update()
{
	#TODO check if need update
	local check_result
	check_result=1
	echo ${check_result}
}

