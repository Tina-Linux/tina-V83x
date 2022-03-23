#!/bin/bash

PLATFORM="sun8iw3p1"
ATF_EXIST=""
MODE=""

show_help()
{
	printf "\nbuild.sh - Top level build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -t install gcc tools chain"
	echo "  helloworld     compile helloworld"
	echo "  encrypt_storage compile encrypt_file_storage"
	echo "  base64demo compile base64_usage"
	echo "  api_demo compile api_demo"
	echo "  clean     clean the tmp files"
	echo "  config    select the platform to compile"
	printf "\n\n"
}

prepare_toolchain()
{
        local ARCH="arm";
        local GCC="";
        local GCC_PREFIX="";
        local toolchain_archive_aarch64="./tools/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu.tar.xz";
        local toolchain_archive_arm="./tools/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi.tar.xz";
        local tooldir_aarch64="./tools/toolchain/gcc-aarch64";
        local tooldir_arm="./tools/toolchain/gcc-arm-gnueabi";

        local toolchain_archive_none="./tools/toolchain/gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.xz";
        local tooldir_none="./tools/toolchain/gcc-arm-none-eabi-5_4-2016q3";
        local toolchain_archive_none_4_7="./tools/toolchain/gcc-arm-none-4_7.tar.xz";
        local tooldir_none_4_7="./tools/toolchain/ak47";
        echo "Prepare toolchain,please wait for a time ..."

        if [ ! -d "${tooldir_aarch64}" ]; then
                mkdir -p ${tooldir_aarch64} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_aarch64} -C ${tooldir_aarch64} || exit 1
        fi

        if [ ! -d "${tooldir_arm}" ]; then
                mkdir -p ${tooldir_arm} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_arm} -C ${tooldir_arm} || exit 1
        fi

        if [ ! -d "${tooldir_none}" ]; then
                mkdir -p ${tooldir_none} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_none} -C ${tooldir_none} || exit 1
        fi
        if [ ! -d "${tooldir_none_4_7}" ]; then
                mkdir -p ${tooldir_none_4_7} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_none_4_7} -C ${tooldir_none_4_7} || exit 1
        fi
}

show_success()
{
	echo -e "\033[40;32m #### make completed successfully  #### \033[0m"
}

show_error()
{
	echo -e "\033[40;31m #### make failed to build some targets  #### \033[0m"
}

clean_all()
{

	make clean -C demo/optee_helloworld
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi

	make clean -C demo/encrypt_file_storage
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi

	make clean -C demo/base64-usage
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi

	make clean -C demo/api_demo
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi
}

build_helloworld()
{
	make -C demo/optee_helloworld
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi
}

build_encrypt_file_storage()
{
	make -C demo/encrypt_file_storage
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi
}

build_base64demo()
{
	make -C demo/base64-usage
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi
}

build_api_demo()
{
	make -C demo/api_demo
	if [ $? -ne 0 ]
	then
		show_error
		exit
	else
		show_success
	fi
}

build_all()
{
	build_helloworld
	build_encrypt_file_storage
	build_base64demo
	build_api_demo
}
probe_config()
{
	if [ ! -f platform_config.mk ]; then
		echo -e "\033[40;31m #### config.mk is not exist!       #### \033[0m"
		echo -e "\033[40;31m #### use: ./build.sh config first  #### \033[0m"
		exit
	fi
}

build_select_chip()
{
	local count=0
	printf "All valid Sunxi chip:\n"
	for chip in $( find dev_kit -mindepth 1 -maxdepth 1 -type d -name "arm-plat-sun[0-9]*" |sort); do
		chips[$count]=`basename $chip`
		printf "$count. ${chips[$count]#arm-plat-}\n"
		let count=$count+1
	done

	while true; do
		read -p "Please select a chip:"
		RES=`expr match $REPLY "[0-9][0-9]*$"`
		if [ "$RES" -le 0 ]; then
			echo "please use index number"
			continue
		fi
		if [ "$REPLY" -ge $count ]; then
			echo "too big"
			continue
		fi
		if [ "$REPLY" -lt "0" ]; then
			echo "too small"
			continue
		fi
		break
	done

	if grep -q 'CFG_WITH_ARM_TRUSTED_FW' dev_kit/${chips[$REPLY]}/"export-ta_arm32"/host_include/conf.mk ; then
		ATF_EXIST='y' ;
	else
		ATF_EXIST='n' ;
	fi

	touch platform_config.mk
	echo "# auto-generated t-coffer configuration file" > platform_config.mk
	echo -e "\nPLATFORM := ${chips[$REPLY]##*-}\n" >> platform_config.mk
	echo -e "ATF_EXIST := ${ATF_EXIST}" >> platform_config.mk
	echo -e "\nexport  PLATFORM\n" >> platform_config.mk
	echo -e "\nexport  ATF_EXIST\n" >> platform_config.mk

}

while [ $# -gt 0 ]; do
	case "$1" in
	-t)
	prepare_toolchain
	exit;
	;;
	-h)
	show_help
	exit;
	;;
	config*)
		build_select_chip;
		exit;
		;;
	distclean)
		if [ ! -f platform_config.mk ]; then
			touch platform_config.mk
		fi
		clean_all
		rm platform_config.mk
		exit;
		;;
	clean)
		clean_all
		exit;
		;;
	helloworld)
		build_helloworld
		exit;
		;;
	encrypt_storage)
		build_encrypt_file_storage
		exit;
		;;
	base64demo)
		build_base64demo
		exit;
		;;
	api_demo)
		build_api_demo
		exit;
		;;
	*) ;;
	esac;
done

probe_config
build_all


