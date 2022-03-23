#!/bin/bash

###############################################################
# step:
# 1. extract rootfs_tina_xxbit.cpio.gz
# 2. add packages to initramfs dir
# 3. remove unused libraries and resources in initramfs
# 4. generate new rootfs_tina_xxbit.cpio.gz
###############################################################

help_info()
{
	# get the basename of this scripts
	local shell_name=`basename $SH_NAME`

	echo -e "v1.0\n"
	echo -e "This script will add some packages to tina/lichee/linux-x.x/rootfs.cpio.gz\n"
	echo -e "\033[32mUsage:\n\t./$shell_name <package1> <package2> ...\033[0m\n"
	echo -e "\t<packageX> means the packages directory, like:"
	echo -e "\t\t   tina/out/xxx-xxx/compile_dir/target/xxxx/\n"
}

build_rootfs()
{
	cd $KERN_DIR
	./scripts/build_rootfs.sh $1 $2
	cd -
}

add_packages()
{
	# install packages to initramfs dir
	for i in $@
	do
		# rm '/' at the end of of a directory path if it has.
		pkg_dir=${i%/}

		sub_dir=`ls $pkg_dir/ipkg-sunxi/`

		if [ -z "$sub_dir" ]; then
			echo -e "\033[33m==WARNING==\033[0m $sub_dir does not exist!"
			continue
		fi

		for j in $sub_dir
		do
			cp -rf $pkg_dir/ipkg-sunxi/$j/* $KERN_DIR/skel/
			rm -rf $KERN_DIR/skel/CONTROL

			pkg_libs=`cat $pkg_dir/../../../staging_dir/target/pkginfo/$j.provides`
			copy_libs="$pkg_libs $copy_libs"
		done
	done

	# keep each lib one
	copy_libs=`echo $copy_libs | sed 's/ /\n/g' | sort -u`

	# copy libs to initramfs
	for k in $copy_libs
	do
		if [ -f "$ROOTFS_DIR/lib/$k" ]; then
			cp -rf $ROOTFS_DIR/lib/$k $KERN_DIR/skel/lib
		elif [ -f "$ROOTFS_DIR/usr/lib/$k" ]; then
			cp -rf $ROOTFS_DIR/usr/lib/$k $KERN_DIR/skel/usr/lib
		else
			echo -e "\033[33m==WARNING==\033[0m No such library: $k in $ROOTFS_DIR"
		fi
	done
}


################################################################################
SH_NAME=`readlink -f $0`

# check param
if [ $# -lt 1 ]; then
	help_info
	exit
fi

# get linux version
TMP=`readlink -f $1`
case $TMP in
	*"r16"* | *"r11"* | *"r7"*)
		KERNEL_VERSION=linux-3.4
		;;
	*"r6"* | *"r40"*)
		KERNEL_VERSION=linux-3.10
		;;
	*"r18"*)
		KERNEL_VERSION=linux-4.4
		;;
	*)
		exit 1
		echo "ERROR: $1"
		;;
esac

TINA_DIR=`dirname $SH_NAME`/..
KERN_DIR=$TINA_DIR/lichee/${KERNEL_VERSION}
ROOTFS_DIR=${1%/}/../rootfs

# get arch information
file $ROOTFS_DIR/lib/libc.so | grep 64-bit > /dev/null

if [ $? -eq 0 ]; then
	BIT=64bit
else
	BIT=32bit

	grep 'r6' $TMP > /dev/null
	if [ $? -eq 0 ]; then
		BIT=arm9
	fi
fi

# 1. extract rootfs_tina_xxbit.cpio.gz
build_rootfs e rootfs_tina_${BIT}.cpio.gz

# 2. add packages to initramfs dir
add_packages $@

# 3. remove unused libraries and resources in initramfs
$TINA_DIR/scripts/reduce-rootfs-size.sh d $KERN_DIR/skel
$TINA_DIR/scripts/reduce-rootfs-size.sh c $KERN_DIR/skel

# 4. generate new rootfs_tina_xxbit.cpio.gz
build_rootfs c rootfs_tina_${BIT}.cpio.gz
