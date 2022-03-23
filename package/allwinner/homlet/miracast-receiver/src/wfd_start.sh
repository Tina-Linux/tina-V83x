#!/bin/sh

echo "1048576" > /proc/sys/net/core/rmem_max
echo "1048576" > /proc/sys/net/core/rmem_default

killall -9 wpa_supplicant
killall -9 MiracastReceiver
wpa_supplicant -ip2p0 -Dnl80211 -c/etc/p2p_supplicant.conf -O/var/run/wpa_supplicant &
MiracastReceiver -p/var/run/wpa_supplicant
