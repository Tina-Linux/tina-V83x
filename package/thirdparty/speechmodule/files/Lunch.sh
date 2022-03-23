#arecord -D vad -c 8 -r 16000 -f S16_LE -d 1 -t raw /tmp/test.pcm
#rm /tmp/test.pcm
echo 0x60 0x40ff01C2 > /sys/kernel/debug/vad/reg
echo 0x5c 0x000e2020 > /sys/kernel/debug/vad/reg

ln -s /oem/tts /data/tts
ln -s /oem/res /data/res

export  AISPEECH_WIFI_CFG="/data/cfg/wpa_supplicant.conf"

export  PATH=/bin:/sbin:/usr/bin:/usr/sbin:/userdata:/data/res/

#0 for 0db, 3 for 20db
amixer cset name='ADC MIC Group 2 Left Volume' 0
amixer cset name='ADC MIC Group 3 Left Volume' 3
amixer cset name='ADC MIC Group 3 Right Volume' 3

#0~31for -18~28.5db, step in is 1.5db
amixer cset name='ADC ALC Group 2 Left Volume' 18
amixer cset name='ADC ALC Group 3 Left Volume' 18
amixer cset name='ADC ALC Group 3 Right Volume' 18

#0 ~ 3 for -6 ~ 0 db
amixer cset name='DAC LINEOUT Left Volume' 0
amixer cset name='DAC LINEOUT Right Volume' 0

#Digital gain, 0~99 for -40~0db
if [ -e /data/cfg/volume_config ]; then
	vol=$(cat /data/cfg/volume_config)
else
	vol=90
fi
amixer cset name='Master Playback Volume' $vol%

#enable/disable hardware vad
amixer cset name='vad switch' 0

if [ -f ${AISPEECH_WIFI_CFG} ]; then
	ifconfig wlan0 down
	ifconfig wlan0 up
	wpa_supplicant -B -i wlan0 -c ${AISPEECH_WIFI_CFG} &
	wl down;wl band b;wl up
	dhcpcd &
fi

function get_result() {
	targets="/data/.item_echo_bt_test_result /data/.item_echo_micfit_test_result /data/.item_echo_register_license_result /data/.item_echo_wlan_test_result /data/.item_echo_serial_test_result"
	count=0
	i=0
	#for f in /data/.echo*
	for f in $targets
	do
		#echo $f
		result=`cat $f`
		[ $result == "PASS" ] && {
		count=$(($count + 1))
		#echo $count
					}
		i=$(($i + 1))
	done
	if [ $i -eq $count ];then
		#echo success
		rm /data/.item_echo*
		sync
		return 0
	fi
	return 1
}


if [ -f /data/.pcba_test_result ]; then
	count=`grep fail /data/.pcba_test_result -cw`
	if [ $count -eq 0 ];then
		get_result
		if [ $? -eq 0 ];then
			rm /oem/.initial
			find /userdata/ -type l -name "echo_*" -exec rm -rf {} \;
		fi
	fi
	rm /data/.pcba_test_result
	sync
else
	get_result
	if [ $? -eq 0 ];then
		rm /oem/.initial
		find /userdata/ -type l -name "echo_*" -exec rm -rf {} \;
		sync
	fi
fi


if [ -f /oem/.initial ];then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oem/libs/:/oem/factorytest/libs/
	export PATH=$PATH:/data/:/oem/factorytest/bin/
	ln -s /oem/factorytest/bin/* /data/
	echo_pcbatest_server &
#	echo_auto_test echo_wlan_test &
#	echo_auto_test echo_bt_test &
else
	aplay /oem/AC/wav157.wav
	#for factory test
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oem/libs/:/oem/factorytest/libs/
	/oem/deamon &
	./oem/wifi_monitor.sh &
fi


ntpd -n -N -p cn.ntp.org.cn -p 0.cn.pool.ntp.org \
           -p 1.asia.pool.ntp.org -p 2.asia.pool.ntp.org \
           -p 3.asia.pool.ntp.org >/dev/null 2>&1 &
