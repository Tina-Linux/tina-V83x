#!/bin/sh

##############################################################################
# \version     1.0.0
# \date        2017-09-20
# \author      xuri <xuripsw@allwinnertech.com>
# \descriptions
#              Check axp driver exist and print power supply info.
# \check list
#              Board Name   |    Axp Name
#             -----------------------------
#                 R16            axp22_board
#                 R40            axp221s
#                 R18            axp803
##############################################################################

work_probe()
{
    if [ $axp_name = $(cat $axppath | sed 's/platform://g') ]
    then
        exit 0
    else
        ttips "ERROR: $axp_name has some broken"
        exit 1
    fi
}

axp_name=`mjson_fetch /base/production/pmutester/axp_name`

ttrue "AXP Check List:" \
    -n -n "Board name | Axp name" \
    -n "R16 -------- axp22_board" \
    -n "R40 -------- axp221s" \
    -n "R18/R30 ---- axp803" \
    -n "R311 ------- pmu1736" \
    -n -n "Use default configuration?(AXP: ${axp_name})"

[ $? -ne 0 ] && axp_name=$(task "Please input axp name in addition to your board")

for i in $(ls /sys/class/power_supply/)
do
    ps=$(cat /sys/class/power_supply/${i}/uevent | head -n 1)
    if [ "${ps}" != "POWER_SUPPLY_NAME=${i}" ]
    then
        ttips "ERROR: axp-${i} support has some broken"
        exit 1
    fi
done

case $(get_kernel_version) in
"3.4.39")
    axppath="/sys/devices/platform/axp*_board/modalias"
    work_probe
    ;;
"3.10.65"|"4.4.89"|"4.9.56"|"4.9.118")
    axppath="/sys/class/axp/axp_name"
    work_probe
    ;;
*)
    ttips "ERROR: Couldn't detect kernel version."
    exit 1
    ;;
esac
