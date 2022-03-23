#!/bin/sh

echo "1048576" > /proc/sys/net/core/rmem_max
echo "1048576" > /proc/sys/net/core/rmem_default

killall -9 wpa_supplicant
killall -9 p2p_gc
killall -9 dnsmasq
killall -9 udhcpc

if [ ! -d "/var/run" ]; then
  mkdir /var/run
fi

wpa_supplicant -ip2p0 -Dnl80211 -c/etc/p2p_supplicant_gc.conf -O/var/run/wpa_supplicant &
p2p_gc
