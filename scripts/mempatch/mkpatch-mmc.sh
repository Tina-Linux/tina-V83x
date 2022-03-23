#!/bin/bash

VERSION="0.2.1"
source $(dirname $0)/target.tb

show_help()
{
    echo -ne "Usage: mkpatch-mmc.sh -p <platform> [-o <outdir>] [-bh] "
    echo -e "\033[32m<Root of TinaSDK>\033[0m"
    echo -e "Options:"
    echo -e "\t-h: show this help message and exit"
    echo -e "\t-o <outdir>: out directory [default current dir]"
    echo -e "\t-p <platform>: the platform to make"
    echo -e "\t-v: just show driver version and exit"
    echo -e ""
    echo "Eg. ./mkpatch-mmc.sh -p r16 ~/worksapce/tina : make mmc patch for r16"
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
}

get_chip()
{
    local fconfig
    fconfig="${ROOT}/target/allwinner/$1-*/config-$(awk -F'-' '{print $2}' <<< $2)"
    chip="$(awk -F'[_=]' '/CONFIG_ARCH_SUN.*P.*=y/{print tolower($3)}' ${fconfig} | head -n 1)"
}

check_platform()
{
    up_platform="$(sed 's/[a-z]/\u&/g' <<< "${platform}")"
    info="$(sed 's/ /\n/g' <<< "${table[@]}" | awk "/${up_platform}/{print}")"
    [ -z "${info}" ] && return 1

    platform="$(sed 's/[A-Z]/\l&/g' <<< ${platform})"
    target="$(awk -F: '{print $1}' <<< ${info})"
    get_kernel_version ${target}
    get_uboot_version ${kernel}
    get_chip ${target} ${kernel}

    # kernel
    kdriver="lichee/${kernel}/drivers/mmc"
    case "${kernel}" in
        *-3.4)
            kversion="$(grep '#define *DRIVER_RIVISION.*' ${ROOT}/${kdriver}/host/sunxi-mci.h \
                | awk -F'[ "]' '{print $4 " " $5 " " $6}')"
            ;;
        *)
            kversion="$(grep '#define *DRIVER_RIVISION.*' ${ROOT}/${kdriver}/host/sunxi-mmc.h \
                | awk -F'[ "]' '{print $4 " " $5 " " $6}')"
            ;;
    esac

    # uboot
    case ${uboot} in
        *2011*)
            uroot="lichee/brandy/${uboot}"
            ubase="${uroot}/drivers/mmc/mmc_def.h"
            ;;
        *2014*)
            uroot="lichee/brandy/${uboot}"
            ubase="${uroot}/drivers/mmc/mmc_def.h"
            ;;
        *2018*)
            uroot="lichee/brandy-2.0/${uboot}"
            ubase="${uroot}/drivers/mmc/sunxi_mmc.c"
            ;;
        *)
            echo "invalid uboot: ${uboot}"
            exit 1
            ;;
    esac
    udriver="${uroot}/drivers/mmc"
    uversion="$(grep '#define *DRIVER_VER.*' ${ROOT}/${ubase} \
        | awk -F'[ "]' '{print $5 " " $6}')"

    echo "===================================================================="
    echo "platform: ${platform}"
    echo "kernel: ${kernel}"
    echo "uboot: ${uboot}"
    echo "chip: ${chip}"
    echo "target: ${target}"
    echo "kernel driver: ${kdriver}"
    echo "kernel driver version: ${kversion}"
    echo "uboot driver: ${udriver}"
    echo "uboot driver version: ${uversion}"
    echo "===================================================================="
    [ -z "${platform}" -o -z "${kernel}" -o -z "${uboot}" -o -z "${chip}" -o -z "${target}" \
        -o -z "${kversion}" -o -z "${uversion}" ] && return 1 || return 0
}

mk_README()
{
cat > ${tmpdir}/README.txt << EOF
注意：
=============================================================
此补丁只适用于更新全志 TinaSDK 或 AndroidSDK 的 mmc 驱动
=============================================================

补丁信息：
=============================================================
mmc驱动: ${kversion}
适用方案: ${platform} (${kernel} + ${uboot})
补丁提取日期: $(date +%Y-%m-%d\ %H:%M:%S)
=============================================================

文件内容：
=============================================================
patch.tar.gz    : mmc 补丁
bin.tar.gz      : Tina mmc bin 补丁
README.txt      : 说明文档
md5sum.txt      : 补丁包的md5码
install.sh      : 快速更新脚本
=============================================================

更新步骤：
=============================================================
1. 安装补丁
   ./install.sh <SDK根目录>
       Android SDK: 打补丁及重新编译uboot
       Tina SDK: 打补丁及替换uboot/boot0的bin
   Eg. install.sh ~/workspace/tina : 打补丁+替换uboot/boot0的bin
   Eg. install.sh ~/workspace/android : 打补丁+编译uboot

   Note.
   Android SDK根目录 指全志AndroidSDK中，包含lichee与Android源码
   的目录。目录结构如下：
       android/ (SDK根目录)
       ├── android
       └── lichee

2. 对TinaSDK，如果有定制uboot，需要手动编译uboot
   ./install -u <SDK根目录>
       Android SDK: 步骤1，默认编译uboot，不用执行步骤2
       Tina SDK: 重新编译uboot
   Eg. install.sh -u ~/workspace/tina : 重新编译Tina的uboot

3. 重新编译&打包
   若编译失败，表示补丁更新失败，请提供编译log以进一步分析
=============================================================
EOF
}

mk_md5sum()
{
    ! [ -f "${tmpdir}/patch.tar.gz" ] \
        && echo -e "\033[31mNot Found ${tmpdir}/patch.tar.gz\033[0m" \
        && return 1
    cd ${tmpdir} >/dev/null
    md5sum bin.tar.gz >> ${tmpdir}/md5sum.txt
    md5sum patch.tar.gz > ${tmpdir}/md5sum.txt
}

mk_install()
{
    touch ${tmpdir}/install.sh
    chmod +x ${tmpdir}/install.sh

cat > ${tmpdir}/install.sh << EOF
#!/bin/bash

show_help()
{
    echo -e "Usage: install.sh [-ahpu] \\033[32m<Root of TinaSDK or AndroidSDK>\\033[0m"
    echo -e "Options:"
    echo -e "\\t-a: patch and build uboot [default]"
    echo -e "\\t-h: show this help message and exit"
    echo -e "\\t-p: just patch"
    echo -e "\\t-u: just build uboot"
    echo -e ""
    echo "Eg. ./install.sh ~/worksapce/tina : just patch"
    echo "Eg. ./install.sh -u ~/worksapce/android: just build uboot"
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

    [ ! -f \${cur}/patch.tar.gz ] \\
        && echo -e "\\033[31mNot Found Tina Patch In Current Path\\033[0m" \\
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

uboot()
{
    echo -e "\\033[33mCheck Env\\033[0m"
    export TARGET_PLATFORM="${target}"
    chip="${chip}"
    echo -e "\\033[32mCheck Env ... OK\\033[0m"

    # build uboot
    echo -e "\\n\\033[33mBuild Uboot\\033[0m"
    cd \${OBJ_ROOT}/${uroot}
    grep "HAVE_BLOCK_DEVICE" include/configs/\${chip}.h &>/dev/null \\
        || sed -i '\$i #define HAVE_BLOCK_DEVICE' include/configs/\${chip}.h
    make distclean
    make \${chip}_config
    make -j4

    # check build
    [ "\$?" -ne 0 ] \\
        && echo -e "\\n\\033[31mBuild Uboot Failed\\033[0m" \\
        && exit 1

    echo -e "\\033[32mBuild Uboot ... OK\\033[0m"
    return 0
}

patch()
{
    ! tar -zmxf \${cur}/patch.tar.gz -C \${OBJ_ROOT} \\
        && echo -e "\\033[31mPatch Failed\\033[0m" \\
        && exit 1

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
    opts=\$(getopt -o 'ahpu' -- \$@) || return 1
    eval set -- "\${opts}"
    while true
    do
        case "\$1" in
            -a)
                actor="patch uboot"
                shift
                ;;
            -h)
                show_help
                exit 0
                ;;
            -p)
                actor="patch"
                shift
                ;;
            -u)
                actor="uboot"
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
        [ "\${SDK}" = "android" ] && actor="\${actor} uboot"
    fi

    eval "\$(sed 's/ /\\n/g' <<< \${actor})"
    echo -e "\\033[32m--- END ---\\033[0m"
}

update \$@
EOF
}

mk_patch()
{
    [ ! -d "${ROOT}/${kdriver}" -o ! -d "${ROOT}/${udriver}" ] \
        && echo -e "\033[31mMiss Driver Codes\033[0m" && return 1

    # clean all
    echo -ne "\033[32mCleaning kernel ... \033[0m"
    make -C ${ROOT}/lichee/${kernel} M=drivers/mmc clean &>/dev/null
    echo -e "\033[32mOK\033[0m"
    echo -ne "\033[32mCleaning uboot ... \033[0m"
    make -C ${ROOT}/lichee/brandy/${uboot} M=drivers/mmc clean &>/dev/null
    echo -e "\033[32mOK\033[0m"
    echo

    # kernel
    mkdir -p ${tmpdir}/${kdriver%/*}
    cp -r ${ROOT}/${kdriver} ${tmpdir}/${kdriver}
    mkdir -p ${tmpdir}/lichee/${kernel}/include/linux
    cp -r ${ROOT}/lichee/${kernel}/include/linux/mmc ${tmpdir}/lichee/${kernel}/include/linux/mmc

    # uboot
    mkdir -p ${tmpdir}/${udriver%/*}
    cp -r ${ROOT}/${udriver} ${tmpdir}/${udriver}
    mkdir -p ${tmpdir}/${uroot}/disk
    cp -r ${ROOT}/${uroot}/disk/* ${tmpdir}/${uroot}/disk
    mkdir -p ${tmpdir}/${uroot}/include
    cp -r ${ROOT}/${uroot}/include/mmc.h ${tmpdir}/${uroot}/include
    cp -r ${ROOT}/${uroot}/include/part.h ${tmpdir}/${uroot}/include
    # fix for uboot-2018: drivers/sunxi_flash/mmc
    ubase="${uroot}/drivers/sunxi_flash/mmc"
    if [ -d "${ROOT}/${ubase}" ]; then
        mkdir -p ${tmpdir}/${ubase%/*}
        cp -r ${ROOT}/${ubase} ${tmpdir}/${ubase}
    fi


    # bin
    mkdir -p ${tmpdir}/target/allwinner/${target}-common/bin
    cp -r ${ROOT}/target/allwinner/${target}-common/bin/{boot0_sdcard_sun*.bin,u-boot-sun*.bin} \
        ${tmpdir}/target/allwinner/${target}-common/bin

    # pack
    cd ${tmpdir}
    echo -e "\033[33mPatch List\033[0m"
    tar -zcvf bin.tar.gz target
    tar -zcvf patch.tar.gz lichee
}

do_patch()
{
    base="${up_platform}_Update_MMC_To_$(awk '{print $1}' <<< ${kversion})_$(date +%Y-%m-%d)"
    tmpdir="${outdir}/${base}"
    outfile="${base}.tar.gz"

    rm -rf ${tmpdir}
    rm -f ${outdir}/${outfile}
    mkdir -p ${tmpdir}

    mk_README && mk_install && mk_patch && mk_md5sum
    [ "$?" -ne 0 ] && exit 1

    echo ""
    rm -rf lichee target
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
