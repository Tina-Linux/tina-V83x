#!/bin/sh

clear_and_exit() {
    umount ${mnt_point}
    rm -rf ${mnt_point}
    exit $1
}

check_emmc() {
    [ ! -b "/dev/mmcblk1" ] \
        && echo "Not Found /sys/block/mmcblk1" \
        && exit 1

    # if brun SDK to mmc before, mmcblk1p1 must be UDISK
    [ -b "/dev/mmcblk1p1" ] \
        && dev="/dev/mmcblk1p1" \
        || dev="/dev/mmcblk1"
    echo "========= To check ${dev} =========="

    # mounted allready
    mnt_point=$(mount | grep "${dev}" | awk '{print $3}' | head -n 1)
    if [ -d "${mnt_point}" ]; then
        echo "========= allready mount on ${mnt_point} =========="

        echo -e "========= do rw/check now ==========\n"
        rwcheck -p 1 -t 1 -s 10M -d ${mnt_point} && exit 0 || exit 1
    fi

    # mount by self
    mnt_point="/tmp/$(basename ${dev})"
    mkdir -p ${mnt_point}
    if mount ${dev} ${mnt_point} &>/dev/null; then
        echo "========= Mount ${dev} to ${mnt_point} ========="

        echo -e "========= do rw check now ==========\n"
        rwcheck -p 1 -t 1 -s 10M -d ${mnt_point}

        clear_and_exit $?
    fi

    # disable format
    if [ "${can_format}" = "false" ]; then
        echo "========= Mount Failed and DISABLE to format ========="
        exit 0
    fi

    # enable format
    echo -e "========= Try mount & rw check after formating ========="
    ! mkfs.ext4 -F ${dev} \
        && echo "========= mkfs.ext FAILED =========" \
        && clear_and_exit 1
    ! mount ${dev} ${mnt_point} \
        && echo "========= still mount FAILED =========" \
        && clear_and_exit 1
    ! rwcheck -p 1 -a -t 1 -s 10M -d ${mnt_point} \
        && echo "========= rw check FAILED =========" \
        && clear_and_exit 1

    echo "========= Check PASS ========="
    clear_and_exit 0
}

boot_media=$(get_boot_media)
can_format=$(mjson_fetch /base/production/emmctester/can_format)
echo "Boot Media: ${boot_media}"
echo "Enable Format: ${can_format}"

case "$(get_boot_media)" in
    emmc|nand|nor-flash)
        echo "No need to check"
        exit 0
		;;
	sdcard)
        check_emmc
		;;
esac
