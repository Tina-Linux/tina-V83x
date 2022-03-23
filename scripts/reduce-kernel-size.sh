#!/bin/bash

#This script will reduce the storage size of the kernel by tuning kernel configuration.

SH_NAME=$0
OP_FLAG=$1
CONFIG_FILE=$2

DISABLE_OPTIONS="
CONFIG_KERNEL_GZIP
CONFIG_KERNEL_LZMA
CONFIG_KERNEL_LZO
CONFIG_SYSCTL_SYSCALL
CONFIG_KALLSYMS
CONFIG_KALLSYMS_ALL
CONFIG_BUG
CONFIG_ELF_CORE
CONFIG_KPROBES
CONFIG_MAGIC_SYSRQ
CONFIG_STACKTRACE
CONFIG_DEBUG_INFO
CONFIG_LATENCYTOP
CONFIG_FTRACE
CONFIG_KGDB
CONFIG_DEBUG_LL
"
ENABLE_OPTIONS="
CONFIG_KERNEL_XZ
CONFIG_CC_OPTIMIZE_FOR_SIZE
"

help_info()
{
	# get the basename of this scripts
	local shell_name=`basename $SH_NAME`

	echo -e "v1.0\n"
	echo -e "This script will downsize the kernel or check the downsizing result!\n"
	echo -e "\033[32mUsage:\n\t./$shell_name <op_flag> <kernel_config_file>\033[0m\n"
	echo -e "\t<op_flag>: operation.\n\t\t d - downsize kernel\n\t\t c - check the downsizing result"
	echo -e "\t<kernel_config_file>: the configuration of your kernel.\n"
}

disable_config()
{
	for var in $@
	do
		grep $var $CONFIG_FILE > /dev/null

		if [ $? -eq 0 ]
		then
			grep "# $var is not set" $CONFIG_FILE > /dev/null

			if [ $? -ne 0 ]
			then
				sed -i "s/$var\=y/\#\ $var\ is\ not\ set/g" $CONFIG_FILE
				#echo "$var enable -> disable."
			fi
		else
			echo "# $var is not set" >> $CONFIG_FILE
		fi
	done
}

enable_config()
{
	for var in $@
	do
		grep $var $CONFIG_FILE > /dev/null

		if [ $? -eq 0 ]
		then
			grep "$var=y" $CONFIG_FILE > /dev/null

			if [ $? -ne 0 ]
			then
				sed -i "s/\#\ $var\ is\ not\ set/$var\=y/g" $CONFIG_FILE
				#echo "$var disable -> enable."
			fi
		else
			echo "$var=y" >> $CONFIG_FILE
			#echo "$var none -> enable."
		fi
	done
}

disable_config_check()
{
	for var in $@
	do
		grep $var $CONFIG_FILE > /dev/null

		if [ $? -eq 0 ]
		then
			grep "# $var is not set" $CONFIG_FILE > /dev/null

			if [ $? -eq 0 ]
			then
				echo -e "$var disabled?\n\tCheck result:\033[32m == RIGHT ==\033[0m\n"
			else
				echo -e "$var disabled?\n\tCheck result:\033[31m == ERROR ==\033[0m\n"
			fi
		else
			echo -e "$var disabled?\n\tCheck result:\033[32m == RIGHT ==\033[0m\n"
		fi
	done
}

enable_config_check()
{
	for var in $@
	do
		grep $var $CONFIG_FILE > /dev/null

		if [ $? -eq 0 ]
		then
			grep "$var=y" $CONFIG_FILE > /dev/null

			if [ $? -eq 0 ]
			then
				echo -e "$var enabled?\n\tCheck result: \033[32m == RIGHT ==\033[0m\n"
			else
				echo -e "$var enabled?\n\tCheck result: \033[31m == ERROR ==\033[0m\n"
			fi
		else
			echo -e "$var enabled?\n\tCheck result: \033[31m == ERROR ==\033[0m\n"
		fi
	done
}

###############################################################################
if [ $# -ne 2 ]
then
	help_info
	exit
fi

if [ "x$OP_FLAG" != "xd" ]
then
	if [ "x$OP_FLAG" != "xc" ]
	then
		help_info
		echo -e "\t\033[31m== ERROR ==\033[0m <op_flag>: '$OP_FLAG' should be 'd' or 'c'"
		exit
	fi
fi

if [ ! -f "$CONFIG_FILE" ]
then
	help_info
	echo -e "\t\033[31m== ERROR ==\033[0m <kernel_config_file>: '$CONFIG_FILE' does not exist!"
	exit
fi

case "$OP_FLAG" in
	d)
		disable_config $DISABLE_OPTIONS
		enable_config $ENABLE_OPTIONS
		;;
	c)
		disable_config_check $DISABLE_OPTIONS
		enable_config_check $ENABLE_OPTIONS
		;;
	*)
		help_info
		echo -e "\t\033[31m== ERROR ==\033[0m <op_flag>: '$OP_FLAG' should be 'd' or 'c'"
		;;
esac
