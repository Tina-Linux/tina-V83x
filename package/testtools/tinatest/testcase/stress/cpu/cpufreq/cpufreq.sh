#!/bin/ash
help_info()
{
	if [ ! $1 ]; then
		echo "please input test counter"
		exit
	fi
}

#enable_autohotplug
enable_autohotplug()
{

	autohotplug_file="/sys/kernel/autohotplug/enable"
	if [ -f "$autohotplug_file" ]; then
		hotplug_enable=`cat /sys/kernel/autohotplug/enable`
		if [ $hotplug_enable -eq 0 ]; then
			echo 1 > /sys/kernel/autohotplug/enable
		fi
	fi
}

#close autohotplug
close_autohotplug()
{
	autohotplug_file="/sys/kernel/autohotplug/enable"
	if [ -f "$autohotplug_file" ]; then
		hotplug_enable=`cat /sys/kernel/autohotplug/enable`
		if [ $hotplug_enable -eq 1 ]; then
			echo 0 > /sys/kernel/autohotplug/enable
		fi
	fi
}

# enable all non-boot cpus
enable_none_boot_cpu()
{
	all_cpu=`cat /sys/devices/system/cpu/kernel_max`
	if [ $all_cpu -gt 0 ];then
		for i in $(seq 1 $all_cpu)
		do
			echo 1 > /sys/devices/system/cpu/cpu$i/online
		done
	fi
}

# disable all non-boot cpus
disable_none_boot_cpu()
{
	all_cpu=`cat /sys/devices/system/cpu/kernel_max`
	if [ $all_cpu -gt 0 ];then
		for i in $(seq 1 $all_cpu)
		do
			echo 0 > /sys/devices/system/cpu/cpu$i/online
		done
	fi
}

# cpufreq env save
cpufreq_env_save()
{
	close_autohotplug
	disable_none_boot_cpu
	case $TARGET in
		r16-*)
			CPUFREQ_ARRAY=`cat /sys/devices/system/cpu/cpu0/cpufreq/stats/trans_table | awk 'NR==2 {print}' | awk '$1=" " {print}'`
			;;
		r40-*)
			CPUFREQ_ARRAY=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies`
			;;
		*)
			CPUFREQ_ARRAY=`cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies`
			;;
	esac

	governor_save=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
	freq_save=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`

	online=`cat /sys/devices/system/cpu/online`
	echo "online cpu [$online]"
        echo ==================================================================================
        echo cpufreq_array=$CPUFREQ_ARRAY
        echo ==================================================================================
}

# set freq
cpufreq_set()
{
	echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	size=`echo $CPUFREQ_ARRAY | awk '{print NF}'`
	echo ""
	cur_count=1
	while [ $cur_count -le $1 ]
	do
		sleep 1
		echo No.$cur_count
		case "$2" in
			random)
				# ash does not support $RANDOM, so ...
				#number=$RANDOM
				# srand()'s seed, by default, keys off of current date+time.
				# if awk is called multiple times within the same second,
				# you almost certainly will get the same value.
				#number=`awk 'BEGIN { srand(); print int(rand() * 32767); }'`
				number=$(</dev/urandom tr -dc 0-9 2>/dev/null | head -c 5 | sed 's/^0\+//g')
				let "number %= $size"
				let "number += 1"
				freq_target=`echo $CPUFREQ_ARRAY | awk '{print $num}' num="$number"`
				;;
			seq)
				number=$cur_count
				let "number %= $size"
				let "number += 1"
				freq_target=`echo $CPUFREQ_ARRAY | awk '{print $num}' num="$number"`
				;;
			maxmin)
				number=$cur_count
				let "number %= 2"
				if [ $number -eq 1 ];then
					min_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq`
					freq_target=$min_freq
				else
					max_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq`
					freq_target=$max_freq
				fi
				;;
			*)
				return
				;;
		esac

		echo freq_target=$freq_target
		echo $freq_target > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
		echo set $freq_target done!

		cur_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
		if [ $cur_freq -eq $freq_target ];then
			echo Freq[$cur_freq] OK
			thermal_dir="/sys/class/thermal/"
			if [ -d $thermal_dir ];then
				cur_temp=`cat /sys/class/thermal/thermal_zone0/temp`
				echo temperature=$cur_temp C
			fi
		fi

		echo ""
		let cur_count++

	done
}

#cpufreq env restore
cpufreq_env_restore()
{
	echo $freq_save > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
	if [ $? -eq 0 ];then
		echo restore $freq_save done!
	fi
	cpuinfo_cur_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
	echo $governor_save > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	echo Freq[$cpuinfo_cur_freq] restore OK
	enable_none_boot_cpu
	enable_autohotplug
}

#==================================================================================
#main function
#===================================================================================
echo "$0 start"
TARGET=$(get_target)
echo ======$TARGET======

help_info $1

cpufreq_env_save

max_scaling_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq`
min_scaling_freq=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq`
echo "====================================="
echo "min_scaling_freq is $min_scaling_freq"
echo "max_scaling_freq is $max_scaling_freq"
echo "====================================="
echo
echo
echo
echo "=================set freq random===================="
cpufreq_set $1 random
echo "=================set freq seq======================="
cpufreq_set $1 seq
echo "=================set freq maxmin===================="
cpufreq_set $1 maxmin
echo
echo "=================set freq done!!!==================="

cpufreq_env_restore

echo "$0 success"
