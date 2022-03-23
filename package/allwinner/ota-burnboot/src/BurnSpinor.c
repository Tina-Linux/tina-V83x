/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "Utils.h"
#include "BurnSpinor.h"

#define BOOT0_MAGIC "eGON.BT0"
#define UBOOT_MAGIC "sunxi-package"
#define MBR_MAGIC   ""

#define BOOT_SPINOR_PATH "/dev/mtdblock0"

struct part_info {
unsigned int part_size;
unsigned int boot0_offset;
unsigned int uboot_offset;
unsigned int mbr_offset;
};

static int find_str_from_bin(const char *str , char *bin_buf, unsigned int len)
{
	int ret, i;
	int str_len = strlen(str);

	for(i=0; i<len; i++) {
		if ((i + str_len) > (len - 1))
			break;
		ret = memcmp((bin_buf + i), str, str_len);
		if (!ret)
			return i;
	}
	return -1;
}

static int parse_part_offset(const char *dev_path, struct part_info *boot_info)
{
	int ret;
	char *buf = NULL;
	FILE *fp = NULL;

	memset(boot_info, 0, sizeof(boot_info));

	fp = fopen(dev_path, "r");
	if (fp == NULL) {
		printf("can not open %s\n", dev_path);
		goto erro;
	}

	/* get part sie */
	fseek(fp, 0, SEEK_END);
	boot_info->part_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (!boot_info->part_size) {
		printf("cannt cat boot part size\n");
		goto erro;
	}

	/*read the boot date to buf and parse*/
	buf = (char *)malloc(sizeof(char) * boot_info->part_size);
	if (buf ==NULL ) {
		printf("cannt malloc buf size %u\n",  boot_info->part_size);
		goto erro;
	}
	fread(buf, boot_info->part_size, 1, fp);
	/*get boot0 offset*/
	ret = find_str_from_bin(BOOT0_MAGIC, buf, boot_info->part_size);
	if (ret > 0)
		/* boot0 magic will offset 4 byte from start*/
		boot_info->boot0_offset = ret - 4;
	/*get uboot offset*/
	ret = find_str_from_bin(UBOOT_MAGIC, buf, boot_info->part_size);
	if (ret > 0)
		boot_info->uboot_offset = ret;

	if (boot_info->uboot_offset)
		/* mbr defualt size is 16K ,and it locate in the end of part*/
		boot_info->mbr_offset =
				    (boot_info->part_size - 1) - (16 * 1024);

	if (fp)
		fclose(fp);
	if (buf)
		free(buf);
	return 0;

erro:
	if (fp)
		fclose(fp);
	if (buf)
		free(buf);
	return -1;

}

int burnSpinorBoot0(BufferExtractCookie *cookie) {

	int flash_handle = -1;
	int ret = 0;
	struct part_info boot_info;

	if (checkBoot0Sum(cookie)) {
		ob_error("wrong boot0 binary file!\n");
		return -1;
	}

	ret = parse_part_offset(BOOT_SPINOR_PATH, &boot_info);
	if (ret){
		printf("this part is not boot part \n");
		return -1;
	}

	/*show part info*/
#if 1
	printf("The part size: %u \n", boot_info.part_size);
	printf("The boot0 offset: %u \n", boot_info.boot0_offset);
	printf("The uboot offset: %u \n", boot_info.uboot_offset);
	printf("The mbr offset: %u \n", boot_info.mbr_offset);
#endif

	/*burn boot0 img to flash*/
	flash_handle = open(BOOT_SPINOR_PATH, O_SYNC | O_RDWR);
	if (flash_handle < 0) {
		printf("cant open %s \n", BOOT_SPINOR_PATH);
		return -1;
	}

	if (cookie->len > (boot_info.uboot_offset - boot_info.boot0_offset)) {
		printf("boot0 img size > boot0 part size ,will not burn\n");
		return -1;
	}

	lseek(flash_handle, boot_info.boot0_offset, SEEK_SET);
	ret = write(flash_handle, cookie->buffer, cookie->len);

	system("sync");
	system("sync");
	system("sync");

	return 0;
}

int burnSpinorUboot(BufferExtractCookie *cookie) {

	int flash_handle = -1;
	int ret = 0;
	struct part_info boot_info;

	if (checkUbootSum(cookie)) {
		ob_error("wrong uboot binary file!\n");
		return -1;
	}

	/* parse boot part, and find boot0/uboot offset */
	ret = parse_part_offset(BOOT_SPINOR_PATH, &boot_info);
	if (ret){
		printf("this part is not boot part \n");
		return -1;
	}

	/*show part info*/
#if 1
	printf("The part size: %u \n", boot_info.part_size);
	printf("The boot0 offset: %u \n", boot_info.boot0_offset);
	printf("The uboot offset: %u \n", boot_info.uboot_offset);
	printf("The mbr offset: %u \n", boot_info.mbr_offset);
#endif

	/*burn boot0 img to flash*/
	flash_handle = open(BOOT_SPINOR_PATH, O_SYNC | O_RDWR);
	if (flash_handle < 0) {
		printf("cant open %s \n", BOOT_SPINOR_PATH);
		return -1;
	}

	if (cookie->len > (boot_info.mbr_offset - boot_info.uboot_offset)) {
		printf("boot0 img size > boot0 part size ,will not burn\n");
		return -1;
	}

	lseek(flash_handle, boot_info.uboot_offset, SEEK_SET);
	ret = write(flash_handle, cookie->buffer, cookie->len);

	system("sync");
	system("sync");
	system("sync");

	return 0;

}
