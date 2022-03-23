#ifndef _DIR_H
#define _DIR_H

#include "list.h"
#include "File.h"

typedef time_t DirAccessTime;
typedef time_t DirModifyTime;
typedef uint64_t DirSize;

struct Dir {
	int objectNum;
	struct list_head list;
	char *path;
	char *dirName;
	uint32_t subDirCount;
	uint32_t subFileCount;

	DirSize dirSize;
	DirAccessTime dirAccessTime;
	DirModifyTime dirModifyTime;

	struct Dir *parentDir;
	void *object;

	bool (*renameTo)(const char *newPath, struct Dir *dir);
};

struct Dir *DirInit(const char *path, struct Dir *parentDir);
void DirRelease(struct Dir *dir);

uint32_t DirGetObjectCount(struct Dir *dir);
void updateDirInfo(struct Dir *dir);
void deleteDirInfo(struct Dir *dir);
#endif
