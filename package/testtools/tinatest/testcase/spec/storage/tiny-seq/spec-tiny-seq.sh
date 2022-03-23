#!/bin/sh

get_total()
{
    df -k ${dir} 2>/dev/null | tail -n 1 | awk '{print $2}'
}

get_free()
{
    df -k ${dir} 2>/dev/null | tail -n 1 | awk '{print $4}'
}

get_ddr()
{
    awk '/^MemTotal/{print $2}' /proc/meminfo
}

get_flash()
{
    df -k ${dir} 2>/dev/null | tail -n 1 | awk '{print $1}'
}

get_fs()
{
    df -T ${dir} 2>/dev/null | tail -n 1 | awk '{print $2}'
}

get_time()
{
    local min="$(awk '/real/{print $2}' $1 | sed 's/m//')"
    local sec="$(awk '/real/{print $3}' $1 | sed 's/[s\.]//g' | sed 's/^0*//')"

    echo $(( ${min} * 60 * 100 + ${sec} ))
}

get_r_speed()
{
    [ -z "${sec}" ] && sec=$(get_time $1)

    [ "${sec}" -le "0" ] \
        && echo "Get Speed Failed" \
        && exit 1

    echo $(( ${test_k} * 100 / ${sec} ))
}

get_w_speed()
{
    [ -z "${sec}" ] && sec=$(get_time $1)

    [ "${sec}" -le "0" ] \
        && echo "Get Speed Failed" \
        && exit 1

    echo $(( ${test_k} * 100 / ${sec} ))
}

# test size depends on free flash and total ddr.
get_test_size()
{
    # suggest 2 times bigger than free ddr
    local size_k="$(( ${ddr_k} * 2 ))"

    # if no enough space in flash, set to 1 times bigger than free ddr
    [ "${size_k}" -gt "${free_k}" ] && size_k="$(( ${ddr_k} * 1 ))"

    # if still no enough space in flash, set to 95% of free flash
    [ "${size_k}" -gt "${free_k}" ] && size_k=$(( ${free_k} * 95 / 100 ))

    # if vfat-fs and bigger than 3.9G, set around 3.9G
    [ "${fs}" = "vfat" -a "${size_k}" -gt $(( 3 * 1024 * 1024 + 921 * 1024 )) ] \
        && size_k=$(( 3 * 1024 * 1024 + 921 * 1024 ))

    echo ${size_k}
}

do_tiny_test()
{
    in="$1"
    out="$2"
    bs="${blk}k"
    count="$(( ${test_k} / ${blk} ))"
    [ "${out}" = "/dev/null" ] \
        && args="if=${in} of=${out} bs=${bs} count=${count}" \
        || args="if=${in} of=${out} bs=${bs} count=${count} conv=fsync"
    echo dd ${args}
    time dd ${args}
}

get_info()
{
    tt_base="/spec/storage/tiny-seq"
    dir=`mjson_fetch ${tt_base}/check_directory`
    blk=`mjson_fetch ${tt_base}/block_size_kb`
    avg=`mjson_fetch ${tt_base}/times_for_average`
    fast=`mjson_fetch ${tt_base}/fast_mode`
    ddr_k="$(get_ddr)"
    total_k="$(get_total)"
    free_k="$(get_free)"
    flash="$(get_flash)"
    fs="$(get_fs)"
    # test size depends on free flash, total ddr and fs.
    test_k="$(( $(get_test_size) / ${blk} * ${blk} ))"

    [ -z "${dir}" ] \
        && echo "Lose directory, set in default: /mnt/UDISK" \
        && dir="/mnt/UDISK"
    [ -z "${blk}" ] \
        && echo "Lose block size, set in default: 4M" \
        && blk=512
    [ -z "${avg}" ] \
        && echo "Lose times for average, set in default: 3" \
        && blk=3
    [ "${blk}" -gt "$(( 4 * 1024 ))" ] \
        && echo -n "block size should not larger than 4m, use 512k instead" \
        && blk=512
    [ -z "${fast}" ] && fast="false"

    ! [ -d "${dir}" ] \
        && echo "No Found: ${dir}, quit!!" \
        && exit 1

    echo "------------------- INFO -------------------"
    echo "fast mode: ${fast}"
    echo "ddr total size: ${ddr_k} KB"
    echo "flash device: ${flash}"
    echo "flash device fs: ${fs}"
    echo "flash device size: ${total_k} KB"
    echo "flash device free: ${free_k} KB"
    echo "test directory: ${dir}"
    echo "test file size: ${test_k} KB"
    echo "test block size: ${blk} KB"
    echo "test times for average: ${avg}"
    echo "------------------- END -------------------"
}

wait_end()
{
    local size=$1
    local sec=0
    while true
    do
        if [ $(( ${sec} % 10 )) -eq 0 ]; then
            echo -ne "\r$2 .         "
            echo -ne "\b\b\b\b\b\b\b\b\b"
        else
            echo -n '.'
        fi
        [ -d "/proc/$!" ] || break
        sleep 1
        sec=$(( sec + 1 ))
    done
    echo " OK (${size}KB in ${sec}s)"
    echo
}

do_test()
{
    if [ "${fast}" != "true" ]; then
        # if flash used space is over 1/2 for MLC/TLC, the speed will drop.
        # So, we should ensure that flash used space is over 1/2
        cnt=$(( (${free_k} - ${test_k}) / 512 * 95 / 100 ))
        [ "${cnt}" -gt "$(( ${free_k} / 2 ))" ] && cnt="$(( ${free_k} / 2 ))"
        dd if=/dev/zero of=${dir}/dd.fill bs=512K conv=fsync count=${cnt} &>/dev/null &
        wait_end "$(( ${cnt} * 512 ))" "filling"
    fi

    log="/tmp/spec-tiny-seq.log"
    r_speed_sum_k=0
    w_speed_sum_k=0

    for one in $(seq ${avg})
    do
        echo "=========== the ${one} times ==========="
        # free memory
        echo
        echo "clear cache"
        time sync
        echo 3 > /proc/sys/vm/drop_caches
        echo

        # do test for write
        rm -f ${log}.w
        do_tiny_test "/dev/zero" "${dir}/dd.tmp" 2>&1 | tee ${log}.w
        w_speed_once_k=$(get_w_speed ${log}.w)
        echo "[${one}]write: ${w_speed_once_k} KB/s"
        echo

        # free memory
        echo
        echo "clear cache"
        time sync
        echo 3 > /proc/sys/vm/drop_caches
        echo

        # do test for read
        rm -f ${log}.r
        do_tiny_test "${dir}/dd.tmp" "/dev/null" 2>&1 | tee ${log}.r
        r_speed_once_k=$(get_r_speed ${log}.r)
        echo "[${one}]read: ${r_speed_once_k} KB/s"
        echo

        w_speed_sum_k="$(( ${w_speed_sum_k} + ${w_speed_once_k} ))"
        r_speed_sum_k="$(( ${r_speed_sum_k} + ${r_speed_once_k} ))"
    done
    rm -f ${log}.r ${log}.w ${dir}/dd.tmp ${dir}/dd.fill
    [ -z "${r_speed_sum_k}" -o -z "${w_speed_sum_k}" ] && echo "Get Speed Failed" && exit 1

    r_speed_k=$(( ${r_speed_sum_k} / ${avg} ))
    w_speed_k=$(( ${w_speed_sum_k} / ${avg} ))
    echo "------------------- SEQ SPEED -------------------"
    echo "seq read: ${r_speed_k} KB/s"
    echo "seq write: ${w_speed_k} KB/s"
    echo "------------------- end -------------------"

    ttips "seq read: ${r_speed_k} KB/s" \
       -n "seq write: ${w_speed_k} KB/s"
}

# ====================== begin here ======================
get_info
do_test
