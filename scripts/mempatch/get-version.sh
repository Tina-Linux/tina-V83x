#!/bin/bash
# get-version.sh <lichee1> [<lichee2> ...]
# Nand: 只支持从tinaSDK中获取
# Emmc: 支持sunxi-dev和tina-dev

source $(dirname $0)/target.tb

get_kernel_mmc()
{
    local kdriver="$1/drivers/mmc"
    case "$1" in
        *-3.4)
            local kversion="$(grep '#define *DRIVER_RIVISION.*' ${kdriver}/host/sunxi-mci.h \
                | awk -F'[ "]' '{print $4 " " $5 " " $6}')"
            ;;
        *)
            local kversion="$(grep '#define *DRIVER_RIVISION.*' ${kdriver}/host/sunxi-mmc.h \
                | awk -F'[ "]' '{print $4 " " $5 " " $6}')"
            ;;
    esac
    echo ${kversion}
}

get_uboot_mmc()
{
    local udriver="$1/drivers/mmc"
    local uversion="$(grep '#define *DRIVER_VER.*' ${udriver}/mmc_def.h \
        | awk -F'[ "]' '{print $5 " " $6}')"
    echo ${uversion}
}

get_kernel_version()
{
    local list="$(find ${ROOT}/target/allwinner -maxdepth 1 -name "$1-*" -type d)"
    local one ver version platform
    for one in ${list}
    do
        grep -q 'common$' <<< "${one}" && continue
        [ -f "${one}/Makefile" ] || continue
        unset ver
        ver="$(awk -F":=" '/KERNEL_PATCHVER/{print $2}' ${one}/Makefile)"
        if [ -z "${version}" ]; then
            platform="${one}"
            version="${ver}"
        else
            [ "${version}" = "${ver}" ] && continue
            echo -en "\033[31mkernel version diff: "
            echo -en "$(basename ${one}) (${ver}) Vs. "
            echo -e "$(basename ${platform}) (${version})\033[0m"
            [ "$(echo "${ver} > ${version}" | bc)" -eq 1 ] && version="${ver}"
        fi
    done
    kernel="linux-${version}"
}

get_uboot_version()
{
    local mk="${ROOT}/target/allwinner/${target%%-*}-common/BoardConfigCommon.mk"
    uboot=$(awk -F= '/TARGET_UBOOT_VERSION/{print $2}' ${mk} 2>/dev/null)
    [ -n "${uboot}" ] && uboot="u-boot-${uboot}"
    if [ "${uboot}" = "u-boot-2011.09" ]; then
        boot0="uboot_2011_sunxi_spl"
    elif [ "${uboot}" = "u-boot-2014.07" ]; then
        boot0="uboot_2014_sunxi_spl"
    elif [ "${uboot}" = "u-boot-2018" ]; then
        boot0="spl"
    fi

}

get_kernel_nand()
{
    local kdriver="$1"
    local kversion=$(grep '#define *NAND_DRV_VERSION_.*' ${kdriver}/nfd/nand_osal_for_linux.c \
        | awk '{print $3}' \
        | sed 's/0x0*//;N;s/\n//;s/0x/./')
    echo ${kversion}
}

get_uboot_nand()
{
    case "$1" in
        "*2011*")
            local udriver="$1/nand_sunxi/${chip%p*}"
            ;;
        *)
            local udriver="$1/nand_sunxi/${chip}"
            ;;
    esac
    local uversion=$(grep '#define *NAND_DRV_VERSION_.*' ${udriver}/osal/nand_osal_uboot.c \
        | awk '{print $3}' \
        | sed 's/0x0*//;N;s/\n//;s/0x/./')
    echo ${uversion}
}

get_mmc_version()
{
    echo "============================================"
    echo "[mmc] $@"
    echo "============================================"
    # linux
    for linux in $(find $@ -maxdepth 1 -name "linux-*")
    do
        echo "[$(basename ${linux})]: $(get_kernel_mmc ${linux})"
    done
    # uboot
    for uboot in $(find $@/brandy -maxdepth 1 -name "u-boot*")
    do
        echo "[$(basename ${uboot})]: $(get_uboot_mmc ${uboot})"
    done
}

get_chip()
{
    local fconfig
    fconfig="${ROOT}/target/allwinner/$1-*/config-$(awk -F'-' '{print $2}' <<< $2)"
    chip="$(awk -F'[_=]' '/CONFIG_ARCH_SUN.*P.*=y/{print tolower($3)}' ${fconfig} | head -n 1)"
}

get_nand_version()
{
    echo "============================================"
    echo "[nand] $1"
    echo "============================================"
    # linux
    for info in ${table[@]}
    do
        platform="$(awk -F: '{print $1}' <<< ${info})"
        target="$(awk -F: '{print $1}' <<< ${info})"
        get_kernel_version ${target}
        get_uboot_version ${kernel}
        get_chip ${target} ${kernel}

        case "${kernel}" in
            *-3.4*|*-3.10*)
                local kdriver="$1/${kernel}/drivers/block/nand/${chip}"
                ;;
            *)
                local kdriver="$1/${kernel}/modules/nand/${chip}"
                ;;
        esac
        awk -F: '{for (i=2; i<=NF; i++) {printf "[%s]-", $i}}' <<< ${info}
        echo "[${kernel}]: $(get_kernel_nand ${kdriver})"
    done
}

get_libdrampath()
{
    #1-boot0, 2-kernel, 3-chip
    local dramfile=libdram
    local kernel=$2
    local chip_short=$3

    case $3 in
	sun8iw15p1)
	    dramfile=libdram_ddr3
	    ;;
	sun8iw6p1)
	    dramfile=libdram-iot
	    ;;
    esac

    if [ ${kernel} = "linux-3.4" ]; then
        # delete "p1", such as: sun8iw5p1->sun8iw5
        chip_short=`echo $chip_short | awk '{sub(/.{2}$/,"")}1'`
    fi
    if [ ${chip_short} = "sun8iw18p1" ]; then
        libdrampath=${ROOT}/lichee/brandy-2.0/${1}/drivers/dram/${chip_short}/$dramfile
    else
        libdrampath=${ROOT}/lichee/bootloader/${1}/sunxi_spl/dram/${chip_short}/dram/$dramfile
    fi
}

get_boot0_dram()
{
    local filter="DRIVE INFO:"
    local NextLineFoundVersion=no
    local chip=$1
    local file=$2

    [ $chip = sun50iw1p1 ] && filter="driver version:"

    while read line
    do
        [ $NextLineFoundVersion = yes ] && {
                version=`echo "$line" | cut -d "V" -f 2 | cut -c1-4`
                break;
        }
        res=`echo $line |grep "$filter"`
        [ $? -eq 0 ] && {
            [ $chip = sun8iw18p1 ] && {
                version=`echo "$line" | cut -d "V" -f 2 | cut -c1-4`
                break;
            }
            version=`echo "$res" | cut -d ":" -f 2`
            if [ `echo $version |grep V` ]; then
              version=`echo $version | cut -d "V" -f 2`
              break;
            else
              NextLineFoundVersion=yes
            fi
        }
    done < $file
    echo "V$version"
}

get_dram_version()
{
    echo "============================================"
    echo "[dram] $1"
    echo "============================================"
    # linux
    for info in ${table[@]}
    do
        platform="$(awk -F: '{print $1}' <<< ${info})"
        target="$(awk -F: '{print $1}' <<< ${info})"
        get_kernel_version ${target}
        get_uboot_version ${kernel}
        get_chip ${target} ${kernel}

        get_libdrampath ${boot0} ${kernel} ${chip}
        awk -F: '{for (i=2; i<=NF; i++) {printf "[%s]-", $i}}' <<< ${info}
        echo "[${boot0}]: $(get_boot0_dram ${chip} ${libdrampath})"
    done
}

get_version()
{
    local act
    for act in ${action}
    do
        ${act} $@
    done
}

main()
{
    opts=$(getopt -o 'mnd' -- $@) || return 1
    eval set -- "${opts}"
    while true
    do
        case "$1" in
            -h)
                show_help
                exit 0
                ;;
            -m)
                shift
                action="${action} get_mmc_version"
                ;;
            -n)
                shift
                action="${action} get_nand_version"
                ;;
            -d)
                shift
                action="${action} get_dram_version"
                ;;
            --)
                shift
                break
        esac
    done
    [ -z "${action}" ] \
        && action="get_mmc_version get_nand_version get_dram_version"

    for once in $@
    do
        once="$(sed 's#/$##' <<< ${once})"
        unset lichee
        if grep -E 'lichee$' <<< ${once} &>/dev/null; then
            lichee="${once}"
        elif [ -d "${once}/lichee" ]; then
            lichee="${once}/lichee"
        fi
        [ -z "${lichee}" ] \
            && echo -e "\033[31mInvalid Root: ${once}\033[0m" \
            && continue
        ROOT="${once}"
        get_version ${lichee}
    done
}

main $@
