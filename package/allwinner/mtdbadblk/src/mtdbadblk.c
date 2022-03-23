#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <string.h>
#include <libgen.h>
#include <mtd/mtd-user.h>
#include <errno.h>

#define INFO(fmt, ...)  fprintf(stdout, fmt, ## __VA_ARGS__)
#define ERR(fmt, ...)  fprintf(stderr, fmt, ## __VA_ARGS__)

#if DEBUG
#define DBG(fmt, ...)  fprintf(stdout, fmt, ## __VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

#define MTDPATH_LEN 1024
#define BADBLK_CNT 4096
struct mtddev {
	char mtdpath[MTDPATH_LEN];
	unsigned int badblk_cnt;
	unsigned int blks[BADBLK_CNT];
	struct mtd_info_user info;
};

struct context {
	unsigned int mtdcnt;
	struct mtddev *devs;
};

static int add_mtddev(struct context *cxt, char *mtdpath)
{
	struct mtddev *mtddev;

	DBG("add mtddev: %s\n", mtdpath);
	if (!cxt->devs) {
		cxt->devs = malloc(sizeof(struct mtddev));
		if (!cxt->devs) {
			ERR("allocate memory failed - %s\n", strerror(errno));
			return -ENOMEM;
		}
	} else {
		cxt->devs = realloc(cxt->devs,
				(cxt->mtdcnt + 1) * sizeof(struct mtddev));
		if (!cxt->devs) {
			ERR("allocate memory failed - %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	mtddev = &cxt->devs[cxt->mtdcnt];
	cxt->mtdcnt++;

	snprintf(mtddev->mtdpath, MTDPATH_LEN, "%s", mtdpath);
	return 0;
}

static int get_mtddev(struct context *cxt, char **path, int count)
{
	char mtdpath[MTDPATH_LEN], *p;
	int i, ret;

	if (count == 0) {
		for (i = 0; i < 1024; i++) {
			snprintf(mtdpath, MTDPATH_LEN, "/dev/mtd%d", i);
			ret = access(mtdpath, R_OK);
			if (ret)
				break;
			ret = add_mtddev(cxt, mtdpath);
			if (ret)
				return ret;
		}
		return 0;
	}

	for (i = 0; i < count; i++) {
		struct stat s;

		p = path[i];
		ret = access(p, R_OK);
		if (ret)
			continue;
		ret = stat(p, &s);
		if (ret) {
			ERR("stat %s failed - %s\n", p, strerror(errno));
			return -errno;
		}

		if (!S_ISCHR(s.st_mode)) {
			ERR("%s is not char device, skip it\n", p);
			continue;
		}

		ret = add_mtddev(cxt, p);
		if (ret)
			return ret;
	}
	return 0;
}

static int parse_mtddev(struct mtddev *mtddev)
{
	int ret, fd;
	unsigned int addr;
	struct mtd_info_user *info;

	fd = open(mtddev->mtdpath, O_RDONLY);
	if (fd < 0) {
		ERR("open %s failed - %s\n", mtddev->mtdpath, strerror(errno));
		return fd;
	}

	info = &mtddev->info;
	ret = ioctl(fd, MEMGETINFO, info);
	if (ret) {
		ERR("ioctl failed - %s\n", strerror(errno));
		return ret;
	}

	for (addr = 0; addr < info->size; addr += info->erasesize) {
		ret = ioctl(fd, MEMGETBADBLOCK, &addr);
		if (ret == true) {
			mtddev->blks[mtddev->badblk_cnt] = addr / info->erasesize;
			mtddev->badblk_cnt++;
			if (mtddev->badblk_cnt >= BADBLK_CNT) {
				INFO("too much bad block, max %d, ignore the rest\n",
						BADBLK_CNT);
				return 0;
			}
		}
	}
	return 0;
}

static int print_mtddev(struct mtddev *mtd)
{
	int i;

	INFO("%s: %d", basename(mtd->mtdpath), mtd->badblk_cnt);
	for (i = 0; i < mtd->badblk_cnt; i++)
		INFO(" %d", mtd->blks[i]);
	INFO("\n");
}

int main(int argc, char **argv)
{
	int ret, i, cnt;
	struct context *cxt;

	cxt = malloc(sizeof(*cxt));
	if (!cxt) {
		ERR("allocate failed - %s\n", strerror(errno));
		return -ENOMEM;
	}
	cxt->mtdcnt = 0;
	cxt->devs = NULL;

	ret = get_mtddev(cxt, argv + 1, argc - 1);
	if (ret < 0)
		return ret;

	for (i = 0; i < cxt->mtdcnt; i++) {
		ret = parse_mtddev(&cxt->devs[i]);
		if (ret < 0) {
			ERR("parse mtddev failed - %s\n", strerror(errno));
			return ret;
		}

	}

	for (i = 0; i < cxt->mtdcnt; i++) {
		print_mtddev(&cxt->devs[i]);
		cnt = cxt->devs[i].badblk_cnt;
	}
	INFO("total: %d\n", cnt);

	return 0;
}
