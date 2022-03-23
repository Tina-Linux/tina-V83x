#!/bin/sh

##############################################################################
# \version     1.0.0
# \date        2017-09-20
# \author      xuri <xuripsw@allwinnertech.com>
# \descriptions
#	       sleep 5s,test t2-t1 whether equal to 5.
##############################################################################

hc_seconds()
{
    echo $1 | awk -F: '{print $1 * 3600 + $2 * 60 + $3}'
}

rtc=$(ls /dev/ | grep rtc)
if [ -z "${rtc}" ]
then
    echo "ERROR: RTC device doesn't exist!"
    exit 1
else
    echo "RTC exist: ${rtc}"
fi

hc0=$(hwclock -f /dev/${rtc} | awk '{print $4}')
sleep 5
hc1=$(hwclock -f /dev/${rtc} | awk '{print $4}')

time0=$(hc_seconds $hc0)
time1=$(hc_seconds $hc1)
echo "time0: $hc0"
echo "time1: $hc1"

sub=$(expr $time1 - $time0)
echo
echo "sleep 5s"
echo "rtc count: ${sub}s"
echo
if [ "$sub" == 5  ]
then
    echo "rtc work well."
    exit 0
else
    echo "ERROR:rtc is broken"
    exit 1
fi
