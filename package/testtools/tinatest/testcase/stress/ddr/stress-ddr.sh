#!/bin/sh

##############################################################################
# \version    1.0.0
# \date       2017-12-25
# \author     liaoweixiong <liaoweixiong@allwinnertech.com>
# \default command
#             memtester 524288/1024 * 30/100 1
# \descriptions
#             <memtester dram_total_size_mb*test_percent loop_times>
#             implement loop_times memtester test test_size_mb
##############################################################################

get_free_size_mb()
{
    sync
    echo 3 > /proc/sys/vm/drop_caches
    echo "$(( $(awk '/^MemFree/{print $2}' /proc/meminfo) / 1024 ))"
}

fix_size()
{
    [ -z "${percent}" ] && percent=0
    [ -z "${size_mb}" ] && size_mb=0

    [ "${percent}" -gt "${max_percent}" -o "${percent}" -lt "0" ] \
        && echo -n "WARNING: percent ${percent}% out of range [0,${max_percent}], " \
        && percent=${max_percent} \
        && echo "reset to ${percent}%"

    [ "${size_mb}" -gt "${total_mb}" ] \
        && echo -n "WARNING: size ${size_mb}MB larger than total ${free}MB" \
        && size_mb="$(( ${total_mb} * ${max_percent} / 100 ))" \
        && echo "reset to ${test_mb}MB (${max_percent}% of total)"

    [ "${size_mb}" -eq 0 -a "${percent}" -eq 0 ] \
        && echo -n "WARNING: both size and percent are 0" \
        && percent=${max_percent} \
        && echo "reset to ${percent}%"
}

get_test_size_by_percent()
{
    left_workers="$1"
    free_mb="$(get_free_size_mb)"
    left_percent="$(( ${percent} - (${total_mb} - ${free_mb}) * 100 / ${total_mb} ))"
    test_percent="$(( ${left_percent} / ${left_workers} ))"
    [ "${test_percent}" -le 0 ] \
        && once_mb=0 \
        || once_mb="$(( ${total_mb} * ${test_percent} / 100 ))"

    echo "#### free mb: ${free_mb}MB ####"
    echo "#### ddr worker ${left_workers} : ${once_mb}MB ${test_percent}% ####"
}

get_test_size_by_size()
{
    left_workers="$1"
    free_mb="$(get_free_size_mb)"
    left_size="$(( ${size_mb} - (${total_mb} - ${free_mb}) ))"
    [ "${left_size}" -le 0 ] \
        && once_mb=0 \
        || once_mb="$(( ${left_size} / ${left_workers} ))"

    echo "#### free mb: ${free_mb}MB ####"
    echo "#### ddr worker ${left_workers} : ${once_mb}MB ####"
}

get_test_size_mb()
{
    [ "${percent}" -gt 0 ] \
        && get_test_size_by_percent $@ \
        || get_test_size_by_size $@
}

do_memtester()
{
    memtester ${once_mb}m ${loop_times}
    ret=$?
    if [ "${ret}" -ne 0 ]; then
        killall memtester &>/dev/null
        echo ${ret} > ${ferr}
    fi
}

#################### MAIN ####################
max_percent=80
ferr="/tmp/mem-test-err"
trap "killall memtester; exit" 2
trap "killall memtester; exit" 15

# get memory info
total_mb="$(( $(awk '/^MemTotal/{print $2}' /proc/meminfo) / 1024 ))"
freq_khz="$(cat /sys/class/devfreq/*/cur_freq 2>/dev/null | head -n 1)"
buffers_mb="$(( $(awk '/^Buffers/{print $2}' /proc/meminfo) / 1024 ))"
cached_mb="$(( $(awk '/^Cached/{print $2}' /proc/meminfo) / 1024 ))"
echo "---------- memory info -----------"
echo "total size: ${total_mb}MB"
echo "cached size: ${cached_mb}MB"
echo "buffer size: ${buffers_mb}MB"
[ -n "${freq_khz}" ] && echo "dram freq: $(( ${freq_khz} / 1000 ))MHz"
echo "free size (now): $(get_free_size_mb)MB"

percent=`mjson_fetch /stress/ddr/percent_of_ddr`
size_mb=`mjson_fetch /stress/ddr/size_mb_of_ddr`
workers=`mjson_fetch /stress/ddr/workers`
loop_times=`mjson_fetch /stress/ddr/loop_times`
echo
fix_size
echo "---------- test control -----------"
echo "total test percent of ddr: ${percent}% (first priority)"
echo "total test size of ddr: ${size_mb}MB (second priority)"
echo "memtester workers: ${workers}"
echo "memtester loop times for each worker: ${loop_times}"

echo -n "NOTE: priority percent > size, it means that if both of percent and size"
echo " are set, only percent work."

echo
echo "---------- memtester -----------"
while [ "${workers}" -gt "0" ]
do
    get_test_size_mb ${workers}
    do_memtester &
    sleep 1
    workers=$(( ${workers} - 1 ))
done
wait

if [ -f "${ferr}" ]; then
    ret=$(cat ${ferr} 2>/dev/null)
    rm -f ${ferr}
fi
exit ${ret}
