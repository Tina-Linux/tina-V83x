#!/bin/sh
##############################################################################
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

start_wpa_supplicant() {
    echo "====== start wpa supplicant ======"
	rm -rf /var/run/wpa_supplicant/wlan0
	echo ctrl_interface=/var/run/wpa_supplicant > /data/supplicant.conf
	wpa_supplicant  -B -i wlan0  -D nl80211 -c /data/supplicant.conf
    echo "====== wpa supplicant end ======"
}

check_wifiscan() {
    echo "====== CHECKING scan ======"
    [ ! "$(wpa_cli -i wlan0 scan)" = "OK" ] \
        && echo "failed:wlan0 can not do scaning" \
        && echo "====== CHECKING scan FAILED ======" \
        && continue

    sleep 3 #wait for scaning
    aps=`wpa_cli -i wlan0 scan_results | awk 'NR>1 && NR<5 {print $5}'`
    if [ -n "${aps}" ]; then
        echo "Scanned APs:"
        echo "${aps}"
        return 0
    fi

    echo "scan nothing. Make sure there are APs around"
    echo "====== CHECKING scan FAILED ======"
    return 1
}

down_wlan0() {
	ifconfig wlan0 down
	sleep 1
}

stop_wpa_supplicant() {
	killall wpa_supplicant
	sleep 2
}

check_wifi() {
    for cnt in $(seq 1 ${loop_times})
    do
        echo
        echo "-------------------------- start ${cnt} times  -----------------------------------"

		! check_wlan0 && down_wlan0 && continue

		start_wpa_supplicant

		! check_wifiscan && down_wlan0 && stop_wpa_supplicant && continue

		stop_wpa_supplicant
		return 0
	done
	return 1
}

loop_times="$(mjson_fetch /base/production/wifi/rt_wifitester/max_test_times)"

echo "target: $(get_target)"
echo "max_test_times : ${loop_times}"

check_wifi && echo "wifi test pass" && exit 0

echo
echo "wifi test failed"
exit 1
