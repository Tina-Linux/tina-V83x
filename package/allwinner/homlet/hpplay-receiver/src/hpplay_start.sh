#!/bin/sh

echo "1048576" > /proc/sys/net/core/rmem_max
echo "1048576" > /proc/sys/net/core/rmem_default

killall -9 mdnsd_lebo
killall -9 dlna_app
killall -9 demo_player
mdnsd_lebo &
dlna_app &
demo_player
