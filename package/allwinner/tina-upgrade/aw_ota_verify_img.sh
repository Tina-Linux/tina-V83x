#!/bin/sh
#set -x

. aw_ota_utils.sh

check_img_md5(){
	md5_1=$(busybox md5sum $1 | busybox awk '{print $1}')
	md5_2=$(cat "$2")
	[ x"$md5_1" = x"$md5_2" ] && {
		echo "$1 md5 check success!"
		return 0
	}
	echo "check_img_md5 failed, target: $1 !"
	return 1
}

check_img_signature(){
	#$1 img file #2 signature file #3 pub key
	#return: 0 - success ; 1 - fail
	pub_key=/OTA_Key_pub.pem
	if [ $# -ge 3 ];then
		echo key is "$3"
		pub_key=$3
	fi
	verify_result=$(openssl dgst -sha256 -verify "$pub_key" -signature "$2" "$1")

	# [[ $verify_result = "Verified OK" ]] && {
	[ x"${verify_result:9:2}" = x"OK" ] && {
		echo "$1 signature check success!"
		return 0
	}
	echo "check_img_signature failed, target: $1 !"
	return 1
}

check_img(){
	local part=$1
	local img;
	img=$(part_2_img "$part")
	if [ x"$VERIFY_METHOR" = x"md5" ];then
		check_img_md5 "$LOCAL_OTA_DIR"/"$img.verified"  "$LOCAL_OTA_DIR"/"$img.md5"
		[ $? -eq 1 ] && return 1
	elif [ x"$VERIFY_METHOR" = x"signature" ];then
		check_img_signature  "$LOCAL_OTA_DIR"/"$img.verified"  "$LOCAL_OTA_DIR"/"$img.signature"
		[ $? -eq 1 ] && return 1
	fi
	return 0
}
aw_verify_patch()
{
	local part=$1
	local img
	local old_file
	local new_file
	local patch_file
	local md5_file
	local to_verify_img
	local verified_img
	local signature_file

	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1
	new_file=$LOCAL_OTA_DIR/"$img".merge
	patch_file=/tmp/"$img.patch"
	md5_file=/tmp/"$img.md5"
	signature_file=/tmp/"$img.signature"
	verified_img=$LOCAL_OTA_DIR/"$img".verified

	[ x"$part" = x"uboot" -o x"$part" = x"boot0" ] && {
		echo "boot0 and uboot not support patch now"
		rm -f "$md5_file" "$signature_file"
		return 1
	}


	[ x"$part" = x"boot" ] && {
		old_file=/dev/by-name/boot
	}
	[ x"$part" = x"rootfs" ] && {
		old_file=/dev/by-name/rootfs
	}
	[ x"$part" = x"recovery" ] && {
		old_file=/dev/by-name/recovery
	}
	apply_patch "$old_file" "$new_file" "$patch_file"

	to_verify_img=$new_file
	check_img_md5 "$to_verify_img" "$md5_file"
	[ $? = 0 ] && {
		mv "$to_verify_img" "$verified_img"
		[ x"$USE_SIGNATURE" = x"1" ] && mv "$signature_file" "$LOCAL_OTA_DIR"/
		rm -f "$md5_file"
		return 0;
	}
	rm -f "$to_verify_img" "$md5_file" "$signature_file"
	return 1
}

aw_verify_img()
{
	local part=$1
	local img
	local img_file
	local md5_file
	local to_verify_img
	local verified_img
	local signature_file

	img=$(part_2_img "$part")
	[ x"$img" = x"" ] && echo "set img fail" && return 1
	to_verify_img=/tmp/"$img"
	md5_file=/tmp/"$img.md5"
	signature_file=/tmp/"$img.signature"
	verified_img=$LOCAL_OTA_DIR/"$img".verified

	check_img_md5 "$to_verify_img" "$md5_file"
	[ $? = 0 ] && {
		mv "$to_verify_img" "$verified_img"
		[ x"$USE_SIGNATURE" = x"1" ] && mv "$signature_file" "$LOCAL_OTA_DIR"/
		return 0;
	}
	rm -f "$to_verify_img" "$md5_file" "$signature_file"
	return 1
}
