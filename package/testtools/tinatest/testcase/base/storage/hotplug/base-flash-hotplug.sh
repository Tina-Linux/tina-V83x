#!/bin/sh

get_media()
{
    local link="$(readlink "/sys/class/block/$(basename $1)")"
    case $1 in
        /dev/mmc*)
            echo ${link} | grep "sdc0" &>/dev/null \
                && echo sdcard && return 0
            echo ${link} | grep "sunxi-mmc.0" &>/dev/null \
                && echo sdcard && return 0
            # spi tfcard
            echo ${link} | grep "spi.*mmc_host" &>/dev/null \
                && echo sdcard && return 0
            ;;
        /dev/sd*)
            echo ${link} | grep "usb" &>/dev/null \
                && echo udisk && return 0
            echo ${link} | grep "sata" &>/dev/null \
                && echo sata && return 0
            ;;
    esac
}

get_mount()
{
    grep -w "^$1" /proc/mounts | awk '{print $2}'
}

record_media()
{
    # 已经在列表中，则返回错误
    [ -z "$2" -o -z "$1" ] && return 1
    echo "${plugin_media}" | grep "$1@$2" &>/dev/null && return 1

    plugin_media="${plugin_media} $1@$2"
}

record_mount()
{
    [ -z "$2" -o -z "$1" ] && return 1
    echo "${plugin_mount}" | grep "$1@$2" &>/dev/null && return 1

    plugin_mount="${plugin_mount} $1@$2"
}

unrecord_media()
{
    [ -z "$2" -o -z "$1" ] && return 1
    echo "${plugin_media}" | grep "$1@$2" &>/dev/null || return 1

    plugin_media="$(echo "${plugin_media}" | sed "s#$1@$2 *##")"
}

unrecord_mount()
{
    [ -z "$2" -o -z "$1" ] && return 1
    echo "${plugin_mount}" | grep "$1@$2" &>/dev/null || return 1

    plugin_mount="$(echo "${plugin_mount}" | sed "s#$1@$2 *##")"
}

check_old()
{
    local dev media mount once

    for once in ${plugin_media}
    do
        dev="$(echo ${once} | sed 's/@.*//')"
        media="$(echo ${once} | sed 's/.*@//')"

        [ -b "${dev}" ] && continue
        unrecord_media "${dev}" "${media}" \
            && echo -e "Plugout:\t${media} @ ${dev}"
    done

    for once in ${plugin_mount}
    do
        dev="$(echo ${once} | sed 's/@.*//')"
        mount="$(echo ${once} | sed 's/.*@//')"

        grep "${dev}.*${mount}" /proc/mounts 1>/dev/null && continue
        unrecord_mount "${dev}" "${mount}" \
            && echo -e "Umount:\t\t${dev} @ ${mount}"
    done
}

check_new()
{
    local dev media mount
    for dev in /dev/mmc* /dev/sd*
    do
        [ -b "${dev}" ] || continue

        media="$(get_media ${dev})"
        mount="$(get_mount ${dev})"
        record_media "${dev}" "${media}" \
            && echo -e "Plugin:\t\t${media} @ ${dev}"
        record_mount "${dev}" "${mount}" \
            && echo -e "Mount:\t\t${dev} @ ${mount}"
    done
}

# ============ begin here ============
while true
do
    check_old
    check_new
done
