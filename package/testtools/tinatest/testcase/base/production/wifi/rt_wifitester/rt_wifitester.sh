#!/bin/sh
##############################################################################
# need to be done:
#           make kernel_menuconfig
#    (choose)  Device Drivers------->
#               <*> Network device support------->
#                    <*> wireless LAN------------------>
#                             <*>IEEE 802.11 for.....---------->
#
#
#############################################################################

check_wlan0() {
    echo "====== CHECKING wlan0 ======"
	ifconfig -a | grep wlan0 || {
        echo "Not Found wlan0"
        echo "====== CHECKING wlan0 FAILED ======"
        return 1
    }

    # enable wlan0
    for cnt in `seq 1 3`; do
        ifconfig wlan0 up && {
            echo "ifconfig wlan0 up done"
            echo "====== CHECKING wlan0 PASS ======"
            return 0
        }
        echo "ifconfig wlan0 up failed, try ${cnt} times again 1s later"
        sleep 1
    done
    echo "====== CHECKING wlan0 FAILED ======"
    return 1
}

down_wlan0() {
	ifconfig wlan0 down
	sleep 1
}

check_wifiscan() {
    echo "====== CHECKING scan ======"
    #aps="$(iwlist wlan0 scan | awk -F: '/ESSID/{print $2}' | sed 's/"//g')"
    aps="$(iw dev wlan0 scan | awk -F: '/SSID/{print $2}' | sed 's/"//g')"
    [ -z "${aps}" ] \
        && echo "failed:scan nothing. Make sure there are APs around" \
        && echo "====== CHECKING scan FAILED ======" \
        && return 1

    echo "Scanned APs:"
    echo "${aps}"
    echo "====== CHECKING scan PASS ======"
    return 0
}

check_wifi() {
    for cnt in $(seq 1 ${loop_times})
    do
        echo
        echo "-------------------------- start ${cnt} times  -----------------------------------"

		! check_wlan0 && down_wlan0 && continue
		! check_wifiscan && down_wlan0 && continue

        down_wlan0
		return 0
	done
	return 1
}

loop_times="$(mjson_fetch /base/production/wifi/rt_wifitester/max_test_times)"

echo "target: $(get_target)"
echo "max_test_times : ${loop_times}"

check_wifi && echo "rt wifi test pass" && exit 0

echo
echo "rt wifi test failed"
exit 1
