#!/bin/bash

help_info()
{
	echo -e "Usage: $0 <rootfs>"
}

OUT_DIR=$TINA_BUILD_TOP/out/$TARGET_BOARD
HOST_DIR=$TINA_BUILD_TOP/out/host/bin

if [ ! -d "$OUT_DIR/verity" ]; then
	echo "error: $OUT_DIR/verity is not exit!"
	echo "Please enable CONFIG_PACKAGE_dm-verity or run ./script/dm-verity-keys.sh"
	exit
fi

IN_FILE=$1
VERITY_DIR=$OUT_DIR/verity

rm -rf $VERITY_DIR/raw_table $VERITY_DIR/hash_tree $VERITY_DIR/sign

# 1. gen raw_table and hash_tree
$HOST_DIR/veritysetup format $IN_FILE $VERITY_DIR/hash_tree > $VERITY_DIR/raw_table

# 2. gen signature of raw_table
openssl dgst -sha256 -binary -sign $VERITY_DIR/keys/dm-verity-pri.pem $VERITY_DIR/raw_table > $VERITY_DIR/sign

#3. padding sign, raw_table and hash_tree to input file
generate_squashfs_verity $IN_FILE $VERITY_DIR/sign $VERITY_DIR/raw_table $VERITY_DIR/hash_tree
if [ $? -ne 0 ]; then
	echo "generate squashfs verity error!"
	exit 1
fi
