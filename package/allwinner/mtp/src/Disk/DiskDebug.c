#include "Disk.h"

#ifdef DEBUG
#include <stdio.h>
#define DLOG(fmt, arg...)               printf("[%s:%u] "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#else
#define DLOG(fmt, arg...)
#endif

typedef struct {
    const char* name;
    uint32_t value;
}ValueEntry;

static const ValueEntry sDiskType[] = {
	{ "ext4",		DISK_FS_TYPE_EXT4 },
	{ "vfat",		DISK_FS_TYPE_FAT },
	{ "fuse",		DISK_FS_TYPE_FUSE },
	{ "proc",		DISK_FS_TYPE_PROC },
	{ "sysfs",		DISK_FS_TYPE_SYSFS },
	{ "cgroup",		DISK_FS_TYPE_CGROUPFS },
	{ "tmpfs",		DISK_FS_TYPE_TMPFS },
	{ "debugfs",		DISK_FS_TYPE_DEBUGFS },
	{ "devtmpfs",		DISK_FS_TYPE_DEVTMPFS },
	{ "devpts",		DISK_FS_TYPE_DEVPTS },
	{ "rootfs",		DISK_FS_TYPE_ROOTFS },
	{ "squashfs",		DISK_FS_TYPE_SQUASHFS },
};

static const char* getValueName(uint32_t value, const ValueEntry* table) {
    const ValueEntry * entry = table;
    while (entry->name) {
        if (entry->value == value)
            return entry->name;
        entry++;
    }
    return "UNKNOWN";
}
const char *getDiskType(DiskType type)
{
	return getValueName(type, sDiskType);
}
