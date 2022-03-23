#!/bin/bash

help_msg="
This script will copy the ALSA UCM configuration files to the specified
directory, according to the specified target board.

USAGE:
    cp_ucm_configs_by_target.sh <target> <ucm_dir> <dest_dir>

        <target>: target board name (e.g. r16-parrot, r18-noma, etc.)
        <ucm_dir>: directory of UCM configurations
        <dest_dir>: destination directory that the UCM configurations will be
                    copied
"

if [ "$#" -ne "3" ]; then
    echo "$help_msg"
    exit 1
fi

target="$1"
ucm_dir="$2"
dest_dir="$3"

target_dir=$(find "$ucm_dir" -type d -name "$target")
if [ "x$target_dir" != "x" ]; then
    mkdir -p "$dest_dir"
    cp -r "${target_dir}"/* "$dest_dir"
else
    platform=${target%-*}
    platform_dir=$(find "$ucm_dir" -type d -name "$platform")
    if [ "x$platform_dir" == "x" ]; then
        echo -e "\033[31m[ERROR] [$0] The ALSA UCM configurations of"\
            "$platform do not exist.\033[0m"
        exit 1
    elif [ ! -d "${platform_dir}"/Default ]; then
        echo -e "\033[31m[ERROR] [$0] Neither the ALSA UCM configurations"\
            "of $target nor the default configurations of $platform exist.\033[0m"
        exit 1
    else
        mkdir -p "$dest_dir"
        cp -r "${platform_dir}"/Default/* "$dest_dir"
    fi
fi
