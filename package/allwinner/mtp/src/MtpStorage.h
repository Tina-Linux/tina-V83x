#ifndef _MTPSTORAGE_H
#define _MTPSTORAGE_H

#include "Disk/Disk.h"
#include "MtpTypes.h"

struct MtpStorage {
	MtpStorageID mStorageID;
	uint64_t mMaxCapacity;
	uint64_t mFreeSpace;
	uint64_t mReserveSpace;
	uint64_t mMaxFileSize;

	char *mFilePath;
	char *mDescription;

	struct Disk *mDisk;
};

struct MtpStorage *MtpStorageInit(MtpStorageID id, const char *path,
					const char *description,
					uint64_t reserveSpace,
					uint64_t maxFileSize);
void MtpStorageRelease(struct MtpStorage *mStorage);

static inline uint64_t MtpStorageGetMaxFileSize(struct MtpStorage *storage)
{
	return storage->mMaxFileSize;
}
static inline const char *MtpStorageGetPath(struct MtpStorage *storage)
{
	return storage->mFilePath;
}

uint64_t MtpStorageGetFreeSpace(struct MtpStorage *storage);

#endif
