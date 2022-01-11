#!/bin/bash

#
#build.sh for uboot/spl
#wangwei@allwinnertech
#
set -e

TOP_DIR=$(cd `dirname $0`;pwd;cd - >/dev/null 2>&1)
BRANDY_SPL_DIR=$TOP_DIR/spl

show_help()
{
	printf "\nbuild.sh - Top level build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -t install gcc tools chain"
	echo "  -o build,e.g. uboot,spl,clean"
	echo "  -p <platform> platform, e.g. sun8iw18p1,  sun5i, sun6i, sun8iw1p1, sun8iw3p1, sun9iw1p1"
	echo "  -m mode,e.g. nand,mmc,nor"
    echo "example:"
    echo "./build.sh -o uboot -p sun8iw18p1"
    echo "./build.sh -o spl -p sun8iw18p1 -m nand"
	printf "\n\n"
}

prepare_toolchain()
{
        local ARCH="arm";
        local GCC="";
        local GCC_PREFIX="";
        local toolchain_archive_arm="./tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi.tar.xz";
        local tooldir_arm="./tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi";

        echo "Prepare toolchain ..."

        if [ ! -d "${tooldir_arm}" ]; then
                mkdir -p ${tooldir_arm} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_arm} -C ${tooldir_arm} || exit 1
        fi
}

function build_clean(){
    cd $TOP_DIR/spl
    make distclean
    cd - >/dev/null 2>&1
    cd $TOP_DIR/u-boot-2018
    make distclean
    cd - >/dev/null 2>&1
}

build_uboot_once()
{
    local defconfig=$1
    if [ "x${defconfig}" = "x" ];then
        echo "please set defconfig"
        exit 1
    fi
    echo build for ${defconfig} ...
	cd u-boot-2018/

	make distclean
	make ${defconfig}
	make -j16

	cd - 1>/dev/null
}

function build_uboot(){
    if [ "x${PLATFORM}" = "xall" ];then
        for defconfig in `ls ${TOP_DIR}/u-boot-2018/configs`;do
            if [[ $defconfig =~ .*_defconfig$ ]];then
                build_uboot_once $defconfig
            fi
        done
    else
        build_uboot_once ${PLATFORM}_defconfig
        if [ -e ${TOP_DIR}/u-boot-2018/configs/${PLATFORM}_nor_defconfig ]; then
            build_uboot_once ${PLATFORM}_nor_defconfig
        fi
    fi
}

function build_spl_once(){
    platform=$1
    mode=$2
    echo --------build for platform:${platform} mode:${mode}-------------------
    cd spl
    make distclean
    make p=${platform} m=${mode}
    case ${mode} in
        nand | mmc | spinor)
            make boot0
            ;;
        sboot_nor)
            echo "Neednot build sboot_nor ..."
            ;;
        *)
            make ${mode}
            ;;
    esac
    cd - > /dev/null 2>&1
}

function build_spl(){
    if [ "x${PLATFORM}" = "xall" ];then
        for platform in `ls $TOP_DIR/spl/board`;do
            if [ "x${MODE}" = "xall" ];then
                for mode in `ls ${TOP_DIR}/spl/board/${platform}`;do
                    if [[ $mode =~ .*\.mk$ ]]  \
                        && [ "x$mode" != "xcommon.mk" ];then
                        mode=${mode%%.mk*}
                        build_spl_once ${platform} ${mode}
                    fi
                done
            else
                build_spl_once $platform ${MODE}
            fi
        done
    elif [ "x${MODE}" = "xall" ];then
        for mode in `ls ${TOP_DIR}/spl/board/${PLATFORM}`;do
            if [[ $mode =~ .*\.mk$ ]]  \
                && [ "x$mode" != "xcommon.mk" ];then
                mode=${mode%%.mk*}
                build_spl_once ${PLATFORM} ${mode}
            fi
        done
    else
        build_spl_once ${PLATFORM} ${MODE}
    fi
}

function build_all(){
    build_uboot

    if [ -d ${BRANDY_SPL_DIR} ] ; then
        build_spl
	fi
}

while getopts to:p:m: OPTION
do
	case $OPTION in
	t)
		prepare_toolchain
		exit $?
		;;
    o)
        prepare_toolchain
		if [ "x$OPTARG" == "xboot0" ]; then
			command="build_spl"
		else
			command="build_$OPTARG"
		fi
        ;;

	p)
		PLATFORM=$OPTARG
		;;
	m)
		MODE=$OPTARG
        ;;
	*)
        show_help
		exit $?
		;;
    esac
done

if [ "x${PLATFORM}" = "x" ];then
    PLATFORM=all
fi
if [ "x${MODE}" = "x" ];then
    MODE=all
fi
if [ "x$command" != "x" ];then
    $command
else
    build_all
fi
exit $?

