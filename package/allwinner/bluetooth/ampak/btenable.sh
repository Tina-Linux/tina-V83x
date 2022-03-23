#!/bin/ash
# $1: on or off

BSA_SERVER=/usr/bin/bsa_server

bt_on()
{
  if ps | grep [b]sa_server ; then
        killall -q -KILL bsa_server
        sleep 2
  fi

  echo 0 > /sys/class/rfkill/rfkill0/state
  sleep 1
  echo 1 > /sys/class/rfkill/rfkill0/state
  sleep 1

  $BSA_SERVER -all=0 -d$1 -pp /lib/firmware -r12 &
  sleep 2
}

bt_off()
{
  killall bsa_server
  sleep 1

  echo 0 > /sys/class/rfkill/rfkill0/state
}

if [ "$1" = "on" ]; then
    echo "turn on bt"
	if [ -z "$2" ]; then
		tmp_device=$(awk 'NR==2 {print $3}' /etc/config/aw_bluetooth)
		tty_num=${tmp_device:1:10}
	else
		tty_num=$2
	fi
    bt_on $tty_num
else
    if [ "$1" = "off" ]; then
        echo "turn off bt"
        bt_off
    else
        echo "no paras"
        exit 1
    fi
fi
