#!/bin/bash
#
# pack/pack
# (c) Copyright 2013 - 2016 Allwinner
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# James Deng <csjamesdeng@allwinnertech.com>
# Trace Wong <wangyaliang@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

############################ Notice #####################################
# a. Some config files priority is as follows:
#    - xxx_${platform}.{cfg|fex} > xxx.{cfg|fex}
#    - ${chip}/${board}/*.{cfg|fex} > ${chip}/default/*.{cfg|fex}
#    - ${chip}/default/*.cfg > common/imagecfg/*.cfg
#    - ${chip}/default/*.fex > common/partition/*.fex
#  e.g. sun8iw7p1/configs/perf/image_linux.cfg > sun8iw7p1/configs/default/image_linux.cfg
#       > common/imagecfg/image_linux.cfg > sun8iw7p1/configs/perf/image.cfg
#       > sun8iw7p1/configs/default/image.cfg > common/imagecfg/image.cfg
#
# b. Support Nor storages rule:
#    - Need to create sys_partition_nor.fex or sys_partition_nor_${platform}.fex
#    - Add "{filename = "full_img.fex",     maintype = "12345678", \
#      subtype = "FULLIMG_00000000",}" to image[_${platform}].cfg
#
# c. Switch uart port
#    - Need to add your chip configs into ${LONGAN_COMMON_DIR}/debug/card_debug_pin
#    - Call pack with 'debug' parameters

enable_pause=0

function get_char()
{
	SAVEDSTTY=`stty -g`
	stty -echo
	stty cbreak
	dd if=/dev/tty bs=1 count=1 2> /dev/null
	stty -raw
	stty echo
	stty $SAVEDSTTY
}

function pause()
{
	if [ "x$1" != "x" ] ;then
		echo $1
	fi
	if [ $enable_pause -eq 1 ] ; then
		echo "Press any key to continue!"
		char=`get_char`
	fi
}

function pack_error()
{
	echo -e "\033[47;31mERROR: $*\033[0m"
}

function pack_warn()
{
	echo -e "\033[47;34mWARN: $*\033[0m"
}

function pack_info()
{
	echo -e "\033[47;30mINFO: $*\033[0m"
}

source scripts/shflags

# define option, format:
#   'long option' 'default value' 'help message' 'short option'
DEFINE_string 'chip' '' 'chip to build, e.g. sun7i' 'c'
DEFINE_string 'platform' '' 'platform to build, e.g. linux, android, camdroid' 'p'
DEFINE_string 'board' '' 'board to build, e.g. evb' 'b'
DEFINE_string 'kernel' '' 'kernel to build, e.g. linux-3.4, linux-3.10' 'k'
DEFINE_string 'debug_mode' 'uart0' 'config debug mode, e.g. uart0, card0' 'd'
DEFINE_string 'signture' 'none' 'pack boot signture to do secure boot' 's'
DEFINE_string 'secure' 'none' 'pack secure boot with -v arg' 'v'
DEFINE_string 'mode' 'normal' 'pack dump firmware' 'm'
DEFINE_string 'function' 'android' 'pack private firmware' 'f'
DEFINE_string 'topdir' 'none' 'sdk top dir' 't'
DEFINE_string 'programmer' '' 'creat programmer img or not' 'w'
DEFINE_string 'tar_image' '' 'creat downloadfile img .tar.gz or not' 'i'

# parse the command-line
FLAGS "$@" || exit $?
eval set -- "${FLAGS_ARGV}"

PACK_CHIP=${FLAGS_chip}
PACK_PLATFORM=${FLAGS_platform}
PACK_BOARD=${FLAGS_board}
PACK_KERN=${FLAGS_kernel}
PACK_DEBUG=${FLAGS_debug_mode}
PACK_SIG=${FLAGS_signture}
PACK_SECURE=${FLAGS_secure}
PACK_MODE=${FLAGS_mode}
PACK_FUNC=${FLAGS_function}
PACK_PROGRAMMER=${FLAGS_programmer}
PACK_TAR_IMAGE=${FLAGS_tar_image}
PACK_TOPDIR=${FLAGS_topdir}
MULTI_CONFIG_INDEX=0

SUFFIX=""

ROOT_DIR=${PACK_TOPDIR}/out/${PACK_BOARD}
OTA_TEST_NAME="ota_test"
export PATH=${PACK_TOPDIR}/out/host/bin/:$PATH

if [ "x${PACK_CHIP}" = "xsun5i" ]; then
	PACK_BOARD_PLATFORM=unclear
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw5p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw6p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw7p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw8p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw11p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw10p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw15p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw17p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw18p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw19p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun3iw1p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun50iw1p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm64
elif [ "x${PACK_CHIP}" = "xsun50iw3p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm64
elif [ "x${PACK_CHIP}" = "xsun8iw12p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun8iw16p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm
elif [ "x${PACK_CHIP}" = "xsun50iw6p1" -o "x${PACK_CHIP}" = "xsun50iw9p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm64
elif [ "x${PACK_CHIP}" = "xsun50iw10p1" ]; then
	PACK_BOARD_PLATFORM=${PACK_BOARD%%-*}
	ARCH=arm64
else
	echo "board_platform($PACK_CHIP) not support"
fi

TINA_CONFIG_DIR="${PACK_TOPDIR}/target/allwinner"
LONGAN_CONFIG_DIR="${PACK_TOPDIR}/device/config/chips/${PACK_BOARD_PLATFORM}/configs"
LONGAN_COMMON_DIR="${PACK_TOPDIR}/device/config/common"

tools_file_list=(
${TINA_CONFIG_DIR}/generic/tools/split_xxxx.fex
${TINA_CONFIG_DIR}/generic/tools/usbtool_test.fex
${TINA_CONFIG_DIR}/generic/tools/usbtool_crash.fex
${TINA_CONFIG_DIR}/generic/tools/cardscript.fex
${TINA_CONFIG_DIR}/generic/tools/cardscript_secure.fex
${TINA_CONFIG_DIR}/generic/tools/cardtool.fex
${TINA_CONFIG_DIR}/generic/tools/usbtool.fex
${TINA_CONFIG_DIR}/generic/tools/aultls32.fex
${TINA_CONFIG_DIR}/generic/tools/aultools.fex

${LONGAN_COMMON_DIR}/tools/split_xxxx.fex
${LONGAN_COMMON_DIR}/tools/usbtool_test.fex
${LONGAN_COMMON_DIR}/tools/usbtool_crash.fex
${LONGAN_COMMON_DIR}/tools/cardscript.fex
${LONGAN_COMMON_DIR}/tools/cardscript_secure.fex
${LONGAN_COMMON_DIR}/tools/cardtool.fex
${LONGAN_COMMON_DIR}/tools/usbtool.fex
${LONGAN_COMMON_DIR}/tools/aultls32.fex
${LONGAN_COMMON_DIR}/tools/aultools.fex

${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/split_xxxx.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/usbtool_test.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/usbtool_crash.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/cardscript.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/cardscript_secure.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/cardtool.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/usbtool.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/aultls32.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/aultools.fex

${LONGAN_CONFIG_DIR}/../tools/split_xxxx.fex
${LONGAN_CONFIG_DIR}/../tools/usbtool_test.fex
${LONGAN_CONFIG_DIR}/../tools/usbtool_crash.fex
${LONGAN_CONFIG_DIR}/../tools/cardscript.fex
${LONGAN_CONFIG_DIR}/../tools/cardscript_secure.fex
${LONGAN_CONFIG_DIR}/../tools/cardtool.fex
${LONGAN_CONFIG_DIR}/../tools/usbtool.fex
${LONGAN_CONFIG_DIR}/../tools/aultls32.fex
${LONGAN_CONFIG_DIR}/../tools/aultools.fex
)

configs_file_list=(
${TINA_CONFIG_DIR}/generic/toc/toc1.fex
${TINA_CONFIG_DIR}/generic/toc/toc0.fex
${TINA_CONFIG_DIR}/generic/toc/boot_package.fex
${TINA_CONFIG_DIR}/generic/dtb/sunxi.fex
${TINA_CONFIG_DIR}/generic/configs/*.fex
${TINA_CONFIG_DIR}/generic/configs/*.cfg
${TINA_CONFIG_DIR}/generic/version/version_base.mk

${LONGAN_COMMON_DIR}/toc/*.fex
${LONGAN_COMMON_DIR}/hdcp/esm.fex
${LONGAN_COMMON_DIR}/dtb/sunxi.fex
${LONGAN_COMMON_DIR}/imagecfg/*.cfg
${LONGAN_COMMON_DIR}/partition/sys_partition_dump.fex
${LONGAN_COMMON_DIR}/partition/sys_partition_private.fex
${LONGAN_COMMON_DIR}/version/version_base.mk

${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/configs/*.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/configs/*.cfg
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/configs/*.its
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/version/version_base.mk
${TINA_CONFIG_DIR}/${PACK_BOARD}/configs/*.fex
${TINA_CONFIG_DIR}/${PACK_BOARD}/configs/*.cfg
${TINA_CONFIG_DIR}/${PACK_BOARD}/version/version_base.mk

${LONGAN_CONFIG_DIR}/default/*.fex
${LONGAN_CONFIG_DIR}/default/*.cfg
${LONGAN_CONFIG_DIR}/default/*.its
${LONGAN_CONFIG_DIR}/default/version_base.mk
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/*.fex
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/*.cfg
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/linux/*.fex
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/linux/*.cfg
)

boot_resource_list=(
${TINA_CONFIG_DIR}/generic/boot-resource/boot-resource:image/
${TINA_CONFIG_DIR}/generic/boot-resource/boot-resource.ini:image/
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/boot-resource:image/
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/boot-resource.ini:image/
${TINA_CONFIG_DIR}/${PACK_BOARD}/configs/*.bmp:image/boot-resource/

${LONGAN_CONFIG_DIR}/default/boot-resource:image/
${LONGAN_CONFIG_DIR}/default/boot-resource.ini:image/
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/configs/*.bmp:image/boot-resource/
)

possible_bin_path=(
configs/${PACK_BOARD}/${PACK_PLATFORM}/${LICHEE_BUSSINESS}/bin
configs/${PACK_BOARD}/${PACK_PLATFORM}/bin
configs/${PACK_BOARD}/${LICHEE_BUSSINESS}/bin
configs/${PACK_BOARD}/bin
${LICHEE_BUSSINESS}/bin
bin)

BIN_PATH=""
for d in ${possible_bin_path[@]}; do
	[ -d ${LICHEE_CHIP_CONFIG_DIR}/$d ] && BIN_PATH=$d && break
done

[ -z "${BIN_PATH}" ] &&
pack_error "No BIN_PATH found!" && exit 1

pack_info "Use BIN_PATH: ${LONGAN_CONFIG_DIR}/../$BIN_PATH"

boot_file_list=(
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_nand_${PACK_CHIP}.bin:image/boot0_nand.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_sdcard_${PACK_CHIP}.bin:image/boot0_sdcard.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_spinor_${PACK_CHIP}.bin:image/boot0_spinor.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/fes1_${PACK_CHIP}.bin:image/fes1.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-${PACK_CHIP}.bin:image/u-boot.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-crashdump-${PACK_CHIP}.bin:image/u-boot-crash.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-crashdump-spinor-${PACK_CHIP}.bin:image/u-boot-spinor-crash.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/bl31.bin:image/monitor.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/scp.bin:image/scp.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/optee_${PACK_CHIP}.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-spinor-${PACK_CHIP}.bin:image/u-boot-spinor.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_nand_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/boot0_nand-${OTA_TEST_NAME}.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_sdcard_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/boot0_sdcard-${OTA_TEST_NAME}.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/boot0_spinor_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/boot0_spinor-${OTA_TEST_NAME}.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/u-boot-${OTA_TEST_NAME}.fex
${LONGAN_CONFIG_DIR}/../${BIN_PATH}/u-boot-spinor-${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/u-boot-spinor-${OTA_TEST_NAME}.fex
)

boot_file_secure=(
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/semelis_${PACK_CHIP}.bin:image/semelis.bin
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/optee_${PACK_CHIP}.bin:image/optee.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/optee_${PACK_CHIP}-secure.bin:image/optee.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/u-boot-${PACK_CHIP}-secure.bin:image/u-boot.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin

${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/semelis_${PACK_CHIP}.bin:image/semelis.bin
${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/optee_${PACK_CHIP}.bin:image/optee.fex
${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/optee_${PACK_CHIP}-secure.bin:image/optee.fex
${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/u-boot-${PACK_CHIP}-secure.bin:image/u-boot.fex
${TINA_CONFIG_DIR}/${PACK_BOARD}/bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin

${LONGAN_CONFIG_DIR}/../bin/semelis_${PACK_CHIP}.bin:image/semelis.bin
${LONGAN_CONFIG_DIR}/../bin/optee_${PACK_CHIP}.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/../bin/optee_${PACK_CHIP}-secure.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/../bin/u-boot-${PACK_CHIP}-secure.bin:image/u-boot.fex
${LONGAN_CONFIG_DIR}/../bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
${LONGAN_CONFIG_DIR}/../bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin

${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/semelis_${PACK_CHIP}.bin:image/semelis.bin
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/optee_${PACK_CHIP}.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/optee_${PACK_CHIP}-secure.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/u-boot-${PACK_CHIP}-secure.bin:image/u-boot.fex
${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin

)

a64_boot_file_secure=(
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/optee_${PACK_CHIP}.bin:image/optee.fex
${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
# ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin

${LONGAN_CONFIG_DIR}/../bin/optee_${PACK_CHIP}.bin:image/optee.fex
${LONGAN_CONFIG_DIR}/../bin/sboot_${PACK_CHIP}.bin:image/sboot.bin
# ${LONGAN_CONFIG_DIR}/../bin/sboot_${PACK_CHIP}-${OTA_TEST_NAME}.bin:image/sboot-${OTA_TEST_NAME}.bin
)

function get_partition_downfile_size()
{
	local downloadfile_name=`echo $1 | awk -F '=' '{print $2}'`
	if [ ! -f ${downloadfile_name} ]; then
		echo "  file ${downloadfile_name} not find"
	else
		if [ -L ${downloadfile_name} ]; then
			local downloadfile_name_link=`readlink -f ${downloadfile_name}`
			local linkfile_name=${downloadfile_name_link##*/}
			echo "  ${downloadfile_name} -> ${downloadfile_name_link}"
			if [ ! -f ${downloadfile_name_link} ]; then
				echo "  link file ${linkfile_name} not find"
			else
				local linkfile_size=`ls -lh ${downloadfile_name_link} | awk '{print $5}'`
				echo "  ${linkfile_name} size : ${linkfile_size} byte"
			fi
		else
			local downloadfile_size=`ls -lh ${downloadfile_name} | awk '{print $5}'`
			echo "  ${downloadfile_name} size : ${downloadfile_size} byte"
		fi
	fi
}

function get_partition_mbr_size()
{
	local partition_size_name=`echo $1 | awk -F '=' '{print $1}' | sed 's/partition/mbr/g'`
	local partition_size=`echo $1 | awk -F '=' '{print $2}'`
	echo "  ${partition_size_name}  : ${partition_size} Kbyte"
}

function show_partition_message()
{
	grep -c '[mbr]' $1 > /dev/null
	if [ $? -eq 0 ]; then
		cp $1 ./show_sys_partition.tmp;
		sed -i '/^[\r;]/d' ./show_sys_partition.tmp;
		sed -i '/partition_start/d' ./show_sys_partition.tmp;
		sed -i '/user_type/d' ./show_sys_partition.tmp;
		sed -i 's/\[partition\]/------------------------------------/g' ./show_sys_partition.tmp;
		sed -i 's/[ "\r]//g' ./show_sys_partition.tmp;
		sed -i '/^[;]/d' ./show_sys_partition.tmp;
		sed -i 's/name/partition_name/g' ./show_sys_partition.tmp;
		sed -i 's/size/partition_size/g' ./show_sys_partition.tmp;
		echo ------------------------------------
		while read line
		do
			if [ "$line" == "------------------------------------" ];then
				echo $line
			else
				echo "  $line" | sed 's/=/  : /g'
				echo "  $line" | grep "mbr" >> /dev/null
				if [ $? -eq 0 ]; then
					read line
					get_partition_mbr_size $line
				fi
				echo $line | grep "downloadfile" >> /dev/null
				if [ $? -eq 0 ]; then
					get_partition_downfile_size $line
				fi
			fi
		done < ./show_sys_partition.tmp
		echo ------------------------------------
		rm ./show_sys_partition.tmp
	else
		echo "==========input is not a partition file=========="
	fi
}

function show_boards()
{
	printf "\nAll avaiable chips, platforms and boards:\n\n"
	printf "Chip            Board\n"
	for chipdir in $(find chips/ -mindepth 1 -maxdepth 1 -type d) ; do
		chip=`basename ${chipdir}`
		printf "${chip}\n"
		for boarddir in $(find chips/${chip}/configs/${platform} \
			-mindepth 1 -maxdepth 1 -type d) ; do
			board=`basename ${boarddir}`
			printf "                ${board}\n"
		done
	done
	printf "\nFor Usage:\n"
	printf "     $(basename $0) -h\n\n"
}

function do_platform_pack_hook()
{
	PWD=$(pwd)
	if [ -e ${TINA_CONFIG_DIR}/${PACK_BOARD}/hooks/pack_${1}_hook.sh ] ; then
		pack_info "--Call platform pack ${1} hook now--"
		cd ${TINA_CONFIG_DIR}/${PACK_BOARD}/hooks/
		./pack_${1}_hook.sh
		cd ${PWD}
	fi
}

function call_platform_pack_hook()
{
	hook_type=$1
	do_platform_pack_hook $hook_type
}

function call_platform_pack_prepare_hook()
{
	call_platform_pack_hook prepare
}

function uart_switch()
{
	local DEBUG_DIR="${LONGAN_COMMON_DIR}/debug"

	rm -rf ${ROOT_DIR}/image/awk_debug_card0
	touch ${ROOT_DIR}/image/awk_debug_card0
	TX=`awk  '$0~"'$PACK_CHIP'"{print $2}' ${DEBUG_DIR}/card_debug_pin`
	RX=`awk  '$0~"'$PACK_CHIP'"{print $3}' ${DEBUG_DIR}/card_debug_pin`
	PORT=`awk  '$0~"'$PACK_CHIP'"{print $4}' ${DEBUG_DIR}/card_debug_pin`
	MS=`awk  '$0~"'$PACK_CHIP'"{print $5}' ${DEBUG_DIR}/card_debug_pin`
	CK=`awk  '$0~"'$PACK_CHIP'"{print $6}' ${DEBUG_DIR}/card_debug_pin`
	DO=`awk  '$0~"'$PACK_CHIP'"{print $7}' ${DEBUG_DIR}/card_debug_pin`
	DI=`awk  '$0~"'$PACK_CHIP'"{print $8}' ${DEBUG_DIR}/card_debug_pin`

	BOOT_UART_ST=`awk  '$0~"'$PACK_CHIP'"{print $2}' ${DEBUG_DIR}/card_debug_string`
	BOOT_PORT_ST=`awk  '$0~"'$PACK_CHIP'"{print $3}' ${DEBUG_DIR}/card_debug_string`
	BOOT_TX_ST=`awk  '$0~"'$PACK_CHIP'"{print $4}' ${DEBUG_DIR}/card_debug_string`
	BOOT_RX_ST=`awk  '$0~"'$PACK_CHIP'"{print $5}' ${DEBUG_DIR}/card_debug_string`
	UART0_ST=`awk  '$0~"'$PACK_CHIP'"{print $6}' ${DEBUG_DIR}/card_debug_string`
	UART0_USED_ST=`awk  '$0~"'$PACK_CHIP'"{print $7}' ${DEBUG_DIR}/card_debug_string`
	UART0_PORT_ST=`awk  '$0~"'$PACK_CHIP'"{print $8}' ${DEBUG_DIR}/card_debug_string`
	UART0_TX_ST=`awk  '$0~"'$PACK_CHIP'"{print $9}' ${DEBUG_DIR}/card_debug_string`
	UART0_RX_ST=`awk  '$0~"'$PACK_CHIP'"{print $10}' ${DEBUG_DIR}/card_debug_string`
	UART1_ST=`awk  '$0~"'$PACK_CHIP'"{print $11}' ${DEBUG_DIR}/card_debug_string`
	JTAG_ST=`awk  '$0~"'$PACK_CHIP'"{print $12}' ${DEBUG_DIR}/card_debug_string`
	MS_ST=`awk  '$0~"'$PACK_CHIP'"{print $13}' ${DEBUG_DIR}/card_debug_string`
	CK_ST=`awk  '$0~"'$PACK_CHIP'"{print $14}' ${DEBUG_DIR}/card_debug_string`
	DO_ST=`awk  '$0~"'$PACK_CHIP'"{print $15}' ${DEBUG_DIR}/card_debug_string`
	DI_ST=`awk  '$0~"'$PACK_CHIP'"{print $16}' ${DEBUG_DIR}/card_debug_string`
	MMC0_ST=`awk  '$0~"'$PACK_CHIP'"{print $17}' ${DEBUG_DIR}/card_debug_string`
	MMC0_USED_ST=`awk  '$0~"'$PACK_CHIP'"{print $18}' ${DEBUG_DIR}/card_debug_string`

	echo '$0!~";" && $0~"'$BOOT_TX_ST'"{if(C)$0="'$BOOT_TX_ST' = '$TX'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$BOOT_RX_ST'"{if(C)$0="'$BOOT_RX_ST' = '$RX'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$BOOT_PORT_ST'"{if(C)$0="'$BOOT_PORT_ST' = '$PORT'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	if [ "`grep "auto_print_used" "${ROOT_DIR}/image//sys_config${SUFFIX}.fex" | grep "1"`" ]; then
		echo '$0!~";" && $0~"'$MMC0_USED_ST'"{if(A)$0="'$MMC0_USED_ST' = 1";A=0} \' >> ${ROOT_DIR}/image/awk_debug_card0
	else
		echo '$0!~";" && $0~"'$MMC0_USED_ST'"{if(A)$0="'$MMC0_USED_ST' = 0";A=0} \' >> ${ROOT_DIR}/image/awk_debug_card0
	fi
	echo '$0!~";" && $0~"\\['$MMC0_ST'\\]"{A=1}  \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$UART0_TX_ST'"{if(B)$0="'$UART0_TX_ST' = '$TX'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$UART0_RX_ST'"{if(B)$0="'$UART0_RX_ST' = '$RX'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"\\['$UART0_ST'\\]"{B=1} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$UART0_USED_ST'"{if(B)$0="'$UART0_USED_ST' = 1"}  \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '/^'$UART0_PORT_ST'/{next} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"\\['$UART1_ST'\\]"{B=0} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"\\['$BOOT_UART_ST'\\]"{C=1} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"\\['$JTAG_ST'\\]"{C=0} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$MS_ST'"{$0="'$MS_ST' = '$MS'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$CK_ST'"{$0="'$CK_ST' = '$CK'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$DO_ST'"{$0="'$DO_ST' = '$DO'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '$0!~";" && $0~"'$DI_ST'"{$0="'$DI_ST' = '$DI'"} \' >> ${ROOT_DIR}/image/awk_debug_card0
	echo '1' >> ${ROOT_DIR}/image/awk_debug_card0

	if [ "`grep "auto_print_used" "${ROOT_DIR}/image/sys_config${SUFFIX}.fex" | grep "1"`" ]; then
		sed -i -e '/^uart0_rx/a\pinctrl-1=' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		sed -i -e '/^uart0_rx/a\pinctrl-0=' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	fi
	awk -f ${ROOT_DIR}/image/awk_debug_card0 ${ROOT_DIR}/image/sys_config${SUFFIX}.fex > ${ROOT_DIR}/image/sys_config${SUFFIX}_debug.fex
	rm -f ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	mv ${ROOT_DIR}/image/sys_config${SUFFIX}_debug.fex ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	echo "uart -> card0"
}

function copy_ota_test_file()
{
	printf "ota test bootloader by diff bootlogo\n"
	mv ${ROOT_DIR}/image/boot-resource/bootlogo_ota_test.bmp ${ROOT_DIR}/image/boot-resource/bootlogo.bmp

	printf "copying ota test boot file\n"
	if [ -f ${ROOT_DIR}/image/sys_partition_nor.fex -o \
	-f ${ROOT_DIR}/image/sys_partition_nor_${PACK_PLATFORM}.fex ];  then
		mv ${ROOT_DIR}/image/boot0_spinor-${OTA_TEST_NAME}.fex	${ROOT_DIR}/image/boot0_spinor.fex
		mv ${ROOT_DIR}/image/u-boot-spinor-${OTA_TEST_NAME}.fex	${ROOT_DIR}/image/u-boot-spinor.fex
	else
		mv ${ROOT_DIR}/image/boot0_nand-${OTA_TEST_NAME}.fex		${ROOT_DIR}/image/boot0_nand.fex
		mv ${ROOT_DIR}/image/boot0_sdcard-${OTA_TEST_NAME}.fex	${ROOT_DIR}/image/boot0_sdcard.fex
		mv ${ROOT_DIR}/image/u-boot-${OTA_TEST_NAME}.fex		${ROOT_DIR}/image/u-boot.fex
	fi

	if [ "x${PACK_SECURE}" = "xsecure" -o  "x${PACK_SIG}" = "prev_refurbish"] ; then
		printf "Copying ota test secure boot file\n"
		mv ${ROOT_DIR}/image/sboot-${OTA_TEST_NAME}.bin ${ROOT_DIR}/image/sboot.bin
	fi

	printf "OTA test env by bootdelay(10) and logolevel(8)\n"
	sed -i 's/\(logolevel=\).*/\18/' ${ROOT_DIR}/image/env.cfg
	sed -i 's/\(bootdelay=\).*/\110/' ${ROOT_DIR}/image/env.cfg
}


function update_mbr_to_sys_config()
{
	cd ${ROOT_DIR}/image
	#use sys_partition_tmp
	cp $1 sys_partition_tmp.fex

	#convert sys_partition.fex to sunxi_mbr.fex
	#don't care about downloadfile
	sed -i '/^[ \t]*downloadfile/d' sys_partition_tmp.fex
	/bin/busybox unix2dos sys_partition_tmp.fex
	script  sys_partition_tmp.fex > /dev/null
	update_mbr sys_partition_tmp.bin 1 sunxi_mbr_tmp.fex

	#get size and offset
	local PART_INDEX
	local PART_NAME
	local PART_SIZE
	local PART_OFFSET
	echo "" >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	echo [partitions] >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	echo "" >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex

	# add one partition to sys_config
	# PART_NAME="vital"
	# PART_SIZE=`parser_mbr sunxi_mbr_tmp.fex get_size_by_name ${PART_NAME}`
	# PART_OFFSET=`parser_mbr sunxi_mbr_tmp.fex get_offset_by_name ${PART_NAME}`
	# #update to sys_config
	# echo [partitions/vital] >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	# echo offset = ${PART_OFFSET} >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	# echo size = ${PART_SIZE} >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex

	# add all partitions to sys_config
	total_num=`parser_mbr sunxi_mbr_tmp.fex get_total_num`
	let total_num--
	for PART_INDEX in $( /usr/bin/seq 0 ${total_num} )
	do
		PART_NAME=`parser_mbr sunxi_mbr_tmp.fex get_name_by_index ${PART_INDEX}`
		PART_SIZE=`parser_mbr sunxi_mbr_tmp.fex get_size_by_index ${PART_INDEX}`
		PART_OFFSET=`parser_mbr sunxi_mbr_tmp.fex get_offset_by_index ${PART_INDEX}`
		echo [partitions/${PART_NAME}] >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		echo offset = ${PART_OFFSET} >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		echo size = ${PART_SIZE} >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		echo "" >> ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	done

	#clean
	rm -f sys_partition_tmp.fex sys_partition_tmp.bin sunxi_mbr_tmp.fex dlinfo.fex
	cd -
}

function sparse_ext4()
{
	local img=$1
	local sparse_img=$2

	if file $img | grep -q ext4 ;then
		echo "now make sparse ext4 img: $img"
	else
		echo "$img is not ext4 img"
		file $img
		return
	fi

	img2simg $img $sparse_img
}

# pack user resources to a vfat filesystem
# To use this, please add a folder "user-resource" in configs to save files, and add a partition to sys_partition.fex/sys_partition_nor.fex like this:
# [partition]
#	name         = user-res
#	size         = 1024
#	downloadfile = "user-resource.fex"
#	user_type    = 0x8000
function make_user_res()
{
	printf "make user resource for : $1\n"
	local USER_RES_SYS_PARTITION=$1
	local USER_RES_PART_NAME=user-res
	local USER_RES_FILE=user-resource
	printf "handle partition ${USER_RES_PART_NAME}\n"
	local USER_RES_PART_DOWNLOAD_FILE=`awk 'BEGIN {FS="\n"; RS=""} /'${USER_RES_PART_NAME}'/{print $0}' ${USER_RES_SYS_PARTITION} | awk '{if($1 == "downloadfile"){print $3}}' | sed 's/"//g'`
	local USER_RES_PART_SIZE=`awk 'BEGIN {FS="\n"; RS=""} /'${USER_RES_PART_NAME}'/{print $0}' $1 | awk '$0~"size"{print $3/2}'`
	local USER_RES_FILE_PATH=${TINA_CONFIG_DIR}/${PACK_BOARD}/configs/${USER_RES_FILE}
	if [ x${USER_RES_PART_DOWNLOAD_FILE} != x'' -a  x${USER_RES_PART_SIZE} != x'' ];then
		rm -f ${ROOT_DIR}/image/user-resource.fex
		mkfs.vfat ${ROOT_DIR}/image/user-resource.fex -C ${USER_RES_PART_SIZE}
		if [ -d ${USER_RES_FILE_PATH} ];then
			USER_RES_FILE_SIZE=`du --summarize "${USER_RES_FILE_PATH}" | awk '{print $1}'`
			printf "file size: ${USER_RES_FILE_SIZE}\n"
			printf "partition size: ${USER_RES_PART_SIZE}\n"
			if [ ${USER_RES_PART_SIZE} -le ${USER_RES_FILE_SIZE} ]; then
				printf "file size is larger than partition size, please check your configuration\n"
				printf "please enlarge size of ${USER_RES_PART_NAME} in sys_partition or remove some files in $USER_RES_FILE_PATH\n"
				exit -1
			fi
			mcopy -s -v -i ${ROOT_DIR}/image/${USER_RES_PART_DOWNLOAD_FILE} ${USER_RES_FILE_PATH}/* ::
			if [ $? -ne 0 ]; then
				printf "mcopy file fail, exit\n"
				exit -1
			fi
		else
			printf "can not find ${USER_RES_FILE_PATH}, ignore it\n"
		fi
	else
		printf "no user resource partitions\n"
	fi
}

function make_app_res()
{
	cd ${ROOT_DIR}/image
	local APP_PART_NAME=app
	cp $1 sys_partition_tmp_app.fex

	sed -i '/^[ \t]*downloadfile/d' sys_partition_tmp_app.fex
	/bin/busybox unix2dos sys_partition_tmp_app.fex
	script  sys_partition_tmp_app.fex > /dev/null
	update_mbr sys_partition_tmp_app.bin 1 sunxi_mbr_tmp_app.fex

	local APP_PART_DOWNLOAD_FILE=app.fex
	local APP_PART_FILE_PATH=${PACK_TOPDIR}/out/${TARGET_BOARD}/compile_dir/target/app
	local APP_PART_SIZE_IN_SECTOR=`parser_mbr sunxi_mbr_tmp_app.fex get_size_by_name ${APP_PART_NAME}`

	if [ x${APP_PART_DOWNLOAD_FILE} != x'' -a  x${APP_PART_SIZE_IN_SECTOR} != x'' ]; then
		let APP_PART_SIZE_IN_K=$APP_PART_SIZE_IN_SECTOR/2
		echo "APP_PART_DOWNLOAD_FILE = ${ROOT_DIR}/image/${APP_PART_DOWNLOAD_FILE}"
		rm -f ${ROOT_DIR}/image/${APP_PART_DOWNLOAD_FILE}
		${PACK_TOPDIR}/out/host/bin/make_ext4fs -l ${APP_PART_SIZE_IN_K}k -b 1024 -m 0 -j 1024 ${ROOT_DIR}/image/${APP_PART_DOWNLOAD_FILE}  ${APP_PART_FILE_PATH}
	else
		printf "no app resource partitions\n"
	fi
	cd -
}

function make_data_res()
{
	cd ${ROOT_DIR}/image
	local DATA_PART_NAME=data
	cp $1 sys_partition_tmp_data.fex

	sed -i '/^[ \t]*downloadfile/d' sys_partition_tmp_data.fex
	/bin/busybox unix2dos sys_partition_tmp_data.fex
	script  sys_partition_tmp_data.fex > /dev/null
	update_mbr sys_partition_tmp_data.bin 1 sunxi_mbr_tmp_data.fex

	local DATA_PART_DOWNLOAD_FILE=data.fex
	local DATA_PART_DOWNLOAD_FILE_SPARSE=data_s.fex
	local DATA_PART_FILE_PATH=${PACK_TOPDIR}/out/${TARGET_BOARD}/compile_dir/target/data
	local DATA_PART_SIZE_IN_SECTOR=`parser_mbr sunxi_mbr_tmp_data.fex get_size_by_name ${DATA_PART_NAME}`

	if [ x${DATA_PART_DOWNLOAD_FILE} != x'' -a  x${DATA_PART_SIZE_IN_SECTOR} != x'0' ]; then
		let DATA_PART_SIZE_IN_K=$DATA_PART_SIZE_IN_SECTOR/2
		echo "DATA_PART_DOWNLOAD_FILE = ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE}"
		rm -f ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE}
		rm -f ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE_SPARSE}
		${PACK_TOPDIR}/out/host/bin/make_ext4fs -l ${DATA_PART_SIZE_IN_K}k -b 1024 -m 0 -j 1024 ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE}  ${DATA_PART_FILE_PATH}
		sparse_ext4 ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE} ${ROOT_DIR}/image/${DATA_PART_DOWNLOAD_FILE_SPARSE}
	else
		printf "no data resource partitions\n"
	fi
	cd -
}

function make_boottone_fex()
{
	local BOOTTONE_FILE_PATH=${TINA_CONFIG_DIR}/${PACK_BOARD}/configs/boottone_res
	if [ -e $BOOTTONE_FILE_PATH ];then
		echo "make boottone.fex"
		${PACK_TOPDIR}/out/host/bin/make_ext4fs -l 512k -b 1024 -m 0 -J ${ROOT_DIR}/image/boottone.fex $BOOTTONE_FILE_PATH
	fi
}

function update_suffix()
{
	if [ $MULTI_CONFIG_INDEX -gt 0 ]; then
		SUFFIX="-$MULTI_CONFIG_INDEX"
	else
		SUFFIX=""
	fi
}

function fetch_multiconfig()
{
	MULTI_CONFIG_INDEX=0
	update_suffix
	while [ -e ${ROOT_DIR}/image/sys_config${SUFFIX}.fex ];do
		echo sys_config${SUFFIX} exist
		let "MULTI_CONFIG_INDEX=MULTI_CONFIG_INDEX+1"
		update_suffix
	done
	let "MULTI_CONFIG_INDEX=MULTI_CONFIG_INDEX-1"
	echo "Multiconfig num:"$MULTI_CONFIG_INDEX
}


ENV_SUFFIX=

function get_kernel
{
	local f="${TINA_CONFIG_DIR}/${PACK_BOARD}/Makefile"
	[ -f "$f" ] || return -1

	awk -F":=" '/KERNEL_PATCHVER/{print $2}' $f
}

function do_early_prepare()
{
	# Cleanup
	rm -rf ${ROOT_DIR}/image
	mkdir -p ${ROOT_DIR}/image
	do_prepare
	fetch_multiconfig
}

function do_prepare()
{
	if [ -z "${PACK_CHIP}" -o -z "${PACK_PLATFORM}" -o -z "${PACK_BOARD}" ] ; then
		pack_error "Invalid parameters Chip: ${PACK_CHIP}, \
			Platform: ${PACK_PLATFORM}, Board: ${PACK_BOARD}"
		show_boards
		exit 1
	fi

	[ -d ${TINA_CONFIG_DIR}/${PACK_BOARD}/configs -o -d ${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/linux ] || {
		pack_error "Board's directory \
			\"${TINA_CONFIG_DIR}/${PACK_BOARD}/configs or ${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/linux\" not exist."
		show_boards
		exit 1
	}
	#TODO:diff kernel version support
	if [ -z "${PACK_KERN}" ] ; then
		printf "No kernel param, parse it from ${PACK_BOARD_PLATFORM}\n"
		[ "x$(get_kernel)" != "x" ] && {
			PACK_KERN="linux-$(get_kernel)"
			ENV_SUFFIX=$(get_kernel)
		}
		if [ -z "${PACK_KERN}" ] ; then
			pack_error "Failed to parse kernel param from ${PACK_BOARD_PLATFORM}"
			exit 1
		fi
	fi

	call_platform_pack_prepare_hook

	printf "copying tools file\n"
	for file in ${tools_file_list[@]} ; do
		cp -f $file ${ROOT_DIR}/image/ 2> /dev/null
	done

	if [ "x${PACK_KERN}" = "xlinux-3.4" ]; then
		if [ -f ${LONGAN_COMMON_DIR}/tools/cardscript.fex ]; then
			cp -f ${LONGAN_COMMON_DIR}/tools/cardscript.fex ${ROOT_DIR}/image/ 2> /dev/null
		else
			cp -f ${TINA_CONFIG_DIR}/generic/tools/cardscript.fex ${ROOT_DIR}/image/ 2> /dev/null
		fi
	fi

	printf "copying configs file\n"
	for file in ${configs_file_list[@]} ; do
		cp -f $file ${ROOT_DIR}/image/ 2> /dev/null
	done

	if [ -f ${ROOT_DIR}/image/sys_config_${PACK_KERN}${SUFFIX}.fex ]; then
		cp ${ROOT_DIR}/image/sys_config_${PACK_KERN}${SUFFIX}.fex ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	fi

	# get storage_type value from sys_config.fex
	if [ ! -f ${ROOT_DIR}/image/sys_config${SUFFIX}.fex ];then
		echo "sys_config${SUFFIX}.fex is not exist."
		exit 1
	fi
	# If platform config files exist, we will cover the default files
	# For example, mv out/image_linux.cfg out/image.cfg
	cd ${ROOT_DIR}
	PACK_PLATFORM_IMAGE_CFG=$PACK_PLATFORM
	[ x"$PACK_PLATFORM" = x"tina" ] && PACK_PLATFORM_IMAGE_CFG="linux"
	find image/* -type f -a \( -name "*.fex" -o -name "*.cfg" \) -print | \
		sed "s#\(.*\)_${PACK_PLATFORM_IMAGE_CFG}\(\..*\)#mv -fv & \1\2#e"
	cd -

	local storage_type
	storage_type=`sed -e '/^$/d' -e '/^;/d' -e '/^\[/d' -n -e '/^storage_type/p' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex | sed 's/=/ /g' | awk '{ print $2;}'`
	echo "storage_type value is ${storage_type}"
	image_instruction="image is for nand/emmc"

	case ${storage_type} in
		3)
		echo "storage type is nor"
		if [ -f ${ROOT_DIR}/image/image_nor.cfg ];then
			echo "image_nor.cfg is exist"
			mv ${ROOT_DIR}/image/image_nor.cfg ${ROOT_DIR}/image/image.cfg && echo "mv image_nor.cfg image.cfg"
			image_instruction="image is for nor"
			IMG_FLASH="_nor"
		fi
		;;
		-1)
		;;
		*)
		if [ -f ${ROOT_DIR}/image/sys_partition_nor.fex ];then
			rm ${ROOT_DIR}/image/sys_partition_nor.fex && echo "rm ${ROOT_DIR}/image/sys_partition_nor.fex"
		fi
		if [ -f ${ROOT_DIR}/image/image_nor.cfg ];then
			rm ${ROOT_DIR}/image/image_nor.cfg && echo "rm ${ROOT_DIR}/image/image_nor.cfg"
		fi
		;;
	esac

	# amend env copy
	mv ${ROOT_DIR}/image/env-${ENV_SUFFIX}.cfg ${ROOT_DIR}/image/env.cfg 2> /dev/null

	if [ "x${PACK_MODE}" = "xdump" ] ; then
		cp -vf ${ROOT_DIR}/image/sys_partition_dump.fex ${ROOT_DIR}/image/sys_partition.fex
		cp -vf ${ROOT_DIR}/image/usbtool_test.fex ${ROOT_DIR}/image/usbtool.fex
	elif [ "x${PACK_FUNC}" = "xprvt" ] ; then
		cp -vf ${ROOT_DIR}/image/sys_partition_private.fex ${ROOT_DIR}/image/sys_partition.fex
	fi

	grep "CONFIG_USE_DM_VERITY=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		cp -vf ${ROOT_DIR}/image/sys_partition_secure.fex ${ROOT_DIR}/image/sys_partition.fex
	fi

	printf "copying boot resource\n"
	for file in ${boot_resource_list[@]} ; do
		cp -rf `echo $file | awk -F: '{print $1}'` \
			${ROOT_DIR}/`echo $file | awk -F: '{print $2}'` 2>/dev/null
	done
	lzma e ${ROOT_DIR}/image/boot-resource/bootlogo.bmp ${ROOT_DIR}/image/bootlogo.bmp.lzma
	printf "copying boot file\n"
	for file in ${boot_file_list[@]} ; do
		cp -f `echo $file | awk -F: '{print $1}'` \
			${ROOT_DIR}/`echo $file | awk -F: '{print $2}'` 2>/dev/null
	done

	if [ "x${ARCH}" != "xarm64" ] ; then
		if [ "x${PACK_SECURE}" = "xsecure" -o "x${PACK_SIG}" = "xsecure" -o  "x${PACK_SIG}" = "xprev_refurbish" ] ; then
			printf "copying secure boot file\n"
			for file in ${boot_file_secure[@]} ; do
				cp -f `echo $file | awk -F: '{print $1}'` \
					${ROOT_DIR}/`echo $file | awk -F: '{print $2}'`
			done
		fi
	else
		if [ "x${PACK_SECURE}" = "xsecure" -o "x${PACK_SIG}" = "xsecure" -o  "x${PACK_SIG}" = "xprev_refurbish" ] ; then
			printf "copying arm64 secure boot file\n"
			for file in ${a64_boot_file_secure[@]} ; do
				cp -f `echo $file | awk -F: '{print $1}'` \
					${ROOT_DIR}/`echo $file | awk -F: '{print $2}'`
			done
		fi
	fi

	# If longan platform config use
	if [ -f ${LONGAN_CONFIG_DIR}/../tools/plat_config.sh ] ; then
		${LONGAN_CONFIG_DIR}/../tools/plat_config.sh
	fi
	# If tina platform config use
	if [ -f ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/plat_config.sh ] ; then
		${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/tools/plat_config.sh
	fi


	if [ "x${PACK_SECURE}" = "xsecure"  -o "x${PACK_SIG}" = "xsecure" ] ; then
		printf "add burn_secure_mode in target in sys config\n"
		sed -i -e '/^\[target\]/a\burn_secure_mode=1' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		sed -i -e '/^\[platform\]/a\secure_without_OS=0' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	elif [ "x${PACK_SIG}" = "xprev_refurbish" ] ; then
		printf "add burn_secure_mode in target in sys config\n"
		sed -i -e '/^\[target\]/a\burn_secure_mode=1' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		sed -i -e '/^\[platform\]/a\secure_without_OS=1' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	else
		sed -i '/^burn_secure_mod/d' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		sed -i '/^secure_without_OS/d' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	fi

	if [ "x${PACK_MODE}" = "xota_test" ] ; then
		printf "copy ota test file\n"
		copy_ota_test_file
	fi

	if [ "x${PACK_PROGRAMMER}" = "xprogrammer" ]; then
		printf "add programmer img info target in sys config\n"
		sed -i -e '/^\[target\]/a\programmer=1' out/sys_config.fex
	fi

	# Here, we can switch uart to card or normal
	if [ "x${PACK_DEBUG}" = "xcard0" -a "x${PACK_MODE}" != "xdump" \
		-a "x${PACK_FUNC}" != "xprvt" ] ; then \
		uart_switch
	else
		sed -i -e '/^auto_print_used/s\1\0\' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	fi

	sed -i 's/\\boot-resource/\/boot-resource/g' ${ROOT_DIR}/image/boot-resource.ini
	sed -i 's/\\\\/\//g' ${ROOT_DIR}/image/image.cfg
	sed -i 's/^imagename/;imagename/g' ${ROOT_DIR}/image/image.cfg

	IMG_NAME="${PACK_PLATFORM}_${PACK_BOARD}_${PACK_DEBUG}"

	if [ "x${PACK_SIG}" != "xnone" ]; then
		IMG_NAME="${IMG_NAME}_${PACK_SIG}"
	fi

	if [ "x${PACK_MODE}" = "xdump" -o "x${PACK_MODE}" = "xota_test" -o "x${PACK_MODE}" = "xcrashdump" ] ; then
		IMG_NAME="${IMG_NAME}_${PACK_MODE}"
	fi

	if [ "x${PACK_FUNC}" = "xprvt" ] ; then
		IMG_NAME="${IMG_NAME}_${PACK_FUNC}"
	fi

	if [ "x${PACK_SECURE}" = "xsecure" ] ; then
		IMG_NAME="${IMG_NAME}_${PACK_SECURE}"
	fi

	if [ "x${PACK_FUNC}" = "xprev_refurbish" ] ; then
		IMG_NAME="${IMG_NAME}_${PACK_FUNC}"
	fi

	IMG_NAME="${IMG_NAME}${IMG_FLASH}"

	IMG_PROGRAMMER_NAME="${IMG_NAME}_programmer.img"

	if [ "x${PACK_SECURE}" != "xnone" -o "x${PACK_SIG}" != "xnone" ]; then
		MAIN_VERION=`awk  '$0~"MAIN_VERSION"{printf"%d", $3}' ${ROOT_DIR}/image/version_base.mk`

		IMG_NAME="${IMG_NAME}_v${MAIN_VERION}.img"
	else
		IMG_NAME="${IMG_NAME}.img"
	fi

	echo "imagename = $IMG_NAME" >> ${ROOT_DIR}/image/image.cfg
	echo "" >> ${ROOT_DIR}/image/image.cfg

	cp ${ROOT_DIR}/image/env.cfg  ${ROOT_DIR}/image/env_bk.cfg
	# boot time optimization:
	# 1.remove uboot uart log;
	# 2.do not check kernel image crc.
	# 3.remove kernel uart log.
	# 4.set rootfstype.
	grep "CONFIG_BOOT_TIME_OPTIMIZATION=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		sed -i "/debug_mode/d" ${ROOT_DIR}/image/sys_config${SUFFIX}.fex && sed -i '/^\[platform\]$/a\debug_mode\ \ =\ 0' ${ROOT_DIR}/image/sys_config${SUFFIX}.fex
		sed -i "/^verify=/d" ${ROOT_DIR}/image/env.cfg && sed -i '/^init=/a\verify=no' ${ROOT_DIR}/image/env.cfg
		sed -i "/^loglevel=/d" ${ROOT_DIR}/image/env.cfg && sed -i '/^init=/a\loglevel=0' ${ROOT_DIR}/image/env.cfg

		grep "CONFIG_TARGET_ROOTFS_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			rootfstype=squashfs
		fi

                grep "CONFIG_TARGET_ROOTFS_EXT4FS=y" ${PACK_TOPDIR}/.config > /dev/null
                if [ $? -eq 0 ]; then
                        rootfstype=ext4
                fi

		if [ "x${rootfstype}" != "x" ]; then
			sed -i "s/setargs_.*=.*/& rootfstype=${rootfstype}/" ${ROOT_DIR}/image/env.cfg
		fi
	fi

	# for busybox init, default use /pseudo_init as init process.
	grep "CONFIG_SYSTEM_INIT_BUSYBOX=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		sed -i "/^init=/d" ${ROOT_DIR}/image/env.cfg && sed -i '/^mmc_root=/a\init=\/pseudo_init' ${ROOT_DIR}/image/env.cfg
	fi

	# for small storage ota, no recovery partition, recovery system store in extend partition.
	grep "CONFIG_SUNXI_SMALL_STORAGE_OTA=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		sed -i '/^boot_recovery.*/s/ recovery/ extend/g' ${ROOT_DIR}/image/env.cfg
	fi

	# bootm from 40007800 for r328 boot.img/raw
	if [ "x${PACK_BOARD_PLATFORM}" = "xr328s2" -o "x${PACK_BOARD_PLATFORM}" = "xr328s3" ]; then
		grep "CONFIG_SUNXI_SD_BOOT_PART is not set" ${PACK_TOPDIR}/.config > /dev/null && \
			grep "CONFIG_SUNXI_SD_BOOT_KERNEL_FORMAT_BOOTIMG=y" ${PACK_TOPDIR}/.config > /dev/null

		if [ $? -eq 0 ]; then
			sed -i '/^boot_normal.*bootm/s/40007fc0/40007800/g' ${ROOT_DIR}/image/env.cfg
		fi

		grep "CONFIG_SUNXI_RECOVERY_INITRAMFS_PART is not set" ${PACK_TOPDIR}/.config > /dev/null && \
			grep "CONFIG_SUNXI_RECOVERY_INITRAMFS_KERNEL_FORMAT_BOOTIMG=y" ${PACK_TOPDIR}/.config > /dev/null

		if [ $? -eq 0 ]; then
			sed -i '/^boot_recovery.*bootm/s/40007fc0/40007800/g' ${ROOT_DIR}/image/env.cfg
		fi

	fi

	make_boottone_fex
	if [ -e ${ROOT_DIR}/image/sys_partition_nor.fex ];then
		make_user_res ${ROOT_DIR}/image/sys_partition_nor.fex
		make_app_res ${ROOT_DIR}/image/sys_partition_nor.fex
		make_data_res ${ROOT_DIR}/image/sys_partition_nor.fex
		update_mbr_to_sys_config ${ROOT_DIR}/image/sys_partition_nor.fex
	else
		make_user_res ${ROOT_DIR}/image/sys_partition.fex
		make_app_res ${ROOT_DIR}/image/sys_partition.fex
		make_data_res ${ROOT_DIR}/image/sys_partition.fex
		update_mbr_to_sys_config ${ROOT_DIR}/image/sys_partition.fex
	fi

}

function do_ini_to_dts()
{
	if [ "x${PACK_KERN}" == "xlinux-3.4" ]; then
		return
	fi
	if [ "x${PACK_KERN}" == "xlinux-4.4" -a "x${ARCH}" == "xarm64" ]; then
		local extra_path=sunxi
	elif [ "x${PACK_KERN}" == "xlinux-4.9" -a "x${ARCH}" == "xarm64" ]; then
		local extra_path=sunxi
	else
		local extra_path=
	fi

	local DTC_SRC_PATH=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/${extra_path}/
	local DTC_DEP_FILE=${DTC_SRC_PATH}/.${PACK_CHIP}-${PACK_BOARD}${SUFFIX}.dtb.d.dtc.tmp
	local DTC_SRC_FILE=${DTC_SRC_PATH}/.${PACK_CHIP}-${PACK_BOARD}${SUFFIX}.dtb.dts.tmp

	local DTC_DEP_BOARD1=${DTC_SRC_PATH}/.board${SUFFIX}.dtb.d.dtc.tmp
	local DTC_SRC_BOARD1=${DTC_SRC_PATH}/.board${SUFFIX}.dtb.dts.tmp
	if [ -f ${DTC_DEP_BOARD1} -a -f ${DTC_SRC_BOARD1} ]; then
		DTC_DEP_FILE=${DTC_DEP_BOARD1}
		DTC_SRC_FILE=${DTC_SRC_BOARD1}
	fi
	local DTC_COMPILER=${PACK_TOPDIR}/lichee/$PACK_KERN/scripts/dtc/dtc
	local DTC_INI_FILE_BASE=${ROOT_DIR}/image/sys_config${SUFFIX}.fex
	local DTC_INI_FILE=${ROOT_DIR}/image/sys_config${SUFFIX}_fix.fex
	local DTC_FLAGS=""

	cp $DTC_INI_FILE_BASE $DTC_INI_FILE
	sed -i "s/\(\[dram\)_para\(\]\)/\1\2/g" $DTC_INI_FILE
	sed -i "s/\(\[nand[0-9]\)_para\(\]\)/\1\2/g" $DTC_INI_FILE

	if [ ! -f $DTC_COMPILER ]; then
		pack_error "Script_to_dts: Can not find dtc compiler.\n"
		exit 1
	fi
	if [ ! -f $DTC_DEP_FILE ]; then
		printf "Script_to_dts: Can not find [%s-%s.dts]. Will use common dts file instead.\n" ${PACK_CHIP} ${PACK_BOARD}
		if [ "x${PACK_BOARD_PLATFORM}" = "xr6" ] ; then
			DTC_DEP_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-r6-soc${SUFFIX}.dtb.d.dtc.tmp
			DTC_SRC_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-r6-soc${SUFFIX}.dtb.dts.tmp
		elif [ "x${PACK_BOARD_PLATFORM}" = "xc200s" -o "x${PACK_BOARD_PLATFORM}" = "xv133" -o "x${PACK_BOARD_PLATFORM}" = "xc600"] ; then
				DTC_DEP_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-${PACK_BOARD}${SUFFIX}.dtb.d.dtc.tmp
				DTC_SRC_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-${PACK_BOARD}${SUFFIX}.dtb.dts.tmp
		else
			if [ "x${PACK_KERN}" == "xlinux-4.4" -a "x${ARCH}" == "xarm64" ]; then
				DTC_DEP_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/sunxi/.${PACK_CHIP}-soc${SUFFIX}.dtb.d.dtc.tmp
				DTC_SRC_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/sunxi/.${PACK_CHIP}-soc${SUFFIX}.dtb.dts.tmp
			elif [ "x${PACK_KERN}" == "xlinux-4.9" -a "x${ARCH}" == "xarm64" ]; then
				DTC_DEP_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/sunxi/.${PACK_CHIP}-soc${SUFFIX}.dtb.d.dtc.tmp
				DTC_SRC_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/sunxi/.${PACK_CHIP}-soc${SUFFIX}.dtb.dts.tmp
			else
				DTC_DEP_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-soc${SUFFIX}.dtb.d.dtc.tmp
				DTC_SRC_FILE=${PACK_TOPDIR}/lichee/$PACK_KERN/arch/$ARCH/boot/dts/.${PACK_CHIP}-soc${SUFFIX}.dtb.dts.tmp
			fi
		fi
		#Disbale noisy checks
		if [ "x${PACK_KERN}" == "xlinux-4.9" ]; then
			DTC_FLAGS="-W no-unit_address_vs_reg"
		fi
	fi
	$DTC_COMPILER ${DTC_FLAGS} -O dtb -o ${ROOT_DIR}/image/sunxi${SUFFIX}.dtb	\
		-b 0			\
		-i $DTC_SRC_PATH	\
		-F $DTC_INI_FILE	\
		-d $DTC_DEP_FILE $DTC_SRC_FILE

	if [ $? -ne 0 ]; then
		pack_error "Conver script to dts failed"
		exit 1
	fi

	printf "Conver script to dts ok.\n"

	# It'is used for debug dtb
	$DTC_COMPILER  ${DTC_FLAGS} -I dtb -O dts -o ${ROOT_DIR}/image/.sunxi${SUFFIX}.dts ${ROOT_DIR}/image/sunxi${SUFFIX}.dtb

	return
}

function do_common()
{
	cd ${ROOT_DIR}/image

	if [ ! -f board_config.fex ]; then
		echo "[empty]" > board_config.fex
	fi

	busybox unix2dos sys_config${SUFFIX}.fex
	busybox unix2dos board_config.fex
	script  sys_config${SUFFIX}.fex > /dev/null
	cp -f   sys_config${SUFFIX}.bin config${SUFFIX}.fex
	script  board_config.fex > /dev/null
	cp -f board_config.bin board.fex

	busybox unix2dos sys_partition.fex
	script  sys_partition.fex > /dev/null

	# Those files for SpiNor. We will try to find sys_partition_nor.fex
	if [ -f sys_partition_nor.fex ];  then

		if [ -f "sunxi${SUFFIX}.dtb" ]; then
			cp sunxi${SUFFIX}.dtb sunxi${SUFFIX}.fex
		fi

		if [ -f "scp.fex" ]; then
			echo "update scp"
			update_scp scp.fex sunxi${SUFFIX}.fex >/dev/null
		fi
		# Here, will create sys_partition_nor.bin
		busybox unix2dos sys_partition_nor.fex
		script  sys_partition_nor.fex > /dev/null
		update_boot0 boot0_spinor.fex   sys_config${SUFFIX}.bin SDMMC_CARD > /dev/null
		if [ "x${PACK_KERN}" = "xlinux-3.4" ] ; then
			update_uboot -merge u-boot-spinor.fex  sys_config${SUFFIX}.bin > /dev/null
		else
			update_uboot -no_merge u-boot-spinor.fex  sys_config${SUFFIX}.bin > /dev/null
		fi

		if [ -f boot_package_nor.cfg -a	x${SUFFIX} == x'' ]; then
			mv u-boot-spinor-crash.fex u-boot-crash.fex
			echo "pack boot package"
			busybox unix2dos boot_package.cfg
			dragonsecboot -pack boot_package_nor.cfg
			cp boot_package.fex boot_package_nor.fex
		fi
		# Ugly, but I don't have a better way to change it.
		# We just set env's downloadfile name to env_nor.cfg in sys_partition_nor.fex
		# And if env_nor.cfg is not exist, we should copy one.
		if [ ! -f env_nor.cfg ]; then
			cp -f env.cfg env_nor.cfg >/dev/null 2<&1
		fi

		# Fixup boot mode for SPINor, just can bootm
		sed -i '/^boot_normal/s#\<boota\>#bootm#g' env_nor.cfg

		grep "CONFIG_SUNXI_MAKE_REDUNDANT_ENV=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			local env_size=0x0
			local env_size=`grep "CONFIG_SUNXI_REDUNDANT_ENV_SIZE" ${PACK_TOPDIR}/.config | awk -F '=' '{print $2}' | sed 's/\"//g'`
			echo "--mkenvimage create redundant env data!--"
			echo "---redundant env data size ${env_size}---"
			mkenvimage -r -p 0x00 -s ${env_size} -o env_nor.fex env_nor.cfg
		else
			u_boot_env_gen env_nor.cfg env_nor.fex >/dev/null
		fi
	fi

	if [ ! -f "u-boot-crash.fex" ]; then
		touch "u-boot-crash.fex"
		echo "ensure u-boot-crash.fex is not empty" > u-boot-crash.fex
	fi

	if [ -f "sunxi${SUFFIX}.dtb" ]; then
		cp sunxi${SUFFIX}.dtb sunxi${SUFFIX}.fex
		update_dtb sunxi${SUFFIX}.fex 4096
	fi

	if [ -f "scp.fex" ]; then
		echo "update scp"
		update_scp scp.fex sunxi${SUFFIX}.fex >/dev/null
	fi
	# Those files for Nand or Card
	update_boot0 boot0_nand.fex	sys_config${SUFFIX}.bin NAND > /dev/null
	update_boot0 boot0_sdcard.fex	sys_config${SUFFIX}.bin SDMMC_CARD > /dev/null
	if [ "x${PACK_KERN}" = "xlinux-3.4" ] ; then
		update_uboot -merge u-boot.fex sys_config${SUFFIX}.bin > /dev/null
	else
		update_uboot -no_merge u-boot.fex sys_config${SUFFIX}.bin > /dev/null
	fi
	update_fes1  fes1.fex           sys_config${SUFFIX}.bin > /dev/null
	fsbuild	     boot-resource.ini  split_xxxx.fex > /dev/null

	if [ -f boot_package.cfg  -a x${SUFFIX} == x'' ]; then
			echo "pack boot package"
			busybox unix2dos boot_package.cfg
			dragonsecboot -pack boot_package.cfg
			if [ $? -ne 0 ]
			then
				pack_error "dragon pack run error"
				exit 1
			fi

			update_toc1  boot_package.fex           sys_config${SUFFIX}.bin
			if [ $? -ne 0 ]
			then
				pack_error "update toc1 run error"
				exit 1
			fi
	fi

	if [ "x${PACK_FUNC}" = "xprvt" ] ; then
		grep "CONFIG_SUNXI_MAKE_REDUNDANT_ENV=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			local env_size=0x0
			local env_size=`grep "CONFIG_SUNXI_REDUNDANT_ENV_SIZE" ${PACK_TOPDIR}/.config | awk -F '=' '{print $2}' | sed 's/\"//g'`
			echo "--mkenvimage create redundant env data!--"
			echo "---redundant env data size ${env_size}---"
			mkenvimage -r -p 0x00 -s ${env_size} -o env.fex env_burn.cfg
		else
			u_boot_env_gen env_burn.cfg env.fex > /dev/null
		fi
	else
		grep "CONFIG_SUNXI_MAKE_REDUNDANT_ENV=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			local env_size=0x0
			local env_size=`grep "CONFIG_SUNXI_REDUNDANT_ENV_SIZE" ${PACK_TOPDIR}/.config | awk -F '=' '{print $2}' | sed 's/\"//g'`
			echo "--mkenvimage create redundant env data!--"
			echo "---redundant env data size ${env_size}---"
			mkenvimage -r -p 0x00 -s ${env_size} -o env.fex env.cfg
		else
			u_boot_env_gen env.cfg env.fex > /dev/null
		fi
	fi

	if [ -f "arisc" ]; then
		ln -s arisc arisc.fex
	fi
}

function img_to_programmer()
{
	local out_img=$1
	local in_img=$2

	if [ "x${PACK_SIG}" = "xprev_refurbish" -o "x${PACK_SIG}" = "xsecure" ]; then
		programmer_img toc0.fex toc1.fex ${out_img} > /dev/null
	else
		programmer_img boot0_sdcard.fex boot_package.fex ${out_img} > /dev/null
	fi
	#create_img toc0.fex toc1.fex
	programmer_img sys_partition.bin sunxi_mbr.fex ${out_img} ${in_img} > /dev/null
}


function do_finish()
{
	# Yeah, it should contain all files into full_img.fex for spinor
	# Because, as usually, spinor image size is very small.
	# If fail to create full_img.fex, we should fake it empty.

	# WTF, it is so ugly!!! It must be sunxi_mbr.fex & sys_partition.bin,
	# not sunxi_mbr_xxx.fex & sys_partition_xxx.bin. In order to advoid this
	# loathsome thing, we need to backup & copy files. Check whether
	# sys_partition_nor.bin is exist, and create sunxi_mbr.fex for Nor.
	if [ -f sys_partition_nor.bin ]; then
		update_mbr sys_partition_nor.bin 1 sunxi_mbr_nor.fex
		if [ $? -ne 0 ]; then
			pack_error "update_mbr failed"
			exit 1
		fi
		#only uboot2011&linux-3.4 bsp used full img
		if [ "x${PACK_KERN}" = "xlinux-3.4" ] ; then
			BOOT1_FILE=u-boot-spinor.fex
			#Need to set LOGIC_START and UBOOT_START in .hooks/pre-pack script
			#if not set , will use default
			if [ -z "$LOGIC_START" ] ; then
				LOGIC_START=496 #496+16=512K
			fi
			if [ -z "$UBOOT_START" ] ; then
				UBOOT_START=24 #24K
			fi
			#if use uboot-2018, it not use mbr, but gpt
			if [ "x${TARGET_UBOOT}" = "xu-boot-2018" ] ; then
				mbr_file=sunxi_gpt.fex
			else
				mbr_file=sunxi_mbr_nor.fex
			fi

			merge_full_img --out full_img.fex \
				--boot0 boot0_spinor.fex \
				--boot1 ${BOOT1_FILE} \
				--mbr ${mbr_file} \
				--logic_start ${LOGIC_START} \
				--uboot_start ${UBOOT_START} \
				--partition sys_partition_nor.bin
			if [ $? -ne 0 ]; then
				pack_error "merge_full_img failed"
				exit 1
			fi
			rm -f sys_partition_for_dragon.fex
		else
			#the img for Phoenixsuit/Livesuit do not need full_img
			#but we need to make one for flash programmer
			BOOT1_FILE=boot_package_nor.fex
			if [ -z "$LOGIC_START" ] ; then
				LOGIC_START=496 #496+16=512K
			fi
			if [ -z "$UBOOT_START" ] ; then
				UBOOT_START=24 #24K
			fi

			#if use uboot-2018, it not use mbr, but gpt
			if [ "x${TARGET_UBOOT}" = "xu-boot-2018" ] ; then
				mbr_file=sunxi_gpt.fex
			else
				mbr_file=sunxi_mbr_nor.fex
			fi
			merge_full_img --out full_img.fex \
		              --boot0 boot0_spinor.fex \
		              --boot1 ${BOOT1_FILE} \
			      --mbr ${mbr_file} \
			      --logic_start ${LOGIC_START} \
			      --uboot_start ${UBOOT_START} \
			      --partition sys_partition_nor.bin
			if [ $? -ne 0 ]; then
				pack_error "merge_full_img failed"
				exit 1
			fi
			cp sys_partition_nor.fex sys_partition_for_dragon.fex
		fi
		cp sys_partition_nor.fex sys_partition.fex

	else
		if [ "x${PACK_KERN}" = "xlinux-3.4" -a ! -f full_img.fex ] ; then
			echo "full_img.fex is empty" > full_img.fex
		fi
		update_mbr sys_partition.bin 4
		if [ $? -ne 0 ]; then
			pack_error "update_mbr failed"
			exit 1
		fi
		cp sys_partition.fex sys_partition_for_dragon.fex
	fi

	if [ -f sys_partition_for_dragon.fex ]; then
		do_dragon image.cfg sys_partition_for_dragon.fex
	else
		do_dragon image.cfg
	fi

	if [ "x${PACK_PROGRAMMER}" = "xprogrammer" ]; then
		echo "waiting to ceate programmer img..."
		img_to_programmer ${IMG_PROGRAMMER_NAME} ../${IMG_NAME}
		if [ $? -eq 0 ]; then
			if [ -e ${IMG_PROGRAMMER_NAME} ]; then
				mv ${IMG_PROGRAMMER_NAME} ../${IMG_PROGRAMMER_NAME}
				echo '----------programmer image is at----------'
				echo -e '\033[0;31;1m'
				echo ${ROOT_DIR}/${IMG_PROGRAMMER_NAME}
				echo -e '\033[0m'
			fi
		fi
	fi

	cd ..
	printf "pack finish\n"
}

function do_dragon()
{
	local partition_file_name="x$2"
	if [ $partition_file_name != "x" ]; then
		echo ====================================
		echo show \"$2\" message
		show_partition_message $2
	fi
	dragon $@
	if [ $? -eq 0 ]; then
		if [ -e ${IMG_NAME} ]; then
			mv ${IMG_NAME} ../${IMG_NAME}
			echo "----------${image_instruction}----------"
			echo '----------image is at----------'
			echo -e '\033[0;31;1m'
			echo ${ROOT_DIR}/${IMG_NAME}
			echo -e '\033[0m'
			if [ $PACK_HOOK ]
			then
				$PACK_HOOK ${ROOT_DIR}/${IMG_NAME}
			fi
		fi
	fi
	# you can add scripts/.hooks/post-dragon to do something after dragon
	# for example, you can copy img to another dir, add post-dragon like this:
	#
	# echo "==========post-dragon========"
	# cp ${ROOT_DIR}/${IMG_NAME} ~/myimgs/
	# echo "==========post-dragon done========"
	#
	[ -e ${PACK_TOPDIR}/scripts/.hooks/post-dragon ] &&
		source ${PACK_TOPDIR}/scripts/.hooks/post-dragon
}

function do_signature()
{
	# merge flag: '1' - merge atf/scp/uboot/optee in one package, '0' - do not merge.
	local merge_flag=0

	printf "prepare for signature by openssl\n"
	if [ "x${PACK_SIG}" = "xprev_refurbish" ] ; then
		if [ "x${ARCH}" = "xarm64" ] ; then
			if [ -f ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_a64_no_secureos.cfg ]; then
				cp -v ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_a64_no_secureos.cfg dragon_toc.cfg
			else
				cp -v ${TINA_CONFIG_DIR}/generic/sign_config/dragon_toc_a64_no_secureos.cfg dragon_toc.cfg
			fi
		else
			if [ -f ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_no_secureos.cfg ]; then
				cp -v ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_no_secureos.cfg dragon_toc.cfg
			else
				cp -v ${TINA_CONFIG_DIR}/generic/sign_config/dragon_toc_no_secureos.cfg dragon_toc.cfg
			fi
		fi
	else
		if [ "x${ARCH}" = "xarm64" ] ; then
			if [ -f ${LONGAN_CONFIG_DIR}/default/dragon_toc_a64_package.cfg ] ; then
				cp -v ${LONGAN_CONFIG_DIR}/default/dragon_toc_a64_package.cfg dragon_toc.cfg
				merge_flag=1
			elif [ -f ${LONGAN_CONFIG_DIR}/default/dragon_toc_a64.cfg ] ; then
				cp -v ${LONGAN_CONFIG_DIR}/default/dragon_toc_a64.cfg dragon_toc.cfg
			elif [ -f ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc_a64_package.cfg ] ; then
				cp -v ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc_a64_package.cfg dragon_toc.cfg
				merge_flag=1
			elif [ -f ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc_a64.cfg ] ; then
				cp -v ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc_a64.cfg dragon_toc.cfg
			elif [ -f ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_a64.cfg dragon_toc.cfg ] ; then
				cp -v ${LONGAN_COMMON_DIR}/sign_config/dragon_toc_a64.cfg dragon_toc.cfg
			else
				cp -v ${TINA_CONFIG_DIR}/generic/sign_config/dragon_toc_a64.cfg dragon_toc.cfg
			fi
		else
			if [ -f ${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/sign_config/dragon_toc.cfg ] ; then
				cp -v ${LONGAN_CONFIG_DIR}/${PACK_BOARD#*-}/sign_config/dragon_toc.cfg dragon_toc.cfg
			elif [ -f ${LONGAN_CONFIG_DIR}/default/dragon_toc.cfg ] ; then
				cp -v ${LONGAN_CONFIG_DIR}/default/dragon_toc.cfg dragon_toc.cfg
			elif [ -f ${TINA_CONFIG_DIR}/${PACK_BOARD}/sign_config/dragon_toc.cfg ] ; then
				cp -v ${TINA_CONFIG_DIR}/${PACK_BOARD}/sign_config/dragon_toc.cfg dragon_toc.cfg
			elif [ -f ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc.cfg ] ; then
				cp -v ${TINA_CONFIG_DIR}/${PACK_BOARD_PLATFORM}-common/sign_config/dragon_toc.cfg dragon_toc.cfg
			elif [ -f ${LONGAN_COMMON_DIR}/sign_config/dragon_toc.cfg dragon_toc.cfg ] ; then
				cp -v ${LONGAN_COMMON_DIR}/sign_config/dragon_toc.cfg dragon_toc.cfg
			else
				cp -v ${TINA_CONFIG_DIR}/generic/sign_config/dragon_toc.cfg dragon_toc.cfg
			fi
		fi
	fi

	if [ $? -ne 0 ]
	then
		pack_error "dragon toc config file is not exist"
		exit 1
	fi

	if [ ! -f ${ROOT_DIR}/boot_initramfs_recovery.img ]; then
		printf "recovery img is not exist, remove recovery cert from dragon_toc.cfg"
		sed -i '/recovery/d' dragon_toc.cfg > /dev/null
	fi

	# add squashfs to dragon_toc.cfg
	grep "CONFIG_TARGET_ROOTFS_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null && \
		grep "CONFIG_USE_UBOOT_VERIFY_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		sed -i '/^onlykey=boot/a\onlykey=rootfs,          rootfs-extract.fex,        SCPFirmwareContentCertPK' dragon_toc.cfg
	fi

	rm -f cardscript.fex
	mv cardscript_secure.fex cardscript.fex
	if [ $? -ne 0 ]
	then
		pack_error "dragon cardscript_secure.fex file is not exist"
		exit 1
	fi

	if [ x${SUFFIX} == x'' ]; then
		dragonsecboot -toc0 dragon_toc.cfg ${ROOT_DIR}/keys ${ROOT_DIR}/image/version_base.mk
		if [ $? -ne 0 ]
		then
			pack_error "dragon toc0 run error"
			exit 1
		fi
	fi

	update_toc0  toc0.fex           sys_config${SUFFIX}.bin
	if [ $? -ne 0 ]
	then
		pack_error "update toc0 run error"
		exit 1
	fi

	if [ x${SUFFIX} == x'' ]; then
		if [ ${merge_flag} == 1 ]; then
			printf "dragon boot package\n"
			dragonsecboot -pack dragon_toc.cfg
			if [ $? -ne 0 ]
			then
				pack_error "dragon boot_package run error"
				exit 1
			fi
		fi

		# update optee pubkey
		if [ -f ${ROOT_DIR}/staging_dir/target/usr/dev_kit/arm-plat-${PACK_CHIP}/export-ta_arm32/keys/default_ta.pem ]; then
			${PACK_TOPDIR}/scripts/update_optee_pubkey.py \
				--in_file optee.fex --out_file optee-new.fex \
				--key ${ROOT_DIR}/staging_dir/target/usr/dev_kit/arm-plat-${PACK_CHIP}/export-ta_arm32/keys/default_ta.pem
			mv optee-new.fex optee.fex
		fi

		if [ -f ${LONGAN_COMMON_DIR}/sign_config/cnf_base.cnf ] ; then
			CNF_BASE_FILE=${LONGAN_COMMON_DIR}/sign_config/cnf_base.cnf
		else
			CNF_BASE_FILE=${TINA_CONFIG_DIR}/generic/sign_config/cnf_base.cnf
		fi
		dragonsecboot -toc1 dragon_toc.cfg ${ROOT_DIR}/keys \
			${CNF_BASE_FILE} \
			${ROOT_DIR}/image/version_base.mk
		if [ $? -ne 0 ]
		then
			pack_error "dragon toc1 run error"
			exit 1
		fi

		local correct_boot_img_magic=$(echo -n ANDROID! | md5sum | cut -d " " -f 1)

		local boot_fex_magic=$(dd if=boot.fex bs=1 count=8 | md5sum | cut -d " " -f 1)
		if [ x$correct_boot_img_magic != x$boot_fex_magic ] ; then
			pack_error "boot.fex format error, magic not ANDROID!"
			exit 1
		fi

		sigbootimg --image boot.fex --cert toc1/cert/boot.der --output boot_sig.fex
		if [ $? -ne 0 ] ; then
			pack_error "Pack cert to image error"
			exit 1
		else
			mv -f boot_sig.fex boot.fex
		fi

		# add cert behind recovery image
		if [ -f ${ROOT_DIR}/boot_initramfs_recovery.img ]; then
			local recovery_fex_magic=$(dd if=recovery.fex bs=1 count=8 | md5sum | cut -d " " -f 1)
			if [ x$correct_boot_img_magic != x$recovery_fex_magic ] ; then
				pack_error "recovery.fex format error, magic not ANDROID!"
				exit 1
			fi
			du -b -L recovery.fex
			sigbootimg --image recovery.fex --cert toc1/cert/recovery.der --output recovery_sig.fex
			if [ $? -ne 0 ] ; then
				pack_error "Pack cert to image error"
				exit 1
			else
				mv -f recovery_sig.fex recovery.fex
			fi
			du -b recovery.fex
		fi
	fi

	update_toc1  toc1.fex           sys_config${SUFFIX}.bin
	if [ $? -ne 0 ]
	then
		pack_error "update toc1 run error"
		exit 1
	fi
	echo "secure signature ok!"
}

function do_pack_tina()
{
	printf "packing for tina linux\n"

	rm -rf vmlinux.fex
	rm -rf boot.fex
	rm -rf rootfs.fex
	rm -rf kernel.fex
	rm -rf rootfs_squashfs.fex
	rm -rf usr.fex
	rm -rf recovery.fex
	rm -rf rootfs-extract.fex
	#ln -s ${ROOT_DIR}/vmlinux.tar.bz2 vmlinux.fex
	if [ ! -f vmlinux.fex ]; then
		echo "vmlinux" > vmlinux.fex
	fi

	# sys_partition_nor.fex in longan may use kernel.fex, which always link to uImage
	ln -s ${ROOT_DIR}/uImage        kernel.fex

	if [ -f ${ROOT_DIR}/boot_initramfs.img ]; then
		ln -s ${ROOT_DIR}/boot_initramfs.img        boot.fex
	else
		ln -s ${ROOT_DIR}/boot.img        boot.fex
	fi

	if [ -f ${ROOT_DIR}/${PACK_BOARD}-Image.gz ] ; then
		cp ${ROOT_DIR}/${PACK_BOARD}-Image.gz Image.gz
	fi

	if [ -f Image.gz -a -f fit-image.its ] ; then
		mkimage -f fit-image.its fit-image.fex
	fi

	if [ "x${PACK_SIG}" = "xsecure" ] ; then
		cp -vf ${ROOT_DIR}/rootfs.img rootfs.fex
		# get sample from squashfs rootfs
		grep "CONFIG_TARGET_ROOTFS_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null && \
			grep "CONFIG_USE_UBOOT_VERIFY_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			# get sample from squashfs rootfs
			local rootfs_per_MB=`grep "^rootfs_per_MB=" ${ROOT_DIR}/image/env.cfg | awk -F = '{printf $2}'`
			if [ -z $rootfs_per_MB ]; then
				echo "rootfs_per_MB is not defined in ${ROOT_DIR}/image/env.cfg, use default value 4096"
				rootfs_per_MB=4096
			fi

			extract_squashfs $rootfs_per_MB rootfs.fex rootfs-extract.fex
			if [ $? -ne 0 ]; then
				echo "extract squashfs error"
				exit 1;
			fi
		fi
	else
		ln -s ${ROOT_DIR}/rootfs.img     rootfs.fex
	fi

	if [ -f ${ROOT_DIR}/usr.img ]; then
		ln -s ${ROOT_DIR}/usr.img    usr.fex
	fi

	if [ -f ${ROOT_DIR}/boot_initramfs_recovery.img ]; then
		ln -s ${ROOT_DIR}/boot_initramfs_recovery.img recovery.fex
	else
		touch recovery.fex
		echo "recovery part not used!" > recovery.fex
	fi
	# Those files is ready for SPINor.
	#ln -s ${ROOT_DIR}/uImage          kernel.fex
	#ln -s ${ROOT_DIR}/rootfs.squashfs rootfs_squashfs.fex

	# add for dm-verity block
	grep "CONFIG_USE_DM_VERITY=y" ${PACK_TOPDIR}/.config > /dev/null
	if [ $? -eq 0 ]; then
		cp -vf ${ROOT_DIR}/rootfs.img rootfs.fex
		${PACK_TOPDIR}/scripts/dm-verity-block.sh ${ROOT_DIR}/image/rootfs.fex
		if [ $? -ne 0 ]; then
			echo "error: generate verity block error!"
			exit
		fi
	fi

	if [ "x${PACK_SIG}" = "xsecure" ] ; then
		echo "secure"
		do_signature
	elif [ "x${PACK_SIG}" = "xprev_refurbish" ] ; then
		echo "prev_refurbish"
		do_signature
	else
		echo "normal"
	fi

	if [ "x${PACK_SIG}" = "xsecure" ] ; then
		# append the signature to the behind of rootfs.fex
		grep "CONFIG_TARGET_ROOTFS_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null && \
			grep "CONFIG_USE_UBOOT_VERIFY_SQUASHFS=y" ${PACK_TOPDIR}/.config > /dev/null
		if [ $? -eq 0 ]; then
			update_squashfs ${ROOT_DIR}/image/rootfs.fex ${ROOT_DIR}/image/toc1/cert/rootfs.der
			if [ $? -ne 0 ]
			then
				pack_error "signature squashfs rootfs error."
				exit 1
			fi
		fi
	fi
}

function do_pack_partition_file()
{
	#use sys_partition_tmp
	cp $1 sys_partition_tmp.fex

	#don't care about ; line
	sed -i '/^[ \t]*;/d' sys_partition_tmp.fex
	sed -i 's/"/ /g' sys_partition_tmp.fex
	/bin/busybox unix2dos sys_partition_tmp.fex
	script  sys_partition_tmp.fex > /dev/null

	#rm img_*.tar.gz readme
	rm -f img_[0-9]*.tar.gz
	rm -f readme

	#touch readme
	touch readme
	local tarimg_name=`date +"%Y%m%d_%H%M%S"`
	echo [create time] >> readme
	echo ${tarimg_name} >> readme
	echo "" >> readme

	local tar_file="${IMG_NAME} ./image/sunxi_mbr.fex ./image/sys_partition.fex"

	if [ "x${PACK_SIG}" = "xsecure" ];then
		tar_boot_file="./image/boot0_nand.fex ./image/toc0.fex ./image/toc1.fex"
	else
		tar_boot_file="./image/boot0_nand.fex ./image/boot_package.fex"
	fi
	tar_file="$tar_file $tar_boot_file"

	downloadfile=(`grep -rn "downloadfile" sys_partition_tmp.fex | awk '{print $4}'`)
	for file in ${downloadfile[@]} ; do
		if [[ $tar_file =~ "./image/$file" ]]; then
			echo "tar_file already have $file, skip it"
		else
			tar_file="$tar_file ./image/$file"
		fi
	done

	echo [image md5sum] >> readme
	for file in $tar_file ; do
		echo "file:$file"
		echo `md5sum --binary $file` >> readme
	done

	#delete "*./image/"
	sed -i 's/\*\.\/image\///g' readme

	tar -zcv -h -f img_${tarimg_name}.tar.gz $tar_file readme > /dev/null

	#clean
	rm -f sys_partition_tmp.fex readme

	#announce
	echo '---------- tar image is at----------'
	echo -e '\033[0;31;1m'
	echo ${ROOT_DIR}/img_${tarimg_name}.tar.gz
	echo -e '\033[0m'
}

function do_mkbootimg_add_dtb()
{
	local LOAD_ADDRESS=0x40008000
	local ENTRY_POINT=0x40008000
	local BASE_ADDRESS=0x40000000
	local KERNEL_OFFSET=0x8000
	local RAMDISK_OFFSET=0x01000000
	local DTB_OFFSET=0x01008000
	local IMAGE_DATA=zImage
	local BOOTIMG_IMAGE_DATA=zImage
	local RAMDISK=ramdisk.img
	local DTB=sunxi.dtb
	local UIMAGE_NAME=uImage
	local BOOTIMG_NAME=boot.img
	local KDIR=${ROOT_DIR}/compile_dir/target/linux-${PACK_BOARD}
	# it is a fake ramdisk, because the Android image must have a ramdisk
	local RDIR=${TINA_CONFIG_DIR}/generic/image
	local DDIR=${ROOT_DIR}/image
	# Only version 2 can add dtb
	local HEADER_VERSION=0x2

	echo "====================mkbootimg add dtb start===================="
	if [ -e ${ROOT_DIR}/boot.img ]; then
		rm ${ROOT_DIR}/boot.img
	fi
	echo "mkbootimg --kernel ${KDIR}/${BOOTIMG_IMAGE_DATA} --ramdisk ${RDIR}/${RAMDISK} --dtb ${DTB} --board ${PACK_BOARD} --base ${BASE_ADDRESS} --kernel_offset ${KERNEL_OFFSET} --ramdisk_offse ${RAMDISK_OFFSET} --dtb_offset ${DTB_OFFSET} --header_version ${HEADER_VERSION} -o ${ROOT_DIR}/boot.img"
	mkbootimg --kernel ${KDIR}/${BOOTIMG_IMAGE_DATA} --ramdisk ${RDIR}/${RAMDISK} --dtb ${DTB} --board ${PACK_BOARD} --base ${BASE_ADDRESS} --kernel_offset ${KERNEL_OFFSET} --ramdisk_offse ${RAMDISK_OFFSET} --dtb_offset ${DTB_OFFSET} --header_version ${HEADER_VERSION} -o ${ROOT_DIR}/boot.img
	echo "=====================mkbootimg add dtb end====================="
}

[ -e ${PACK_TOPDIR}/scripts/.hooks/pre-pack ] &&
	source ${PACK_TOPDIR}/scripts/.hooks/pre-pack

do_early_prepare

while [ $MULTI_CONFIG_INDEX -ge 0 ]; do
	update_suffix
	do_prepare
	do_ini_to_dts
	do_common
	let "MULTI_CONFIG_INDEX=MULTI_CONFIG_INDEX-1"
done

grep "CONFIG_SUNXI_MKBOOTIMG_ADD_DTB=y" ${PACK_TOPDIR}/.config > /dev/null
if [ $? -eq 0 ]; then
	do_mkbootimg_add_dtb
fi

do_pack_${PACK_PLATFORM}
do_finish

if [ "x${PACK_TAR_IMAGE}" = "xtar_image" ]; then
	do_pack_partition_file ${ROOT_DIR}/image/sys_partition.fex
fi

[ -e ${PACK_TOPDIR}/scripts/.hooks/post-pack ] &&
	source ${PACK_TOPDIR}/scripts/.hooks/post-pack
