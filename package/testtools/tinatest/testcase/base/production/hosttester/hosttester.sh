#********************************************************************************
#-->Author:ranchao                                                              |
#-->Emil:flyranchao@allwinnertech.com                                           |
#-->Describe:Recognized The USB devices                                         |
#-->Time:2017/9/18                                                              |
#********************************************************************************
#!/bin/sh
usb_count=`mjson_fetch /base/production/hosttester/usb_count`
record=
count=0

#find usbs loop
while true
do
	for usb in $(ls /sys/bus/usb/devices/ | awk '(NF && $1~/^([0-9]-[0-9])$/)')
	do
		#no usb ,go back the begin of the loop
		[ -z "${usb}" ] && continue

		#no new usb ,go back the begin of the loop
		echo $record | grep "${usb}" &>/dev/null && continue

		#have new usb ,print the bus_port manufacturer product

		ttrue  "bus_port is $usb" \
			-n "manufacturer = `cat /sys/bus/usb/devices/$usb/manufacturer`" \
			-n "product = `cat /sys/bus/usb/devices/$usb/product`" \
			-n "Is the expected usb device info?"
		[ $? == 0 ] && exit 0;

		#Append record the information of new usb
		record="${record} ${usb}"

		#find a new usb then count++
		count=$(($count+1))

		#the detected USB is equal to the set?
		[ "$count" -ne "$usb_count" ] && continue
		exit 1
	done
	sleep 1
done
