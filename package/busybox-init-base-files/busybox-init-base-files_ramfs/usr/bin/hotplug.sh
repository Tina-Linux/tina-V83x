#!/bin/sh

######################################################################
# file hotplug.sh
# version 2.0.0
# date 2017-10-11
# author liumingyuan <liumingyuan@allwinnertech.com>
# Copyright (c) 2017 Allwinner Technology. All Rights Reserved.
######################################################################

case $ACTION in
remove)
	umount /mnt/$MDEV
	rmdir /mnt/$MDEV
	 ;;
add)
    mkdir -p /mnt/$MDEV
	mount /dev/$MDEV /mnt/$MDEV
	 ;;
esac

exit 0
