#!/bin/sh

# ============================================================================
# UPGRADE MODE
# ============================================================================
upgrade_pre_mode()
{
    echo "upgrade_pre_mode"
    do_ota clean

    part="recovery boot rootfs"
    for i in $part; do
        old_file="/dev/by-name/$i"
        new_file=$UPGRADE_IMG_DIR/"$i".img
        patch_file=$UPGRADE_IMG_DIR/"$i".img.patch
        echo "apply patch:" "$old_file" "$new_file" "$patch_file"
        bspatch "$old_file" "$new_file" "$patch_file"
    done
    echo "bspatch finished!!!"

    if [ -f $UPGRADE_IMG_DIR/$RECOVERY_IMG ]; then
        do_ota prepare $RECOVERY_IMG
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] || {
		#don't need to prepare others now, as we will reboot after upgrade ramdisk
        do_ota prepare $BOOT_IMG
        do_ota prepare $ROOTFS_IMG
        }
        do_ota upgrade
    else
        echo "OTA.img not found!"
    fi
}
upgrade_target_mode()
{
    echo "upgrade_target_mode"

    #try to mount rootfs_data partition
    fgrep -sq 'rootfs_data' /proc/mounts || {
        [ -h /dev/by-name/rootfs_data ] && try_mount "rootfs_data" "/overlay"
    }

    #try to mount UDISK partition
    fgrep -sq 'UDISK' /proc/mounts || {
        [ -h /dev/by-name/UDISK ] && try_mount "UDISK" "/mnt/UDISK"
    }

    if [ -f $UPGRADE_IMG_DIR/$BOOT_IMG ]; then
        do_ota prepare $BOOT_IMG
        do_ota prepare $ROOTFS_IMG
        do_ota upgrade
    else
        echo "ota.tar not found!"
    fi
    reboot -f
}
upgrade_end_mode()
{
    echo "wait for next upgrade!"
}
try_mount()
{
    # $1 partition name $2 mount dir
    format_list="ext4 jffs2 vfat"
    for i in $format_list; do
        echo "mounting $i /dev/by-name/$1 -> $2"
        mount -t $i /dev/by-name/$1 $2
        [ $? -eq 0 ] && break
    done
}
# ============================================================================
# DO OTA
# ============================================================================
do_ota()
{
    if [ $# -lt 1 ]; then
        show_usage
        exit $ERR_ILLEGAL_ARGS
    elif [ x$1 = x"prepare" ]; then
        upgrade_log "start to prepare -->>> $2 <<<--"
        do_prepare_image $2
    elif [ x$1 = x"upgrade" ]; then
        upgrade_log "start to upgrade"
        do_upgrade_image
    elif [ x$1 = x"clean" ]; then
        upgrade_log "clean the prepared image"
    else
        show_usage
        exit $ERR_ILLEGAL_ARGS
    fi
}
show_usage()
{
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
do_prepare_image()
{
    upgrade_log "prepare image start..."
    md5_1=$(md5sum $UPGRADE_IMG_DIR/$1 | awk '{print $1}')
    md5_2=$(cat $UPGRADE_IMG_DIR/$1.md5)
    [ -n $md5_2 -a $md5_1 = $md5_2 ] && {
        upgrade_log "$1 md5 check success!"
        return 0
    }
    upgrade_log "$1 md5 check failed!"
    exit 1
}
do_upgrade_image()
{
    flag=`get_system_flag`
    [ x$flag = x"upgrade_pre" ] && {
        dd if=$UPGRADE_IMG_DIR/$RECOVERY_IMG of=/dev/by-name/recovery
	    sync
        set_system_flag "boot-recovery"
        #reboot to recovery system
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] && reboot -f
    }
    flag=`get_system_flag`
    [ x$flag = x"boot-recovery" ] && {
        dd if=$UPGRADE_IMG_DIR/$BOOT_IMG of=/dev/by-name/boot
        sync
        dd if=$UPGRADE_IMG_DIR/$ROOTFS_IMG of=/dev/by-name/rootfs
        sync
#        dd if=$UPGRADE_IMG_DIR/bootlogo.img of=/dev/by-name/bootlogo
#        sync

        #for busybox-init, /etc/etc_need_update cause etc update.
        touch /etc/etc_need_update
        sync
        set_system_flag "upgrade_end"
    }
}
upgrade_log()
{
    busybox echo `busybox date`  $1
    #busybox echo `busybox date`  $1 >> $UPGRADE_LOG_FILE
}
get_system_flag()
{
    read_misc command
}
set_system_flag()
{
    upgrade_log "set system flag : $1"
    write_misc -c $1
    sync
}
# ============================================================================
# GLOBAL VARIABLES
# ============================================================================
. /sbin/aw_upgrade_utils.sh

UPGRADE_IMG_DIR=/mnt/UDISK/misc-upgrade
UPGRADE_LOG_FILE=/mnt/UDISK/upgrade.log
# ============================================================================
# MAIN
# ============================================================================
while getopts "fpl:nu:d:" opt; do
    case $opt in
    f)
        mode="--force"
        ;;
    \?)
        echo "Invalid option: -f"
        exit
        ;;
    esac
done
if [ -n $mode ] && [ x$mode = x"--force" ]; then
    set_system_flag "upgrade_pre"
    upgrade_pre_mode
    exit 0
fi

system_flag=`read_misc command`
if [ x$system_flag = x"upgrade_pre" ]; then
    upgrade_pre_mode
elif [ x$system_flag = x"boot-recovery" ]; then
    upgrade_target_mode
elif [ x$system_flag = x"upgrade_end" ]; then
    upgrade_end_mode
else
    upgrade_end_mode
fi
