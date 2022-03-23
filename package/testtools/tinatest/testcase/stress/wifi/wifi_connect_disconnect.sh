#!/bin/sh

path_in_config_tree="/stress/wifi"
test_max=`mjson_fetch $path_in_config_tree/connect_count`
fail_count=0
test_count=0
WLANDEV=wlan0
WPA_CLI="wpa_cli -i $WLANDEV -p /etc/wifi/sockets/"
wpa_up()
{
	wpa=`ps | grep wpa_supplicant | grep -v grep`
	[ "$wpa" == "" ] || \
	{
		killall -q -KILL wpa_supplicant
	}

	[ -f /etc/wifi/wpa_supplicant.conf ] && {
		rm -rf /etc/wifi/wpa_supplicant.conf

(cat <<EOF
ctrl_interface=/etc/wifi/sockets
disable_scan_offload=1
update_config=1
EOF
) >/etc/wifi/wpa_supplicant.conf
	}
	echo "start wpa_suplicant!!!!!!!!!!!!!!!!!"
	wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wifi/wpa_supplicant.conf -O/etc/wifi/sockets -B

	cli=`ps | grep wpa_cli | grep -v grep`
	[ "$cli" == "" ] && {
		echo "start wpa_cli daemon"
		${WPA_CLI} -a /usr/bin/wpa_action.sh &
	}
}

wpa_connect()
{
	if [ "$3" == 0 ]; then
		echo "=============Set OPEN Security============="
		${WPA_CLI} remove_network all
		${WPA_CLI} add_network
		${WPA_CLI} disable_network all
		${WPA_CLI} set_network 0 ssid \"$1\"
		${WPA_CLI} set_network 0 priority 0
		${WPA_CLI} set_network 0 key_mgmt NONE
		${WPA_CLI} set_network 0 scan_ssid 1
		${WPA_CLI} enable_network all
		return 0
	elif [ "$3" == 1 ]; then
		echo "=============Set WPA Security============="
		${WPA_CLI} remove_network all
		${WPA_CLI} add_network
		${WPA_CLI} disable_network all
		${WPA_CLI} set_network 0 ssid \"${1}\"
		${WPA_CLI} set_network 0 priority 0
		${WPA_CLI} set_network 0 key_mgmt WPA-PSK
		${WPA_CLI} set_network 0 psk \"${2}\"
		${WPA_CLI} set_network 0 scan_ssid 1
		${WPA_CLI} enable_network all
		return 0
	else
		${WPA_CLI} disconnect
		echo "==========Security ERROR==================="
		return 1
	fi
}

get_ip()
{
	local ip
	ip=ifconfig eth0|grep "inet addr:"|awk -F":" '{print $2}'|awk '{print $1}'
	return $ip
}

wait_connect()
{
	count=0
	while true
	do
		sleep 2
		ip=$(route -n | awk '($1~/^(0.0.0.0)/&&$2!~/^(0.0.0.0)/) {print $2}' | head -n 1) #get route ip
		let count++
		[ $count -eq 15 ] && {
			echo "=======Connect Failed: timed out==========="
			let fail_count++
			break
		}
		[ -z "$ip" ] && continue
		echo "==========GET IP SUCEESS================"
		echo "ROUTE IP: $ip"
		break
	done
}

wifi_scan()
{
	[ ! "$(${WPA_CLI} scan)" = "OK" ] \
		&& echo "failed:wlan0 can not do scaning" \
		&& echo "====== CHECKING scan FAILED ======" \
		&& continue
	sleep 1 #wait for scaning
	aps=`${WPA_CLI} scan_results`
	sleep 1 #wait for scaning
    if [ -n "${aps}" ]; then
		echo "==============SCAN RESULTS================="
        echo "Scanned APs:"
        echo "${aps}"
		echo "==============SCAN Success=================="
        return 0
    fi

    echo "scan nothing. Make sure there are APs around"
    echo "====== CHECKING scan FAILED ======"
    exit 1

}

disconnect()
{
	echo "==========WIFI DISCONNECT================="
	wpa_ctl_file=`ls /tmp/wpa_ctrl_*`
	[ $? -eq 0 ] && \
		rm /tmp/wpa_ctrl_*

	cli=`ps | grep wpa_cli | grep -v grep`
	[ "$cli" == "" ] || {
		echo "stop wpa_cli daemon"
		killall -q -KILL wpa_cli
	}

	wpa=`ps | grep wpa_supplicant | grep -v grep`
	[ "$wpa" == "" ] || {
		${WPA_CLI} remove_network all
		${WPA_CLI} save
		echo "stop wpa_supplicant"
		killall -q -KILL  wpa_supplicant
	}

	udh=`ps | grep udhcpc | grep -v grep`
	[ "$udh" == "" ] || {
		echo "stop udhcpc"
		killall -q -KILL udhcpc
	}
	ifconfig $WLANDEV down
	ifconfig $WLANDEV 0.0.0.0
	sleep 2
	echo "==========WIFI DISCONNECT SUCEESS============"
}

driver_up()
{
	ifconfig -a | grep wlan0 || {
		ifconfig wlan0 up && {
			sleep 2
			echo "=========WIFI UP SUCEESS ==========="
		}
	}

	ifconfig -a | grep wlan0 || {
		echo "===========Not Found wlan0========="
		exit 1
	}
}

enter_info(){
	local number=0
	until [ $number -ge 8 ];do
		ssid=$(task "Please enter the WIFI SSID")
		password=$(task "Please enter WIFI password")
		key_mgmt=$(task "Please enter encryption method of the AP" \
			-n "0:NONE   1:WPA Security")
		test_max=$(task "Please enter test count,default 3")
		number=$(echo $password | wc -c)

		[ "$password" = "NONE" ] && break
		[ $number -lt 8 ] && {
			ttips "The password must be greater than 8 digits,Please enter again."
		}
	done
}

#enter_info

#ttips "  waiting.......... (ssid:$ssid    password:$password)"

ssid="AWTest"
password="1qaz@WSX"
ttips "  waiting.......... (ssid:$ssid    password:$password)"

for i in $(seq 1 $test_max)
do
	driver_up
	wpa_up
	wifi_scan
	wpa_connect $ssid $password $key_mgmt
	[ $? == 0 ] && wait_connect
	disconnect
	let test_count++
done
echo "==============================================="
echo "TEST $test_count times"
echo "Success:$((test_count-fail_count))"
echo "Failed:$fail_count"
echo "==============================================="
exit 0
