#include "MtpStorage.h"
#include "mtp.h"
#include "MtpCommon.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h>


uint64_t MtpStorageGetFreeSpace(struct MtpStorage *storage)
{
	struct statfs stat;
	uint64_t freeSpace;
	if (statfs(storage->mFilePath, &stat))
		return -1;
	freeSpace = (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
	return (freeSpace > storage->mReserveSpace ? freeSpace - storage->mReserveSpace : 0);
}

struct MtpStorage *MtpStorageInit(MtpStorageID id, const char *path,
					const char *description,
					uint64_t reserveSpace,
					uint64_t maxFileSize)
{
	struct MtpStorage *mStorage = NULL;

	mStorage = (struct MtpStorage *)calloc_wrapper(1, sizeof(struct MtpStorage));

	mStorage->mDisk = DiskInit(path);
	if (!mStorage->mDisk) {
		printf("mDisk init failed, path:%s\n", path);
		free_wrapper(mStorage);
		return NULL;
	}

	mStorage->mFilePath = strdup_wrapper(path);

	mStorage->mDescription = strdup_wrapper(description);

	mStorage->mMaxCapacity = mStorage->mDisk->dMaxCap;
	mStorage->mFreeSpace = mStorage->mDisk->dFreeSpace;

	mStorage->mReserveSpace = reserveSpace;
	mStorage->mMaxFileSize = maxFileSize;

	mStorage->mStorageID = id;

	return mStorage;
}

void MtpStorageRelease(struct MtpStorage *mStorage)
{
	if (!mStorage)
		return;
	if (mStorage->mDisk)
		DiskRelease(mStorage->mDisk);
	if (mStorage->mFilePath)
		free_wrapper(mStorage->mFilePath);
	if (mStorage->mDescription)
		free_wrapper(mStorage->mDescription);
	free_wrapper(mStorage);
	return;
}
