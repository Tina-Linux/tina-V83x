#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "log.h"
#include "common.h"

#define CMDLINE_FILE_PATH   ("/proc/cmdline")

static int flash_boot_type = FLASH_TYPE_UNKNOW;

static int spliteKeyAndValue(char* str, char** key, char** value)
{
	int elocation = strcspn(str,"=");
	if (elocation < 0){
		return -1;
	}
	str[elocation] = '\0';
	*key = str;
	*value = str + elocation + 1;
	return 0;
}

static int getInfoFromCmdline(const char* key, char* value, int val_len)
{
	FILE* fp;
	char cmdline[1024];

	/* read partition info from /proc/cmdline */
	if ((fp = fopen(CMDLINE_FILE_PATH, "r")) == NULL) {
		ALOGD("can't open %s \n", CMDLINE_FILE_PATH);
		return -1;
	}
	fgets(cmdline, 1024, fp);
	fclose(fp);

	/* splite the cmdline by space */
	char* p = NULL;
	char* lkey = NULL;
	char* lvalue = NULL;
	p = strtok(cmdline, " ");
	if (!spliteKeyAndValue(p, &lkey, &lvalue)){
		if (!strcmp(lkey,key)){
			goto done;
		}
	}

	while ((p = strtok(NULL, " "))){
		if (!spliteKeyAndValue(p, &lkey, &lvalue)){
			if (!strcmp(lkey,key)){
				goto done;
			}
		}
	}

	ALOGD("No key named %s in cmdline.\n", key);
	return -1;

done:
	strncpy(value, lvalue, val_len);
	return 0;
}

int flash_type_init(void)
{
	char ctype[20];
	flash_boot_type = FLASH_TYPE_UNKNOW;

	memset(ctype, 0, 16);
	if (getInfoFromCmdline("boot_type", ctype, 10) == 0) {
		/* get type from boot_type */
		ALOGD("flash type = %s\n", ctype);
		flash_boot_type = atoi(ctype);
		if (flash_boot_type == 0 && ctype[0] != '0')
			flash_boot_type = FLASH_TYPE_UNKNOW;
	} else if (getInfoFromCmdline("root", ctype, 20) == 0) {
		/* get type from root */
		if (!strncmp(ctype, "/dev/nand", sizeof("/dev/nand")-1))
			flash_boot_type = FLASH_TYPE_NAND;
		else if (!strncmp(ctype, "/dev/ubi", sizeof("/dev/ubi")-1))
			flash_boot_type = FLASH_TYPE_NAND;
		else if (!strncmp(ctype, "/dev/mtdblock", sizeof("/dev/mtdblock")-1))
			flash_boot_type = FLASH_TYPE_NOR;
		else if (!strncmp(ctype, "/dev/mmcblk", sizeof("/dev/mmcblk")-1))
			flash_boot_type = FLASH_TYPE_EMMC3;
	}
	return flash_boot_type;
}

int get_flash_type(void)
{
	ALOGD("Boot type %d\n", flash_boot_type);
	return flash_boot_type;
}
