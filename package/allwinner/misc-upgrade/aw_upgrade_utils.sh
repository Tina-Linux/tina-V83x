#!/bin/sh

#err code
ERR_ILLEGAL_ARGS=2
ERR_NETWORK_FAILED=3
ERR_NOT_ENOUGH_SPACE=4
ERR_VENDOR_HOOK_NOT_SUPPORT=5
ERR_MD5_CHECK_FAILED=10

#image package name
RAMDISK_PKG=ramdisk_sys.tar.gz
TARGET_PKG=target_sys.tar.gz
BOOT0_PKG=boot0_sys.tar.gz
UBOOT_PKG=uboot_sys.tar.gz
USR_PKG=usr_sys.tar.gz

#image dir name
RAMDISK_DIR=ramdisk_sys
TARGET_DIR=target_sys
BOOT0_DIR=boot0_sys
UBOOT_DIR=uboot_sys
USR_DIR=usr_sys

#image name
RAMDISK_IMG=boot_initramfs.img
BOOT_IMG=boot.img
ROOTFS_IMG=rootfs.img
BOOT0_IMG=boot0.img
UBOOT_IMG=uboot.img
USR_IMG=usr.img
RECOVERY_IMG=recovery.img
LOGO_IMG=bootlogo.img

# REBOOT_TO_RECOVERY=1
#	reboot twice. simple and robust.
#	update recovery --> reboot to recovery system --> update boot/rootfs --> reboot to new system
# REBOOT_TO_RECOVERY=0
#	reboot once. need to make sure no process access the rootfs during ota
#	prepare_env --> update recovery --> update boot/rootfs --> reboot to new system
REBOOT_TO_RECOVERY=1
SUPPORT_TINYPLAY=0
STREAM_UPDATE=0

#PID1=`busybox ps | busybox awk '$1==1 {print $5}'`
PID1=`busybox awk 'NR==1 {print $2}' /proc/1/status`
if [ x"$PID1" = x"init" ];then
	INIT_MODE="BUSYBOX_INIT"
else
	INIT_MODE="PROCD_INIT"
fi

# for compatibility
# busybox-init before tina-3.5 don't use overlayfs, it mount rootfs_data in /etc
fgrep ' /etc ' /proc/mounts && mount_on_etc=1

# when overlay don't need workdir, overlayfs upper dir is /overlay
# when overlay need workdir, overlayfs upper dir is /overlay/upper
OVERLAYFS_UPPER_DIR="/overlay/upper"
fgrep -sq 'Linux version 3' /proc/version && OVERLAYFS_UPPER_DIR="/overlay"

if [ x"$mount_on_etc" = x"1" ]; then
	UPGRADE_SETTING_PATH=/etc/.misc-upgrade/
else
	UPGRADE_SETTING_PATH=$OVERLAYFS_UPPER_DIR/etc/.misc-upgrade/
fi

