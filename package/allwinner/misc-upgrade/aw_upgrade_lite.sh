#!/bin/sh

# ============================================================================
# UPGRADE MODE
# ============================================================================
upgrade_pre_mode()
{
    echo "upgrade_pre_mode"
    do_ota clean
    get_args
    if [ -f $OTA_PACKAGE ]; then
#        do_ota prepare $BOOT0_IMG
#        do_ota prepare $UBOOT_IMG
        do_ota prepare $RECOVERY_IMG
        echo $RECOVERY_IMG
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] || {
		# don't need to prepare others now, as we will reboot after upgrade ramdisk
        do_ota prepare $BOOT_IMG
        do_ota prepare $ROOTFS_IMG
#        do_ota prepare $LOGO_IMG
        }
        do_ota upgrade
    else
        echo "ota.tar not found!"
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

    do_ota clean
    get_args
    if [ -f $OTA_PACKAGE ]; then
        do_ota prepare $BOOT_IMG
        do_ota prepare $ROOTFS_IMG
#        do_ota prepare $LOGO_IMG
        do_ota upgrade
    else
        echo "ota.tar not found!"
    fi
}
upgrade_end_mode()
{
    echo "wait for next upgrade!"
    [ -f $UPGRADE_SETTING_PATH/.image_path ] && rm -rf $UPGRADE_SETTING_PATH/.image_path
    [ -f $UPGRADE_SETTING_PATH/.image_compress ] && rm -rf $UPGRADE_SETTING_PATH/.image_compress
    [ -f $UPGRADE_SETTING_PATH/.image_url ] && rm -rf $UPGRADE_SETTING_PATH/.image_url
    [ -f $UPGRADE_SETTING_PATH/.image_domain ] && rm -rf $UPGRADE_SETTING_PATH/.image_domain
}
get_args()
{
    [ -f $UPGRADE_SETTING_PATH/.image_path ]     && export OTA_PACKAGE=`cat $UPGRADE_SETTING_PATH/.image_path`
    [ -f $UPGRADE_SETTING_PATH/.image_compress ] && export IS_COMPRESS_IMAGE=`cat $UPGRADE_SETTING_PATH/.image_compress`
    [ -f $UPGRADE_SETTING_PATH/.image_domain ]   && export DOMAIN=`cat $UPGRADE_SETTING_PATH/.image_domain`
    [ -f $UPGRADE_SETTING_PATH/.image_url ]      && export URL=`cat $UPGRADE_SETTING_PATH/.image_url`

    echo setting args LOCAL_IMG_PATH:    $OTA_PACKAGE
    echo setting args IS_COMPRESS_IMAGE: $IS_COMPRESS_IMAGE
    echo setting args DOMAIN:            $DOMAIN
    echo setting args URL:               $URL
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
        rm -rf $UPGRADE_IMG_DIR
        rm -rf $RAMDISK_DIR* $TARGET_DIR*
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
    md5_1=$(tar -xf "$OTA_PACKAGE" "$1" -O | md5sum | awk '{print $1}')
    md5_2=$(tar -xf "$OTA_PACKAGE" "$1.md5" -O)
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
#        tar -xvf "$OTA_PACKAGE" "$BOOT0_IMG" -C /mnt/UDISK/misc-upgrade/
#        ota-burnboot0 /mnt/UDISK/misc-upgrade/$BOOT0_IMG
#        tar -xvf "$OTA_PACKAGE" "$UBOOT_IMG" -C /mnt/UDISK/misc-upgrade/
#        ota-burnuboot /mnt/UDISK/misc-upgrade/$UBOOT_IMG
#        sync

        tar -xf "$OTA_PACKAGE" "$RECOVERY_IMG" -O | dd of="/dev/by-name/recovery"
	    sync
        set_system_flag "boot-recovery"
        # reboot to recovery system
        [ x"$REBOOT_TO_RECOVERY" = x"1" ] && reboot -f
    }
    flag=`get_system_flag`
    [ x$flag = x"boot-recovery" ] && {

        # Progress display on screen need enable recovery program
#        /bin/recovery &
#        echo "###recovery start###"
#        sleep 1
#        /usr/bin/recovery-mode &
#        echo "###recovery-mode start###"

        tar -xf "$OTA_PACKAGE" "$BOOT_IMG" -O | dd of="/dev/by-name/boot"
        sync
        tar -xf "$OTA_PACKAGE" "$ROOTFS_IMG" -O | dd of="/dev/by-name/rootfs"
        sync
#        tar -xf "$OTA_PACKAGE" "$LOGO_IMG" -O | dd of="/dev/by-name/bootlogo"
#        sync
        set_system_flag "upgrade_rootfs"

        # for busybox-init, /etc/etc_need_update cause etc update.
        touch /etc/etc_need_update
        sync
        set_system_flag "upgrade_end"
        sleep 1
        reboot -f
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

UPGRADE_LOG_FILE=/mnt/UDISK/upgrade.log
# ============================================================================
# MAIN
# ============================================================================
while getopts "fl:u:" opt; do
    case $opt in
    f)
        mode="--force"
        ;;
    l)
        [ ! -e $OPTARG ] && {
            echo "-l $OPTARG, the settting PATH is unavailable"
            exit $ERR_ILLEGAL_ARGS
        }
        mkdir -p $UPGRADE_SETTING_PATH
        echo $OPTARG > $UPGRADE_SETTING_PATH/.image_path
        WLAN_IMG_PATH="$OPTARG/ota.tar"
        ;;
    u)
        [ -z $OPTARG ] && {
            echo "-u $OPTARG, the setting URL is unavailable \n"
            echo "Please input -u xxx(URL) \n"
            exit $ERR_ILLEGAL_ARGS
        }
        echo $WLAN_IMG_PATH > $UPGRADE_SETTING_PATH/.image_path
        echo $OPTARG > $UPGRADE_SETTING_PATH/.image_url
        echo "### wget $OPTARG -O $WLAN_IMG_PATH ###"
        wget $OPTARG -O $WLAN_IMG_PATH
        return 0
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
elif [ x$system_flag = x"upgrade_rootfs" ]; then
    upgrade_target_mode
elif [ x$system_flag = x"upgrade_end" ]; then
    upgrade_end_mode
else
    upgrade_end_mode
fi
