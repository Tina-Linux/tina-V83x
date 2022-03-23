#!/bin/sh

# The device use either usb1 or usb2 or...
get_usb()
{
    readlink /sys/class/block/${dev} | awk -F'\/' '{print $6}'
}

record()
{
    checked="${checked} ${usbn}"
    [ "$(echo ${checked} | awk '{print NF}')" -ge ${usb_count} ] \
        && exit 0
}

get_capacity()
{
    fsize="/sys/block/${dev}/size"
    ! [ -f "${fsize}" ] \
        && echo "/dev/${dev}: get capacity failed" \
        && return 1

    capacity="$(cat ${fsize} | awk '{sum+=$0}END{printf "%.2f\n", sum/2097152}')"
    ttips "${usbn} : /dev/${dev} : ${capacity}G"
    return 0
}

usb_count=`mjson_fetch /base/production/udisktester/usb_count`
[ -z "${usb_count}" ] \
    && echo "No set usb count, use default 1" \
    && usb_count=1
echo "usb count to check : ${usb_count}"
echo "-----------------------------"

while true
do
    for dev in $(cd /dev/ && ls sd[a-z] 2>/dev/null)
    do
        [ -b "/dev/${dev}" ] || continue
        usbn=$(get_usb)
        # if checked before, continue loop.
        echo ${checked} | grep "${usbn}" &>/dev/null && continue
        get_capacity && record ${usbn}
    done
    sleep 1
done
