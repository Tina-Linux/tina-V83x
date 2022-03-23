#!/bin/bash

set -e

if [[ "$2" =~ xz$ ]];then
    echo "file end with xz"
    export RAMFS_COMPRESS_METHOD="xz"
elif [[ "$2" =~ gz$ ]];then
    echo "file end with gz"
    export RAMFS_COMPRESS_METHOD="gz"
elif [[ "$2" =~ none$ ]];then
    echo "file end with none"
    export RAMFS_COMPRESS_METHOD="none"
else
    echo "error format,file shoule end with gz or xz or none"
    exit 1
fi

generate_rootfs()
{
   if [ -d skel ] ; then
       if [ "x${RAMFS_COMPRESS_METHOD}" = "xxz" ] ; then
          echo "compress ramfs for xz format"
          (cd skel; find . | fakeroot cpio -o -Hnewc | xz -c -k --format=lzma > ../"$1")
       elif [ "x${RAMFS_COMPRESS_METHOD}" = "xgz" ] ; then
          echo "compress ramfs for gzip format"
          (cd skel; find . | fakeroot cpio -o -Hnewc | gzip > ../"$1")
       elif [ "x${RAMFS_COMPRESS_METHOD}" = "xnone" ] ; then
          echo "compress ramfs for none format"
          (cd skel; find . | fakeroot cpio -o -Hnewc  > ../"$1")
       fi
   else
       echo "skel not exist"
       exit 1
   fi
}

extract_rootfs()
{
    if [ -f "$1" ] ; then
        rm -rf skel && mkdir skel
        if [ "x${RAMFS_COMPRESS_METHOD}" = "xxz" ] ; then
            echo "decompress ramfs for xz format"
            xz -d -c -k $1 | (cd skel; fakeroot cpio -i)
        elif [ "x${RAMFS_COMPRESS_METHOD}" = "xgz" ] ; then
            echo "decompress ramfs for gzip format"
            gzip -dc $1 | (cd skel; fakeroot cpio -i)
        elif [ "x${RAMFS_COMPRESS_METHOD}" = "xnone" ] ; then
            echo "decompress ramfs for none format"
            (cd skel;fakeroot cpio -i -Hnewc < ../"$1")
        fi
    else
        echo "$1 not exist"
        exit 1
    fi
}

if [ $# -ne 2 ]; then
	echo -e "please input correct parameters"
	echo -e "\t[build.sh e rootf.cpio.gz] to extract the rootfs template to skel folder"
	echo -e "\tthen make some changes in the skel folder"
	echo -e "\t[build.sh c rootf.cpio.gz] to create the rootfs from the skel folder"
	exit 1
fi

if [ "$1" = "e" ] ; then
	extract_rootfs $2
elif [ "$1" = "c" ] ; then
	generate_rootfs $2
else
	echo "Wrong arguments"
	exit 1
fi

