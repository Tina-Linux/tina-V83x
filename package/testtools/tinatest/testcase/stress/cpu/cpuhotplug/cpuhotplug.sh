#!/bin/ash
#get the cpu numbers
get_cpu_nums()
{
	all_cpu=`cat /sys/devices/system/cpu/kernel_max`
	if [ $all_cpu -le 0 ];then
		echo "$TARGET have only one CPU, and can not hotplug it."
		exit 0
	fi
}
#save the env before test
env_save()
{
	autohotplug_file="/sys/kernel/autohotplug/enable"
	if [ -f "$autohotplug_file" ]; then
		hotplug_enable=`cat /sys/kernel/autohotplug/enable`
		if [ $hotplug_enable -eq 1 ]; then
			echo 0 > /sys/kernel/autohotplug/enable
		fi
	fi
	for i in $(seq 1 $all_cpu)
	do
		cpuonline_file="/sys/devices/system/cpu/cpu$i/online"
		if [ -z $cpuonline_file ];then
			cpux_on=`cat /sys/devices/system/cpu/cpu$i/online`
			cpus_status="$cpus_status $cpux_on"
		fi
	done

}

#disble all non-boot cpus
disable_non_boot_cpu()
{
	for i in $(seq 1 $(($all_cpu+1)))
	do
		cpuonline_file="/sys/devices/system/cpu/cpu$i/online"
		if [ -z $cpuonline_file ];then
			echo 0 > /sys/devices/system/cpu/cpu$i/online
		fi
	done

}

#restore the env after test
env_restore()
{
	if [ $hotplug_enable -eq 1 ]; then
		echo 1 > /sys/kernel/autohotplug/enable
	fi
	for i in $(seq 0 $(($all_cpu)))
	do
		cpuonline_file="/sys/devices/system/cpu/cpu$i/online"
		if [ -z $cpuonline_file ];then
			cpux_saved=`echo $cpus_status | awk '{printk $num}' num="$i"`
			cpux_on=`cat /sys/devices/system/cpu/cpu$i/online`
			if [ "x$cpux_on" != "x$cpux_saved" ]; then
				echo $cpux_saved > /sys/devices/system/cpu/cpu$i/online
			fi
		fi
	done
}
#open or close cpu test
open_close_cpu_test()
{
	cur_count=1
	while [ $cur_count -le $1 ]
	do
	    echo "test time $cur_count"
	    for i in $(seq 1 $all_cpu)
	    do
		result=`cat /sys/devices/system/cpu/cpu$i/online`
		if [ $result -eq 1 ]; then
			echo "kill cpu$i"
			echo "-----------------"
			echo 0 > /sys/devices/system/cpu/cpu$i/online
			temp=`cat /sys/devices/system/cpu/cpu$i/online`
			if [ $temp -ne 0 ]; then
				exit 1
			fi
		else
			echo "bringup cpu$i"
			echo "-----------------"
			echo 1 > /sys/devices/system/cpu/cpu$i/online
			temp=`cat /sys/devices/system/cpu/cpu$i/online`
			if [ $temp -ne 1 ]; then
				exit 1
			fi
		fi
		sleep 1
		online=`cat /sys/devices/system/cpu/online`
		echo "online: [$online]"
		temp_dl="/sys/class/thermal"
		if [ -z $temp_dl ];then
			cur_temp=`cat /sys/class/thermal/thermal_zone0/temp`
			echo temperature=$cur_temp C
			echo ""
		fi
		done
		let cur_count++
	done
}

#===================================MAIN FUNCTION===================================

echo "$0 start"
loop=`mjson_fetch /stress/cpu/cpuhotplug/loop`
if [ -z ${loop} ]; then
	echo "can't get loop!!!"
        exit 1
fi
get_cpu_nums
echo "loop times ${loop}"
echo ""
env_save
disable_non_boot_cpu
open_close_cpu_test ${loop}
env_restore
echo "$0 success"
if [ $cur_count -lt $1 ]; then
	exit 1
fi
exit 0
