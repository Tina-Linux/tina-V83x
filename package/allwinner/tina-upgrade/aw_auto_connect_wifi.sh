#!/bin/sh


[ -f /overlay/etc/wifi/wpa_supplicant.conf ] && {
	echo "get wpa_supplicant from overlay"
	echo "old wpa_supplicant.config"
	cat /etc/wifi/wpa_supplicant.conf
	cp /overlay/etc/wifi/wpa_supplicant.conf /etc/wifi/
	echo "new wpa_supplicant.config"
	cat /etc/wifi/wpa_supplicant.conf
}

[ ! -f /etc/wifi/wpa_supplicant.conf ] && {
	echo "no /etc/wifi/wpa_supplicant.conf"
}

[ -f /etc/wifi/wpa_supplicant.conf ] && {
	echo "get wpa_supplicant"
	cat /etc/wifi/wpa_supplicant.conf
	/etc/wifi/wifi restart
	sleep 2
	/etc/wifi/udhcpc_wlan0 restart
	sleep 2
}

