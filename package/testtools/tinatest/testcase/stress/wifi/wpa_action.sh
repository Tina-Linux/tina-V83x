#!/bin/sh
case "$2" in
    CONNECTED)
		if ps | grep [u]udhcpc;then
			killall -q -KILL uhdcpc
		fi
		/sbin/udhcpc -i wlan0 -T 10 -s /usr/share/udhcpc/default.script
        ;;
    DISCONNECTED)
		echo "WPA supplicant: connection lost"        >/dev/ttyS1
		ifconfig wlan0 0.0.0.0
        ;;
esac
