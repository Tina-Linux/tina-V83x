#define DEBUG
#include "Dir.h"
#include "File.h"
#include "DiskCommon.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

static void getFileInfo(struct Dir *dir)
{
	struct stat sb;

	if (stat(dir->path, &sb) == -1)
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
	dir->dirSize = sb.st_size;
	dir->dirAccessTime = sb.st_atime;
	dir->dirModifyTime = sb.st_mtime;

	DLOG(" <%s> size: %luM %luK %lubytes", dir->dirName, dir->dirSize>>20, dir->dirSize>>10, dir->dirSize);
	DLOG(" <%s> accecc time: %s", dir->dirName, ctime(&dir->dirAccessTime));
	DLOG(" <%s> modify time: %s", dir->dirName, ctime(&dir->dirModifyTime));
	return ;
}

char *getFileNameFromPath(const char *path, char *name, size_t *size)
{
	char *ptr = NULL;
	ptr = strrchr(path, '/');
	if (ptr != NULL) {
		size_t len = strlen(++ptr);
		if (len < 1 || len > *size)
			goto err;
		*size = len + 1;
		strcpy(name, ptr);
		return name;
	}
err:
	printf("path unknown: %s\n", path);
	return NULL;
}

void updateDirInfo(struct Dir *dir)
{
	getFileInfo(dir);
}

void deleteDirInfo(struct Dir *dir)
{
	list_del(&dir->list);
}

static bool renameTo(const char *newPath, struct Dir *dir)
{
	int ret;
	char buf[128];
	size_t len = sizeof(buf);

	if (access(newPath, F_OK) == 0)
		return false;
	ret = rename(dir->path, newPath);
	if (ret != 0) {
		printf("rename oldpath:%s to newpath:%s failed, errno:%d",
				dir->path, newPath, errno);
		return false;
	}
	if (dir->path)
		free_wrapper(dir->path);
	dir->path = strdup_wrapper(newPath);
	if (dir->dirName)
		free_wrapper(dir->dirName);
	if (getFileNameFromPath(newPath, buf, &len) != NULL)
		dir->dirName = strdup_wrapper(buf);
	getFileInfo(dir);
	return true;
}

struct Dir *DirInit(const char *path, struct Dir *parentDir)
{
	struct Dir *dir;
	size_t len;
	char buf[128];

	if (!path)
		return NULL;
	DLOG(" path:%s", path);
	dir = (struct Dir *)calloc_wrapper(1, sizeof(struct Dir));
	memset(dir, 0, sizeof(struct Dir));

	dir->path = strdup_wrapper(path);

	len = sizeof(buf);
	if (getFileNameFromPath(path, buf, &len) != NULL)
		dir->dirName = strdup_wrapper(buf);

	getFileInfo(dir);
	INIT_LIST_HEAD(&dir->list);
	dir->parentDir = parentDir;

	dir->renameTo = renameTo;

	DLOG("init !! dir:%p", dir);
	return dir;
}

void DirRelease(struct Dir *dir)
{
	DLOG("exit !! dir:%p", dir);
	if (!dir)
		return ;
	if (dir->path)
		free_wrapper(dir->path);
	if (dir->dirName)
		free_wrapper(dir->dirName);
	free_wrapper(dir);
}
