#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "log.h"
#include <pthread.h>
#include "secure_storage_ioctl.h"

#define MAX_SECURE_STORAGE_MAX_ITEM				(32)
#define SDMMC_SECTOR_SIZE						(512)
#define SDMMC_SECURE_STORAGE_START_ADD			(6*1024*1024/512)
#define SDMMC_ITEM_SIZE							(4*1024/512)

/*nand secure storage read/write*/
int _nand_read_ioctl(int id, unsigned char *buf, ssize_t len)
{
	int fd;
	int ret;
	struct sst_storage_data *secblk_op;
	 if (!buf) {
		ALOGE("-buf NULL\n");
		return -1;
	}
	if (id > MAX_SECURE_STORAGE_MAX_ITEM) {
		ALOGE("out of range id %x\n", id);
		return -1;
	}
	fd = open(OEM_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s", OEM_PATH, strerror(errno));
		return -1;
	}
	secblk_op = (struct sst_storage_data *)malloc(sizeof(*secblk_op));
	if (!secblk_op) {
		ALOGE("Out of memory\n");
		close(fd);
		return -1;
	}
	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = 0;
	ret = ioctl(fd, SST_STORAGE_READ, secblk_op);
	if (ret < 0) {
		ALOGE("Nand secblk read fail: ret=%d %s\n", ret,
		       strerror(errno));
		free(secblk_op);
		close(fd);
		return -1;
	}
	fsync(fd);
	free(secblk_op);
	close(fd);
	return 0;
}

int _nand_write_ioctl(int id, unsigned char *buf, ssize_t len)
{
	int fd;
	int ret;
	struct sst_storage_data *secblk_op;
	if (!buf) {
		ALOGE("- buf NULL\n");
		return (-1);
	}

	if (id > MAX_SECURE_STORAGE_MAX_ITEM) {
		ALOGE("out of range id %x\n", id);
		return (-1);
	}

	fd = open(OEM_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s", OEM_PATH, strerror(errno));
		return -1;
	}

	secblk_op = (struct sst_storage_data *)malloc(sizeof(*secblk_op));
	if (!secblk_op) {
		ALOGE("Out of memory\n");
		close(fd);
		return -1;
	}

	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = 0;
	ret = ioctl(fd, SST_STORAGE_WRITE, secblk_op);

	if (ret < 0) {
		ALOGE("Nand secblk write fail: ret=%d %s\n", ret,
		       strerror(errno));
		free(secblk_op);
		close(fd);
		return -1;
	}
	fsync(fd);
	free(secblk_op);
	close(fd);
	return 0;
}


/*emmc secure storage read/write*/
int _sd_read_ioctl(int id, unsigned char *buf, ssize_t len)
{
	int offset, ret;
	int fd;
	struct sst_storage_data *secblk_op;
	 if (!buf) {
		ALOGE("-buf NULL\n");
		return -1;
	}

	if (id > MAX_SECURE_STORAGE_MAX_ITEM) {
		ALOGE("out of range id %x\n", id);
		return (-1);
	}

	fd = open(OEM_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s", OEM_PATH, strerror(errno));
		return -1;
	}
	secblk_op = (struct sst_storage_data *)malloc(sizeof(*secblk_op));
	if (!secblk_op) {
		ALOGE("Out of memory\n");
		close(fd);
		return -1;
	}
	/* raw */
	offset = (SDMMC_SECURE_STORAGE_START_ADD + SDMMC_ITEM_SIZE * 2 * id) * SDMMC_SECTOR_SIZE;
	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = offset;
	ret = ioctl(fd, SST_STORAGE_READ, secblk_op);
	if (ret < 0) {
		ALOGE("sd secblk read fail\n");
		free(secblk_op);
		close(fd);
		return -1;
	}
	fsync(fd);
	free(secblk_op);
	close(fd);
	return 0;
}


/*emmc secure storage backup item read*/
int _sd_read_backup_ioctl(int id, unsigned char *buf, ssize_t len)
{
	int offset, ret;
	int fd;
	struct sst_storage_data *secblk_op;
	if (!buf) {
		ALOGE("-buf NULL\n");
		return -1;
	}
	if (id > MAX_SECURE_STORAGE_MAX_ITEM) {
		ALOGE("out of range id %x\n", id);
		return (-1);
	}
	fd = open(OEM_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s", OEM_PATH, strerror(errno));
		return -1;
	}
	secblk_op = (struct sst_storage_data *)malloc(sizeof(*secblk_op));
	if (!secblk_op) {
		ALOGE("Out of memory\n");
		close(fd);
		return -1;
	}

	/*backup */
	offset = (SDMMC_SECURE_STORAGE_START_ADD + SDMMC_ITEM_SIZE * 2 * id + SDMMC_ITEM_SIZE) * SDMMC_SECTOR_SIZE;
	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = offset;
	ret = ioctl(fd, SST_STORAGE_READ, secblk_op);
	if (ret < 0) {
		ALOGE("sd secblk read fail\n");
		free(secblk_op);
		close(fd);
		return -1;
	}
	fsync(fd);
	free(secblk_op);
	close(fd);
	return 0;
}

int _sd_write_ioctl(int id, unsigned char *buf, ssize_t len)
{
	int offset, ret;
	int fd;
	struct sst_storage_data *secblk_op;
	if (!buf) {
		ALOGE("-buf NULL\n");
		return -1;
	}
	if (id > MAX_SECURE_STORAGE_MAX_ITEM) {
		ALOGE("out of range id %x\n", id);
		return (-1);
	}
	fd = open(OEM_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("Failed to open %s: %s", OEM_PATH, strerror(errno));
		return -1;
	}
	secblk_op = (struct sst_storage_data *)malloc(sizeof(*secblk_op));
	if (!secblk_op) {
		ALOGE("Out of memory\n");
		close(fd);
		return -1;
	}
	offset = (SDMMC_SECURE_STORAGE_START_ADD + SDMMC_ITEM_SIZE * 2 * id) * SDMMC_SECTOR_SIZE;
	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = offset;
	ret = ioctl(fd, SST_STORAGE_WRITE, secblk_op);
	if (ret < 0) {
		ALOGE("sd ioctl read fail\n");
		free(secblk_op);
		close(fd);
		return -1;
	}
	/*backup */
	offset = (SDMMC_SECURE_STORAGE_START_ADD + SDMMC_ITEM_SIZE * 2 * id + SDMMC_ITEM_SIZE) * SDMMC_SECTOR_SIZE;
	secblk_op->id = id;
	secblk_op->buf = (unsigned char *)buf;
	secblk_op->len = len;
	secblk_op->offset = offset;
	ret = ioctl(fd, SST_STORAGE_WRITE, secblk_op);
	if (ret < 0) {
		ALOGE("sd ioctl read fail\n");
		free(secblk_op);
		close(fd);
		return -1;
	}

	fsync(fd);
	free(secblk_op);
	close(fd);
	return 0;
}


