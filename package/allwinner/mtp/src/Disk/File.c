#define DEBUG
#include "File.h"
#include "list.h"
#include "DiskCommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

static void getFileInfo(struct File *file)
{
	struct stat sb;

	if (stat(file->path, &sb) == -1)
		return;
#if 0
#ifdef DEBUG
	switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");            break;
		case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");               break;
		case S_IFIFO:  printf("FIFO/pipe\n");               break;
		case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");            break;
		case S_IFSOCK: printf("socket\n");                  break;
		default:       printf("unknown?\n");                break;
	}
	printf("I-node number:            %ld\n", (long) sb.st_ino);
	printf("Mode:                     %lo (octal)\n",
		(unsigned long) sb.st_mode);
	printf("Link count:               %ld\n", (long) sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n",
		(long) sb.st_uid, (long) sb.st_gid);
	printf("Preferred I/O block size: %ld bytes\n",
		(long) sb.st_blksize);
	printf("File size:                %lld bytes\n",
		(long long) sb.st_size);
	printf("Blocks allocated:         %lld\n",
		(long long) sb.st_blocks);
	printf("Last status change:       %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));
#endif
#endif
	file->fileType = sb.st_mode & S_IFMT;
	file->fileSize = sb.st_size;
	file->fileAccessTime = sb.st_atime;
	file->fileModifyTime = sb.st_mtime;

	DLOG(" <%s> size: %luM %luK %lubytes", file->fileName, file->fileSize>>20, file->fileSize>>10, file->fileSize&((1<<10)-1));
	DLOG(" <%s> accecc time: %s", file->fileName, ctime(&file->fileAccessTime));
	DLOG(" <%s> modify time: %s", file->fileName, ctime(&file->fileModifyTime));
	return ;
}

void updateFileInfo(struct File *file)
{
	getFileInfo(file);
}

void deleteFileInfo(struct File *file)
{
	list_del(&file->list);
}

extern char *getFileNameFromPath(const char *path, char *name, size_t *size);

static bool renameTo(const char *newPath, struct File *file)
{
	int ret;
	char buf[128];
	size_t len = sizeof(buf);

	if (access(newPath, F_OK) == 0)
		return false;
	ret = rename(file->path, newPath);
	if (ret != 0) {
		printf("rename oldpath:%s to newpath:%s failed, errno:%d",
				file->path, newPath, errno);
		return false;
	}
	if (file->path)
		free_wrapper(file->path);
	file->path = strdup_wrapper(newPath);
	if (file->fileName)
		free_wrapper(file->fileName);
	if (getFileNameFromPath(newPath, buf, &len) != NULL)
		file->fileName = strdup_wrapper(buf);
	getFileInfo(file);

	return true;
}

struct File *FileInit(const char *path, struct Dir *parentDir)
{
	struct File *file = NULL;
	size_t len;
	char buf[128];

	if (!path)
		return NULL;

	DLOG(" path:%s", path);
	file = (struct File *)calloc_wrapper(1, sizeof(struct File));

	file->path = strdup_wrapper(path);

	len = sizeof(buf);
	if (getFileNameFromPath(path, buf, &len) != NULL) {
		file->fileName = strdup_wrapper(buf);
		DLOG("filename: %p, %s", file->fileName, file->fileName);
	}

	getFileInfo(file);

	INIT_LIST_HEAD(&file->list);

	file->parentDir = parentDir;

	file->renameTo = renameTo;

	DLOG("create File:%p", file);
	return file;
}

void FileRelease(struct File *file)
{
	DLOG("file: %p", file);
	if (!file)
		return ;
	DLOG("path:%p, %s", file->path, file->path);
	if (file->path)
		free_wrapper(file->path);
	DLOG("name:%p, %s", file->fileName, file->fileName);
	if (file->fileName)
		free_wrapper(file->fileName);
	free_wrapper(file);

	return ;
}
