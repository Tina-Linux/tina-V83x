#!/bin/sh
#$1: target upgrade package

. /sbin/aw_upgrade_utils.sh

UPGRADE_IMG_DIR=/tmp/upgrade
UPGRADE_LOG_FILE=/mnt/UDISK/upgrade.log

show_usage(){
    cat <<EOF
Usage: $0 prepare <image file> :
            prepare and md5 check image for upgrade. eg: $1 prepare /tmp/upgrade.tar.gz

       $0 upgrade :
            upgrade the prepared image.
       $0 version :
            set system version string
       $0 clean :
            clean the prepared image
EOF
}

upgrade_log(){
    #$1 msg
    busybox echo `busybox date`  $1
    #busybox echo `busybox date`  $1 >> $UPGRADE_LOG_FILE
}
set_system_version(){
    # $1 version string
    upgrade_log "set system version: $1"
    write_misc -v $1
    sync
}
set_system_flag(){
    # $1 flag string
    upgrade_log "set system flag : $1"
    write_misc -c $1
    sync
    #read_misc command
}
get_system_flag(){
    read_misc command
}

prepare_env_done=0
prepare_env(){
    #prepare env
    flag=`get_system_flag`
    if [ x"$prepare_env_done" != x"1" -a x$flag != x"boot-recovery" ]; then
        #current shell process is busybox, so busybox is already in dram
        #backup other needed tools
        UPGRADE_ROOT=/tmp/upgrade_root
        [ -f $UPGRADE_ROOT/bin/busybox ] && {
            upgrade_log "env already prepared!!"
            return 0
        }
        rm -rf $UPGRADE_ROOT
        mkdir -p $UPGRADE_ROOT/bin
        mkdir -p $UPGRADE_ROOT/sbin
        mkdir -p $UPGRADE_ROOT/lib

        #busybox
        cp /bin/busybox $UPGRADE_ROOT/bin/
        cp /lib/libcrypt.so* $UPGRADE_ROOT/lib/
        cp /lib/libm.so* $UPGRADE_ROOT/lib/
        cp /lib/libgcc_s.so* $UPGRADE_ROOT/lib/
        cp /lib/libc.so* $UPGRADE_ROOT/lib/
        shell_list="awk cat cp cut date dd df echo expr grep kill ln ls md5sum mkdir mount mv ping pwd readlink reboot rm sh sync tar touch umount wget"
        for i in $shell_list; do
            ln -s $UPGRADE_ROOT/bin/busybox $UPGRADE_ROOT/bin/$i
        done

        #misc tools
        cp /sbin/read_misc $UPGRADE_ROOT/sbin/
        cp /sbin/write_misc $UPGRADE_ROOT/sbin/
        cp /sbin/aw_reboot.sh $UPGRADE_ROOT/sbin/

	#tinyplay
	[ x"$SUPPORT_TINYPLAY" = x"1" ] && {
            [ -e /bin/tinyplay ] && cp /bin/tinyplay $UPGRADE_ROOT/bin/
            [ -e /usr/bin/tinyplay ] && cp /usr/bin/tinyplay $UPGRADE_ROOT/bin/
        }

        export PATH=$UPGRADE_ROOT/bin/:$UPGRADE_ROOT/sbin/:$PATH
        export LD_LIBRARY_PATH=$UPGRADE_ROOT/lib

	prepare_env_done=1
    fi
}

check_img_md5(){
    #$1 img file #2 md5 file
    #return: 0 - success ; 1 - fail
    md5_1=`busybox md5sum $1 | busybox awk '{print $1}'`
    md5_2=`cat $2`
    [ $md5_1 = $md5_2 ] && {
        upgrade_log "$1 md5 check success!"
        return 0
    }
    upgrade_log "check_img_md5 failed, target: $1 !"
    return 1
}
try_mount(){
    # $1 partition name $2 mount dir
    format_list="ext4 jffs2 vfat"
    for i in $format_list; do
        echo "mounting $i /dev/by-name/$1 -> $2"
        mount -t $i /dev/by-name/$1 $2
        [ $? -eq 0 ] && break
    done
}
write_mmc_partition(){
    busybox dd if=$1 of=/dev/by-name/$2 conv=fsync
    sync
}
write_mtd_partition(){
    #if varify failed,retry
    let retry=10
    while [ $retry -gt 0 ]
        do
        let retry=$retry-1
        verify_file=$UPGRADE_IMG_DIR/mtd_$2_verify
        mtd write $1 $2
        sync
        mtd verify $1 $2 &> $verify_file
        cat $verify_file | grep "Success"
        [ $? -eq 0 ] && {
            echo "$2: verify success!!!!"
            break
        }
        echo "$2: verify retry failed,retry $retry"
    done

}
write_emmc_partition(){
    # $1 img
    # $2 partition name
    upgrade_log "write_emmc_partition $1 > /dev/by-name/$2"
    [ -e /dev/by-name/$2 ] && {
        write_mmc_partition $1 $2
    }
}
write_nor_partition(){
    # $1 img
    # $2 partition name
    upgrade_log "write_nor_partition $1 > $2"
    cat /proc/mtd | grep "\"$2\""
    if [ $? -eq 0 ]; then
#        write_mtd_partition $1 $2
        write_mmc_partition $1 $2
    else
        upgrade_log "$2 mtd partition is not exsit"
    fi
}
write_nand_partition(){
    # $1 img
    # $2 partition name
    upgrade_log "write_nand_partition $1 > /dev/by-name/$2"
    [ -e /dev/by-name/$2 ] && {
        write_mmc_partition $1 $2
    }
}
do_write_partition(){
    # $1 img
    # $2 partition name
    [ ! -e $1 ] && {
        upgrade_log "$1 not exist"
        return 1
    }
    if [ -e /dev/nand? ]; then
        write_nand_partition $1 $2
    elif [ -e /dev/mtdblock0 ]; then
        write_nor_partition $1 $2
    elif [ -e /dev/mmcblk0 ]; then
        # try emmc after nand/nor, to support nor/nand with sdcard
        write_emmc_partition $1 $2
    else
        upgrade_log "match partition failed!"
        return 1
    fi
    return 0
}

do_stream_update(){
    file_download=$URL/$1
    partition=/dev/by-name/$2
    echo "updating $1"
    file_length=$(wget -S "$file_download" 2>&1 | grep "Content-Length" | awk '{print $2}')
    file_sectors=$(expr "$file_length" / 512)
    echo "length:$file_length length_sectors:$file_sectors"

    wget "$file_download" -q -O - | dd of="$partition"
    md51=$(wget "$file_download.md5" -q -O -)
    md52=$(dd if="$partition" bs=512 count="$file_sectors" | md5sum | cut -d ' ' -f 1)

    echo md51:"$md51"
    echo md52:"$md52"
    [ x"$md51" != x"$md52" ] && {
        exit $ERR_MD5_CHECK_FAILED
    }
    echo "md5 check pass"
}

do_upgrade_image_normal(){
    [ -f $UPGRADE_IMG_DIR/$RAMDISK_IMG ] &&
    {
	set_system_flag "upgrade_pre"
	#if fail #1, reboot ->
	#	boot from boot partition ->
	#	upgrade process ->
	#	get all image ->
	#	do again
	#reboot -f #test
        if [ -e /dev/by-name/extend ];then
		do_write_partition $UPGRADE_IMG_DIR/$RAMDISK_IMG "extend"
        else
		do_write_partition $UPGRADE_IMG_DIR/$RAMDISK_IMG "recovery"
        fi
        local boot_partition=$(fw_printenv -n boot_partition)
        [ -n "${boot_partition}" ] &&
            {
                fw_setenv boot_partition recovery
            }
        set_system_flag "boot-recovery"	#TODO:change to upgrade_target,need change uboot
	#fail #1 end
        rm -rf $UPGRADE_IMG_DIR/$RAMDISK_IMG
        #reboot to recovery system
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] &&
        {
            reboot -f
        }
    }

    flag=`get_system_flag`
    [ -f $UPGRADE_IMG_DIR/$BOOT_IMG ] && [ -f $UPGRADE_IMG_DIR/$ROOTFS_IMG ] && [ x$flag = x"boot-recovery" ] &&	#TODO:change to upgrade_target,need change uboot
    {
	#if fail #2, reboot ->
	#	boot from extend partition(initramfs) ->
	#	upgrade process ->
	#	get target image ->
	#	do again
	#reboot -f #test
        do_write_partition $UPGRADE_IMG_DIR/$BOOT_IMG "boot"
        do_write_partition $UPGRADE_IMG_DIR/$ROOTFS_IMG "rootfs"

	[ -h /dev/by-name/rootfs_data ] && {
		#clear extroot-uuid flag
		mkdir -p /tmp/overlay
		try_mount "rootfs_data" "/tmp/overlay"
		[ -f /tmp/overlay/etc/.extroot-uuid ] &&
		{
			upgrade_log "clear overlay extroot-uuid"
			rm /tmp/overlay/etc/.extroot-uuid
		}
		[ -f /tmp/overlay/upper/etc/.extroot-uuid ] &&
		{
			upgrade_log "clear overlay extroot-uuid"
			rm /tmp/overlay/upper/etc/.extroot-uuid
		}
	}

        local boot_partition=$(fw_printenv -n boot_partition)
        [ -n "${boot_partition}" ] &&
            {
                fw_setenv boot_partition boot
            }
	set_system_flag "upgrade_boot0"
	#fail #2 end
        rm -rf $UPGRADE_IMG_DIR/$ROOTFS_IMG $UPGRADE_IMG_DIR/$BOOT_IMG
    }

    flag=`get_system_flag`
    [ x$flag = x"upgrade_boot0" ] &&
    {
	#if fail #3, reboot ->
	#	boot from extend partition(initramfs) ->
	#	upgrade process ->
	#	get boot0 image ->
	#	do again
	#reboot -f #test
	if [ -f $UPGRADE_IMG_DIR/$BOOT0_IMG ];then
		ota-burnboot0 $UPGRADE_IMG_DIR/$BOOT0_IMG
	else
		echo "boot0.img not exist"
	fi
        local boot_partition=$(fw_printenv -n boot_partition)
        [ -n "${boot_partition}" ] &&
            {
                fw_setenv boot_partition boot
            }
    set_system_flag "upgrade_uboot"
	#fail #3 end
	rm -rf $UPGRADE_IMG_DIR/$BOOT0_IMG
    }

    flag=`get_system_flag`
    [ x$flag = x"upgrade_uboot" ] &&
    {
        #if fail #4, reboot ->
	#	boot from extend partition(initramfs) ->
	#       upgrade process ->
	#       get uboot image ->
	#       do again
	#reboot -f #test
	if [ -f $UPGRADE_IMG_DIR/$UBOOT_IMG ];then
		ota-burnuboot $UPGRADE_IMG_DIR/$UBOOT_IMG
	else
		echo "uboot.img not exist"
	fi
        local boot_partition=$(fw_printenv -n boot_partition)
        [ -n "${boot_partition}" ] &&
            {
                fw_setenv boot_partition boot
            }
    set_system_flag "upgrade_post"
        #fail #4 end
        rm -rf $UPGRADE_IMG_DIR/$UBOOT_IMG
    }

    flag=`get_system_flag`
    [ x$flag = x"upgrade_post" ] &&
    {
	#if fail #5, reboot ->
	#	boot from boot partition(initramfs) ->
	#	upgrade process ->
	#	get usr image ->
	#	do again
	#reboot -f #test
	local ota_end=0
	[ -e /dev/by-name/extend ] && {
		if [ -f $UPGRADE_IMG_DIR/$USR_IMG ];then
			do_write_partition $UPGRADE_IMG_DIR/$USR_IMG "extend"
			ota_end=1
		else
			# extend partition exist, usr.img is necessary
			# don't set end flag, that allow try again later
			echo "usr.img not exist"
			ota_end=0
		fi
	}

	# no extend partition, then we can set end flag
	[ ! -e /dev/by-name/extend ] && ota_end=1

	[ x"$ota_end" = x"1" ] && {
		local boot_partition=$(fw_printenv -n boot_partition)
		[ -n "${boot_partition}" ] && fw_setenv boot_partition boot
		set_system_flag "upgrade_end"
	}

	#fail #5 end
	rm -rf $UPGRADE_IMG_DIR/$USR_IMG

    }

}

do_upgrade_image_stream(){
    flag=`get_system_flag`
    [ x"$flag" = x"upgrade_pre" ] && {
        if [ -e /dev/by-name/extend ];then
            do_stream_update "$RAMDISK_IMG" "extend"
        else
            do_stream_update "$RAMDISK_IMG" "recovery"
        fi
        set_system_flag "boot-recovery"
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] && {
            #reboot to recovery system
            reboot -f
        }
    }

    flag=`get_system_flag`
    [ x"$flag" = x"boot-recovery" ] && {
        do_stream_update "$BOOT_IMG" "boot"
        do_stream_update "$ROOTFS_IMG" "rootfs"
        set_system_flag "upgrade_post"
    }

    flag=`get_system_flag`
    [ x"$flag" = x"upgrade_post" ] && {
        if [ -e /dev/by-name/extend ];then
            do_stream_update "$USR_IMG" "extend"
        fi
        set_system_flag "upgrade_end"

    }
}

do_upgrade_image(){
    echo do_upgrade_image ......

    [ x"$REBOOT_TO_RECOVERY" != x"1" ] && {
        #to upgrade rootfs in main system, need to prepare env
        prepare_env
    }

    if [ x"$STREAM_UPDATE" = x"1" ];then
        do_upgrade_image_stream $*
    else
        do_upgrade_image_normal $*
    fi
}

do_prepare_image(){
    # $1 image file path
    # $2 image file name
    # $3 --none-compress image file is none compress
    #    no set image file is compress file
    upgrade_log "unpack image start..."
    local img_path="$(readlink -f $1)"
    if [ -n $3 ] && [ x$3 = x"--none-compress" ]; then
        mkdir -p $UPGRADE_IMG_DIR
        check_img_md5 $1/$2 $1/$2.md5
	[ $? -eq 1 ] && {
            upgrade_log "check_img_md5 Fail"
            exit $ERR_MD5_CHECK_FAILED
        }
	# do not need to copy or move it, just add a soft link
	ln -s $1/$2  $UPGRADE_IMG_DIR
    else
        tar -zxvf $1/$2 -C /tmp/
        [ $? -eq 1 ] && {
            upgrade_log "no enongh space to unpack"
            exit $ERR_NOT_ENOUGH_SPACE
        }
        cd /tmp
        [ -d $RAMDISK_DIR ] && list="$RAMDISK_DIR/$RAMDISK_IMG"
        [ -d $TARGET_DIR ] && list="$TARGET_DIR/$BOOT_IMG $TARGET_DIR/$ROOTFS_IMG"
        [ -d $BOOT0_DIR ] && list="$BOOT0_DIR/$BOOT0_IMG"
        [ -d $UBOOT_DIR ] && list="$UBOOT_DIR/$UBOOT_IMG"
        [ -d $USR_DIR ] && list="$USR_DIR/$USR_IMG"
        echo .......... $list
        for i in $list;do
            check_img_md5 $i $i.md5
            [ $? -eq 1 ] && {
                upgrade_log "check_img_md5 Fail"
                rm -rf $RAMDISK_DIR $TARGET_DIR $BOOT0_DIR $UBOOT_DIR $USR_DIR
                exit $ERR_MD5_CHECK_FAILED
            }
        done
        mkdir -p $UPGRADE_IMG_DIR
        mv $list $UPGRADE_IMG_DIR
        rm -rf $RAMDISK_DIR $TARGET_DIR $BOOT0_DIR $UBOOT_DIR $USR_DIR
    fi
    upgrade_log "unpack image finish..."
}
###################################################################################
#check args
do_upgrade(){
    if [ $# -lt 1 ]; then
        show_usage
        exit $ERR_ILLEGAL_ARGS
    elif [ x$1 = x"prepare" ] && [ $# -ge 3 ] && [ -f $2/$3 ]; then
        upgrade_log "start to prepare -->>> $2/$3 <<<--"
        do_prepare_image $2 $3 $4
    elif [ x$1 = x"upgrade" ]; then
        upgrade_log "start to upgrade"
        do_upgrade_image
    elif [ x$1 = x"clean" ]; then
        upgrade_log "clean the prepared image"
        rm -rf $UPGRADE_IMG_DIR
        rm -rf $RAMDISK_DIR* $TARGET_DIR* $BOOT0_DIR* $UBOOT_DIR* $USR_DIR*
    elif [ x$1 = x"version" ] && [ -ne $2 ]; then
        set_system_version $2
    else
        show_usage
        exit $ERR_ILLEGAL_ARGS
    fi
}
upgrade_log " "
