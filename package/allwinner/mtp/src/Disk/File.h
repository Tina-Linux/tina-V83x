#ifndef _FILE_H
#define _FILE_H

#include "list.h"
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include "Dir.h"

typedef mode_t FileType;
typedef uint64_t FileSize;
typedef time_t FileAccessTime;
typedef time_t FileModifyTime;

enum {
	FILE_TYPE_SOCK = 0x140000,
	FILE_TYPE_LINK = 0x120000,
	FILE_TYPE_REGULAR = 0x100000,
	FILE_TYPE_BLK = 0x60000,
	FILE_TYPE_DIR = 0x40000,
	FILE_TYPE_CHR = 0x20000,
	FILE_TYPE_FIFO = 0x10000,

};

struct File {
	struct list_head list;
	char *path;
	char *fileName;

	FileType fileType;
	FileSize fileSize;
	FileAccessTime fileAccessTime;
	FileModifyTime fileModifyTime;

	struct Dir *parentDir;
	void *object;

	bool (*renameTo)(const char *newPath, struct File *dir);
};

struct File *FileInit(const char *path, struct Dir *parentDir);
void FileRelease(struct File *file);
void updateFileInfo(struct File *file);
void deleteFileInfo(struct File *file);
#endif
