#!/bin/ash
###########################################################
#function:standby stress test.
#author:  flyranchao@allwinnertech.com
#time:   create at    2017/10/10
#        refator at   2018/12/20
###########################################################

###########################################################
# Before do standby, reboot first
# If call standby on tinatest by adb, this script will do
# standby without any log or tinatest will go wrong.
# To fix it, I set this test as may_reboot, and set
# run_times to 2, which enable reboot before do standby test.
# By this, this will reboot in first time and do standby
# test in next time
may_reboot=$(mjson_fetch ${testcase_path}/may_reboot)
if [ "$may_reboot" = "true" ]
then
    REBOOT_STATUS="/mnt/UDISK/tinatest.reboot.status"
    [ -f "${REBOOT_STATUS}" ] && \
        [ "$(head -n 1 ${REBOOT_STATUS})" == "0" ] && reboot -f
fi
###########################################################

SCENE_CHOOSE=$1

testcase_path="/stress/standby"
# TEST_NUMBERS - counts of test round
TEST_NUMBERS=`mjson_fetch ${testcase_path}/test_numbers`
# STANDBY_PERIOD - standby time
STANDBY_PERIOD=`mjson_fetch ${testcase_path}/standby_period`
# RUNNING_PERIOD - running time
RUNNING_PERIOD=`mjson_fetch ${testcase_path}/running_period`

help_info()
{
	echo ""
	echo "*****************************************************************"
	echo "Three input parameters:"
	echo "1. counts of test numbers : TEST_NUMBERS"
	echo "2. standby period       : STANDBY_PERIOD"
	echo "3. running period       : RUNNING_PERIOD"
	echo "*****************************************************************"
}

#get the target
auto_get_target()
{
	TARGET=$(get_target)
	echo platform = $TARGET
	case $TARGET in
		r6-* | r11-* | r7-*| r328s2-* | r328s3-*)
			STANDBY_TYPE="normal_standby"
			;;
	        r16-*)
		        STANDBY_TYPE="earlysuspend"
			;;
	        r40-* | r18-* | r30-*)
		        STANDBY_TYPE="super_standby"
			;;
	        *)
		        STANDBY_TYPE="super_standby"
			;;
	esac
}

save_env()
{
	backup_console_suspend=$(cat /sys/module/printk/parameters/console_suspend)

	backup_initcall_debug=$(cat /sys/module/kernel/parameters/initcall_debug)

	if [ -f /sys/power/scene_lock ]; then
		backup_scene_lock=`cat /sys/power/scene_lock | awk -vRS="]" -vFS="[" '{print $2}'`
		echo backup_scene_lock = $backup_scene_lock
	fi

	if [ -f /sys/power/wake_lock ];then
		backup_wake_lock=`cat /sys/power/wake_lock`
		echo backup_wake_lock = $backup_wake_lock
	fi
}

restore_env()
{
	if [ -f /sys/power/scene_lock ]; then
		#restore scene_lock
		for scene in $backup_scene_lock; do
			echo $scene > /sys/power/scene_lock >> /dev/NULL
		done
	fi

	if [ -f /sys/power/wake_lock ];then
		echo $backup_wake_lock > /sys/power/wake_lock
	fi

	echo $backup_console_suspend > /sys/module/printk/parameters/console_suspend

	#restore initcall_debug setting value
	echo $backup_initcall_debug > /sys/module/kernel/parameters/initcall_debug

	#restore the system auto wakeup para
	case $TARGET in
		r328s2-* | r328s3-*)
			echo 0  > /sys/power/sunxi_debug/time_to_wakeup_ms
			;;
		*)
			echo 0 > /sys/module/pm_tmp/parameters/time_to_wakeup
			;;
	esac
}

set_auto_wakeup()
{
	echo Y > /sys/module/printk/parameters/console_suspend
	echo N > /sys/module/kernel/parameters/initcall_debug
	echo 8 > /proc/sys/kernel/printk
	if [ -z $1 ];then
		echo "set auto suspend time :5s"
		case $TARGET in
			r328s2-* | r328s3-*)
				echo 1000 > /sys/power/sunxi_debug/time_to_wakeup_ms
				;;
			r311-* | r30-* | t7-* | mr133-*)
				echo 5000 > /sys/module/pm/parameters/time_to_wakeup_ms
				;;
			r18-*)
				echo 1000 > /sys/power/sunxi/time_to_wakeup_ms
				;;
			r6-*)
				echo 5 > /sys/module/pm_tmp/parameters/time_to_wakeup
				;;
			r7-* | r11-*| r7s-* | r332-* | r331-* | r16-* | r58-*)
				echo 5000 > /sys/module/pm_tmp/parameters/time_to_wakeup
				;;
			*)
				echo 1000 > /sys/module/pm_tmp/parameters/time_to_wakeup
				echo 1000 > /sys/module/pm/parameters/time_to_wakeup_ms
				;;
		esac
	else
		case $TARGET in
			r7-* | r11-* | r7s-* | r332-* | r331-* | r16-* | r58-*)
				time_to_wakeup=$(busybox expr $1 \* 1000)
				echo $time_to_wakeup > /sys/module/pm_tmp/parameters/time_to_wakeup
				;;
			r6-*)
				time_to_wakeup=$1
				echo $time_to_wakeup > /sys/module/pm_tmp/parameters/time_to_wakeup
				;;
			r311-* | r30-* | t7-* | mr133-*)
				time_to_wakeup=$(busybox expr $1 \* 1000)
				echo $time_to_wakeup > /sys/module/pm/parameters/time_to_wakeup_ms
				;;
			r18-*)
				echo $time_to_wakeup > /sys/power/sunxi/time_to_wakeup_ms
				;;
			r328s2-* | r328s3-*)
				time_to_wakeup=$(busybox expr $1 \* 200)
				echo $time_to_wakeup > /sys/power/sunxi_debug/time_to_wakeup_ms
				;;
			*)
				if [ -f /sys/class/rtc/rtc0/wakealarm ]; then
					echo 0 > /sys/class/rtc/rtc0/wakealarm
					echo "+$1" > /sys/class/rtc/rtc0/wakealarm
					wakeup_time=`cat /sys/class/rtc/rtc0/wakealarm`
					return 0
				fi
				if [ -f /sys/class/rtc/rtc0/time ]; then
					echo 0 > /sys/class/rtc/rtc0/time
					echo "+$1" > /sys/class/rtc/rtc0/time
					wakeup_time=`cat /sys/class/rtc/rtc0/time`
					return 0
				fi
				time_to_wakeup=$1
				;;
		esac

		echo "set auto suspend time $time_to_wakeup"
	fi
}

clear_all_wake_lock()
{
	if [ -f /sys/power/wake_lock ]; then
		wake_lock_valid=`cat /sys/power/wake_lock`
		if [ "x$wake_lock_valid" != "x" ]; then
			for wake_lock_tmp in $wake_lock_valid; do
				echo $wake_lock_tmp > /sys/power/wake_unlock
			done
		fi
	fi
}

enter_standby()
{
	echo mem > /sys/power/state
}

suspend_resume_test()
{
	auto_get_target

	[ -n $2 ] && STANDBY_PERIOD=$2
	[ -z ${STANDBY_PERIOD} ] && STANDBY_PERIOD=5
	echo "STANDBY_PERIOD == ${STANDBY_PERIOD}s"

	[ -n $3 ] && RUNNING_PERIOD=$3
	[ -z ${RUNNING_PERIOD} ] && RUNNING_PERIOD=5
	echo "RUNNING_PERIOD == ${RUNNING_PERIOD}s"

	[ -n $1 ] && TEST_NUMBERS=$1
	[ -z ${TEST_NUMBERS} ] && TEST_NUMBERS=5
	echo "TEST_NUMBERS == ${TEST_NUMBERS}"

	num=1
	while [ $num -le $TEST_NUMBERS ]
	do
		echo -e "======================================="
		echo -e "Begin: rounds No.$num !"
		echo
		num=$(($num+1))


		# set standby period
		set_auto_wakeup $STANDBY_PERIOD

		# Set standby type: normal, super or earlysuspend
		if [ -f /sys/power/scene_lock ]; then
			grep $STANDBY_TYPE /sys/power/scene_lock > /dev/null
				if [ $? -eq 0 ]; then
					echo $STANDBY_TYPE > /sys/power/scene_lock >> /dev/NULL
				fi
		fi

		# for earlysuspend, remove the wake_tmp wake lock.
		if [ "x$STANDBY_TYPE" == "xearlysuspend" ]; then
			clear_all_wake_lock
		fi

		# enter standby
		echo -e "===>enter standby<==="
		echo
		enter_standby
		echo
		echo -e "===>exit  standby<==="
		sleep $RUNNING_PERIOD

		# for earlysuspend, enter_standby is non-blocking, so we must ensure:
		# 1. It must have entered standby.
		# 2. Preventing the system from entering standby again after it resumed.
		if [ "x$STANDBY_TYPE" == "xearlysuspend" ]; then
			if [ -f /sys/power/wake_lock ]; then
				sleep 6
				echo none > /sys/power/state
			fi
		fi
		echo "=====================resume ok  $(($num-1))!!!===================="
		echo
done
}

#===================================main function===============================
[ -n $1 ] && [ x$1=x"-h" ] && help_info

if [ -z $1 ];then
	save_env
	suspend_resume_test $1 $2 $3
	restore_env
	[ $(($num-1)) -l $TEST_NUMBERS ] && exit 1
	exit 0
fi

