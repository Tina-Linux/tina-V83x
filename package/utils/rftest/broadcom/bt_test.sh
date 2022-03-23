#!/bin/sh
tty_dev=$(uci get bluetooth.bsa.device)
if [ -z $tty_dev ]; then
    tty_dev="/dev/ttyS1"
fi
echo "uart device is $tty_dev"

echo 0 > /sys/class/rfkill/rfkill0/state
echo 1 > /sys/class/rfkill/rfkill0/state

brcm_patchram_plus --enable_hci --no2bytes --tosleep=200000 --baudrate 115200 --patchram /lib/firmware/bcm43438a0.hcd  $tty_dev &
sleep  10

#change all bluetooth type rfkillx/state to 1
rootrfkillpath="/sys/class/rfkill"
let i=1
while [ $i -le 20 ]
do
  subrfkillpath=${rootrfkillpath}"/rfkill$i"
  if [ -e ${subrfkillpath} ]; then
    # echo "i=$i ${subrfkillpath} exist"
    if [ -d ${subrfkillpath} ] ; then
      if cat ${subrfkillpath}"/type" | grep bluetooth ; then
	    substatepath=${subrfkillpath}"/state"
		echo 0 > ${substatepath}
		echo 1 > ${substatepath}
	 fi
   fi
fi
let i++
done
sleep 1

#echo "before hci0 up"
hciconfig hci0 up
sleep 1
