#!/bin/bash

VERSION="0.1.0"
source $(dirname $0)/target.tb

show_help()
{
    echo -ne "Usage: mkpatch-dram.sh -p <platform> [-o <outdir>] [-bh] "
    echo -e "\033[32m<Root of TinaSDK>\033[0m"
    echo -e "Options:"
    echo -e "\t-h: show this help message and exit"
    echo -e "\t-o <outdir>: out directory [default current dir]"
    echo -e "\t-p <platform>: the platform to make"
    echo -e "\t-v: just show driver version and exit"
    echo -e ""
    echo "Eg. ./mkpatch-dram.sh -p r16 ~/worksapce/tina : make dram patch for r16"
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

get_boot0_version()
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

get_chip()
{
    local fconfig
    fconfig="${ROOT}/target/allwinner/$1-*/config-$(awk -F'-' '{print $2}' <<< $2)"
    chip="$(awk -F'[_=]' '/CONFIG_ARCH_SUN.*P.*=y/{print tolower($3)}' ${fconfig} | head -n 1)"
}

get_dram_version()
{
    local filter="DRIVE INFO:"
    local NextLineFoundVersion=no
    local chip=$1
    local file=$2
    local version=

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
    echo "V${version}"
}

get_libdrampath()
{
    #1-boot0, 2-kernel, 3-chip
    local dramfile=libdram
    local kernel=$2
    local chip_short=$3

    boot0binpath=${ROOT}/target/allwinner/${target}-common/bin/boot0_sdcard_${3}.bin
    case $3 in
    sun8iw18p1)
	    boot0binpath=${ROOT}/target/allwinner/${target}-common/bin/boot0_nand_${3}.bin
        ;;
	sun8iw15p1)
	    dramfile=libdram_ddr3
	    boot0binpath=${ROOT}/target/allwinner/${target}-common/bin/boot0_sdcard_${3}_lpddr3.bin
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

check_platform()
{
    up_platform="$(sed 's/[a-z]/\u&/g' <<< "${platform}")"
    info="$(sed 's/ /\n/g' <<< "${table[@]}" | awk "/${up_platform}/{print}")"
    [ -z "${info}" ] && return 1

    up_platform="$(sed 's/[a-z]/\u&/g' <<< "${platform}")"
    target="$(awk -F: '{print $1}' <<< ${info})"
    get_kernel_version ${target}
    get_boot0_version ${kernel}
    get_chip ${target} ${kernel}

    # dram
    get_libdrampath ${boot0} ${kernel} ${chip}
    dramversion=`get_dram_version ${chip} ${libdrampath}`
    boot0_bin_dramversion=`get_dram_version ${chip} ${boot0binpath}`
    [ $boot0_bin_dramversion != $dramversion ] && {
        echo -e "\033[31mDRAM version is $dramversion,"\
		"but boot0 bin version is $boot0_bin_dramversion,"\
		"please updtate boot0 bin\033[0m"
	return 1
    }

    echo "===================================================================="
    echo "platform: ${platform}"
    echo "boot0: ${boot0}"
    echo "chip: ${chip}"
    echo "target: ${target}"
    echo "libdram path: ${libdrampath}"
    echo "boot0 bin path: ${boot0binpath}"
    echo "dram version: ${dramversion}"
    echo "===================================================================="
    [ -z "${platform}" -o -z "${boot0}" -o -z "${chip}" -o -z "${target}" \
        -o -z "${libdrampath}" -o -z "${dramversion}" ] && return 1 || return 0
}

mk_README()
{
cat > ${tmpdir}/README.txt << EOF
注意：
=============================================================
此补丁只适用于更新全志 TinaSDK 的dram 驱动(boot0镜像)
=============================================================

补丁信息：
=============================================================
dram驱动: ${dramversion}
适用方案: ${platform}
补丁提取日期: $(date +%Y-%m-%d\ %H:%M:%S)
=============================================================

文件内容：
=============================================================
bin.tar.gz      : Tina boot0 bin 补丁
README.txt      : 说明文档
md5sum.txt      : 补丁包的md5码
install.sh      : 快速更新脚本
=============================================================

更新步骤：
=============================================================
1. 安装补丁
   ./install.sh <SDK根目录>
   Eg. install.sh ~/workspace/tina : 会替换boot0的bin

3. 打包固件
   替换boot0镜像后，如果之前有编译过固件镜像，则只需要重新pack打包即可
   否则，请按照固件编译方法进行编译
=============================================================
EOF
}

mk_md5sum()
{
    cd ${tmpdir} >/dev/null
    md5sum bin.tar.gz >> ${tmpdir}/md5sum.txt
}

mk_install()
{
    touch ${tmpdir}/install.sh
    chmod +x ${tmpdir}/install.sh

cat > ${tmpdir}/install.sh << EOF
#!/bin/bash

show_help()
{
    echo -e "Usage: install.sh [-ahpu] \\033[32m<Root of TinaSDK >\\033[0m"
    echo -e "Options:"
    echo -e "\\t-p: patch boot0 bin [default]"
    echo -e "\\t-h: show this help message and exit"
    echo -e ""
    echo "Eg. ./install.sh -a ~/worksapce/tina"
}

check_top()
{
    OBJ_ROOT="\$1"

    [ -f "\${OBJ_ROOT}/android/build/envsetup.sh" -a -d "\${OBJ_ROOT}/lichee" ] \\
        && SDK=android && return 0

    [ -f "\${OBJ_ROOT}/build/envsetup.sh" -a -d "\${OBJ_ROOT}/../lichee" ] \\
        && SDK=android && OBJ_ROOT="\${OBJ_ROOT}/.." && return 0

    [ -f "\${OBJ_ROOT}/build/envsetup.sh" -a -d "\${OBJ_ROOT}/target/allwinner" ] \\
        && SDK=tina && return 0

    echo -e "\\033[31mInvalid Root of TinaSDK or AndroidSDK\\033[0m\n"
    return 1
}

check_md5()
{
    ! which md5sum &>/dev/null \\
        && echo -e "\\033[31mPlease Install md5sum\\033[0m" \\
        && return 1

    ! which tar &>/dev/null \\
        && echo -e "\\033[31mPlease Install tar\\033[0m" \\
        && return 1

    [ ! -f \${cur}/md5sum.txt ] \\
        && echo -e "\\033[31mNot Found md5sum.txt In Current Path\\033[0m" \\
        && return 1

    ! md5sum -c md5sum.txt &>/dev/null \\
        && echo -e "\\033[31mCheck md5sum Failed\\033[0m" \\
        && return 1

    return 0
}

check()
{
    [ \$# -ne 1 -o ! -d "\$1" ] && show_help && exit 1
    ! check_top "\$1" && show_help && exit 1
    ! check_md5 && echo -e "\\n\\033[31mBad Patch\\033[0m" && exit 1
    return 0
}

patch()
{
    if [ "\${SDK}" = "tina" -a -f "\${cur}/bin.tar.gz" ]; then
        ! tar -zmxf \${cur}/bin.tar.gz -C \${OBJ_ROOT} target \\
        && echo -e "\\033[31mPatch Failed\\033[0m" \\
        && exit 1
        echo -e "\\033[32mPatch Bin for Tina SDK ... OK\\033[0m"
    fi

    echo -e "\\033[32mPatch ... OK\\033[0m"
    return 0
}

# update
update()
{
    cur=\$(dirname \$0)
    opts=\$(getopt -o 'hp' -- \$@) || return 1
    eval set -- "\${opts}"
    while true
    do
        case "\$1" in
            -h)
                show_help
                exit 0
                ;;
            -p)
                actor="patch"
                shift
                ;;
            --)
                shift
                break
        esac
    done

    check \$@
    if [ -z "\${actor}" ]; then
        actor="patch"
    fi

    eval "\$(sed 's/ /\\n/g' <<< \${actor})"
    echo -e "\\033[32m--- END ---\\033[0m"
}

update \$@
EOF
}

mk_bin()
{
    # bin
    mkdir -p ${tmpdir}/target/allwinner/${target}-common/bin
    cp -r ${ROOT}/target/allwinner/${target}-common/bin/boot0_*.bin \
        ${tmpdir}/target/allwinner/${target}-common/bin

    # pack
    cd ${tmpdir}
    echo -e "\033[33mPatch List\033[0m"
    tar -zcvf bin.tar.gz target
}

do_patch()
{
    base="${up_platform}_Update_DRAM_To_$(awk '{print $1}' <<< ${dramversion})_$(date +%Y-%m-%d)"
    tmpdir="${outdir}/${base}"
    outfile="${base}.tar.gz"

    rm -rf ${tmpdir}
    rm -f ${outdir}/${outfile}
    mkdir -p ${tmpdir}

    mk_README && mk_install && mk_bin && mk_md5sum
    [ "$?" -ne 0 ] && exit 1

    echo ""
    rm -rf target
    cd ${tmpdir}/..
    tar -zcvf ${outdir}/${outfile} ${base} > /dev/null
    ! [ -f "${outdir}/${outfile}" ] \
        && echo -e "\033[32mMake Patch Failed\033[0m" \
        && exit 1

    echo -e "\033[32mMake Patch Finish\033[0m"
    echo -e "Patch at:\n\033[33m  ${outdir}/${outfile}\033[0m"
}

check_top()
{
    ROOT="$(sed 's#/$##' <<< $1)"

    [ ! -f "${ROOT}/build/envsetup.sh" ] \
        && echo -e "\033[31mInvalid Root of Tina SDK\033[0m\n" \
        && return 1
    return 0
}

check()
{
    [ -z "${platform}" ] \
        && echo -e "\033[31mPlease Tell Me The Platform by -p <platform>\033[0m" \
        && return 1

    [ $# -ne 1 -o ! -d "$1" ] \
        && echo -e "\033[31mMiss Root Of SDK\033[0m" \
        && show_help && return 1

    ! check_top "$1" && return 1

    ! check_platform \
        && echo -e "\033[31mInvalid Platform: ${platform}\033[0m" \
        && return 1

    echo ""
    return 0
}

show_version()
{
    echo "VERSION for this script: ${VERSION}"
}

mkpatch()
{
    opts=$(getopt -o 'ho:p:v' -- $@) || return 1
    eval set -- "${opts}"
    while true
    do
        case "$1" in
            -h)
                show_help
                exit 0
                ;;
            -p)
                shift
                platform="$1"
                shift
                ;;
            -o)
                shift
                outdir="$(cd $1 && pwd)"
                shift
                ;;
            -v)
                shift
                do_and_exit="${do_and_exit} show_version"
                ;;
            --)
                shift
                break
        esac
    done
    [ -z "${outdir}" -o "${outdir}" = '.' ] && outdir="${PWD}"

    check $@ || exit 1

    [ -n "${do_and_exit}" ] && {
        $(sed 's/ /\n/g' <<< ${do_and_exit})
        exit 0
    }

    do_patch

    echo -e "\033[32m--- END ---\033[0m"
}

mkpatch $@
