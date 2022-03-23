#!/bin/bash

OUT_DIR=$TINA_BUILD_TOP/out/$TARGET_BOARD

rm -rf $OUT_DIR/verity
mkdir -p $OUT_DIR/verity/keys

openssl genrsa -out $OUT_DIR/verity/keys/dm-verity-pri.pem 2048
openssl rsa -in $OUT_DIR/verity/keys/dm-verity-pri.pem -pubout -out $OUT_DIR/verity/keys/dm-verity-pub.pem

cp -rf $OUT_DIR/verity/keys/dm-verity-pri.pem $OUT_DIR/verity/keys/dm-verity-pub.pem $TINA_BUILD_TOP/package/security/dm-verity/files/
cp -rf $OUT_DIR/verity/keys/dm-verity-pub.pem $OUT_DIR/compile_dir/target/rootfs_ramfs/verity_key
