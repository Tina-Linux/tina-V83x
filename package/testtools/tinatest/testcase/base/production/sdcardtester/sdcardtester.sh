#!/bin/sh

get_dev()
{
    local link
    for dev in $(ls /sys/block | grep -w "mmcblk[[:digit:]]"); do
        mmc="/dev/${dev}"
        [ ! -b "${mmc}" ] && continue

        link="$(readlink "/sys/block/${dev}")"
        # sdc tfcard
        echo ${link} | grep "sdc0" &>/dev/null && return 0
        echo ${link} | grep "sunxi-mmc.0" &>/dev/null && return 0
        # spi tfcard
        echo ${link} | grep "spi.*mmc_host" &>/dev/null && return 0
    done
    return 1
}

get_capacity()
{
    fsize="/sys/block/${dev}/size"
    ! [ -f "${fsize}" ] \
        && echo "get capacity failed" \
        && return 1

    capacity="$(cat ${fsize} | awk '{sum+=$0}END{printf "%.2f\n",sum/2097152}')"
    echo "${mmc} : ${capacity}G"
    return 0
}

################################ start here ###################################
boot_type=$(get_boot_media)
echo "boot from ${boot_type}"

[ "${boot_type}" == "sdcard" ] && echo "checking is't needed" && exit 0

while true; do
    sleep 1
    get_dev \
        && echo "Found sdcard: ${mmc}" \
        || continue
    get_capacity \
        && exit 0 \
        || exit 1
done
