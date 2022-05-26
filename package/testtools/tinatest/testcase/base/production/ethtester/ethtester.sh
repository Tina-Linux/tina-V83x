#!/bin/sh
##############################################################################
check_eth0() {
    echo "====== CHECKING eth0 ======"
	ifconfig -a | grep eth0 || {
        echo "Not Found eth0"
        echo "====== CHECKING wlan0 FAILED ======"
        return 1
    }

    echo "====== CHECKING eth0 PASS ======"
    return 0
}

get_ip() {
    echo "====== GET IP ======"
	ip=$(route -n | awk '($1~/^(0.0.0.0)/&&$2!~/^(0.0.0.0)/) {print $2}' | head -n 1)
	[ -z "${ip}" ] && return 0 || return 1
    echo "IP to ping : ${ip}"
    echo "====== GET IP END ======"
}

check_ping() {
    echo "====== CHECKING PING ======"
	result=`ping ${ip} -c 1 | grep "1 packets transmitted" \
			 | awk '{printf ("%s %s %s", $7, $8, $9) }'`
	if [ -z "${result}" ]; then
        echo "====== CHECKING PING FAILED ======"
		return 1
    else
        echo "Ping result:"
        echo "${result}"
        echo "====== CHECKING PING PASS ======"
        return 0
	fi
}

check_ethernet() {
    check_eth0 || return 1
    get_ip || return 1
    check_ping || return 1
    return 0
}

echo "target: $(get_target)"

check_ethernet && echo "eth test pass" && exit 0

echo
echo "eth test failed"
exit 1
