#!/bin/sh
rmmod bcmdhd
sleep 2
insmod bcmdhd.ko iface_name=wlan0 firmware_path=/etc/rftest/fw_bcmdhd_mfg.bin nvram_path=/lib/firmware/nvram.txt
sleep 2
ifconfig wlan0 up
sleep 1
./wl ver
