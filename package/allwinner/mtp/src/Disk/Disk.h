#ifndef _DISK_H
#define _DISK_H
#include <stdint.h>
#include "Dir.h"


typedef uint32_t DiskType;
typedef uint32_t DiskFsType;
typedef uint64_t DiskFreeSpace;
typedef uint64_t DiskMaxCapacity;

enum {
	DISK_FS_TYPE_EXT4	= 0xEF53,
	DISK_FS_TYPE_FAT	= 0x4D44,
	DISK_FS_TYPE_FUSE	= 0x65735546,
	DISK_FS_TYPE_PROC	= 0x9FA0,
	DISK_FS_TYPE_SYSFS	= 0x62656572,
	DISK_FS_TYPE_CGROUPFS	= 0x27E0EB,
	DISK_FS_TYPE_TMPFS	= 0x01021994,
	DISK_FS_TYPE_DEBUGFS	= 0x64626720,
	DISK_FS_TYPE_DEVTMPFS	= 0x1021994,
	DISK_FS_TYPE_DEVPTS	= 0x1CD1,
	DISK_FS_TYPE_ROOTFS	= 0x794C764F,
	DISK_FS_TYPE_SQUASHFS	= 0x73717368,
};


struct Disk {
	DiskType dType;
	DiskFsType dFsType;
	DiskFreeSpace dFreeSpace;
	DiskMaxCapacity dMaxCap;

	struct Dir *dDirRoot;

	struct list_head dirList;
	struct list_head fileList;

	char *dMountDir;

	void (*addSubFile)(struct File *file, struct Disk *disk);
	void (*addSubDir)(struct Dir *dir, struct Disk *disk);
};

struct Disk *DiskInit(const char *path);
void  DiskRelease(struct Disk *disk);
#endif
