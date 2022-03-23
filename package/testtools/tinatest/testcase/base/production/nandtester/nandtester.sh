#!/bin/sh

#mjson variables read

#platform info
boot_type=$(get_boot_media)

case "${boot_type}" in
    nand)
        echo "boot from ${boot_type}"
        echo "checking is no need"
        exit 0; #boot from nand, just exit 0
        ;;
    *)
        echo "boot from ${boot_type}";
        ;;
esac

test_dev="/dev/nanda"
grep "gpt=1" /proc/cmdline &>/dev/null && test_dev="/dev/nand0p1"
[ ! -b ${test_dev} ]  \
    && echo "not found ${test_dev}" \
    && exit 1

echo "nand test ioctl start"
nandrw "${test_dev}"
if [ $? -ne 0 ];then
    echo "nand ioctl failed"
    exit 1
else
    echo "nand ioctl test ok"
    exit 0
fi
