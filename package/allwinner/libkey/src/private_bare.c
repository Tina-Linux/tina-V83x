#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "log.h"

#define MAGIC                       ("sunxi")
#define PART_NAME               ("private")
#define PRIVATE_SIZE        (16 * 1024 * 1024)
#define USER_DATA_MAXSIZE               (8 * 1024)
#define USER_DATA_PARAMETER_MAX_COUNT   (30)

#define NANDI_PATH          ("/dev/block/by-name/private")
#define NANDI_OLD_PATH      ("/dev/by-name/private")

#define NAME_SIZE       (32)
#define VALUE_SIZE  (128)

typedef struct {
	char magic_name[8];
	int count;
	int reserved[3];
} USER_DATA_HEAR;

typedef struct {
	char name[NAME_SIZE];
	char value[VALUE_SIZE];
	int valid;
	int reserved[3];
} USER_PRIVATE_DATA;

/*
 * Write Content to dev/block/by-name/private partition
 */
static int private_bare_write(char *buf)
{

	int private_fd, n;
	private_fd = open(NANDI_PATH, O_WRONLY);
	if (private_fd < 0) {
		ALOGD("fail open %s\n", NANDI_PATH);
		ALOGD("try to open %s\n", NANDI_OLD_PATH);
		private_fd = open(NANDI_OLD_PATH, O_WRONLY);
		if (private_fd < 0) {
			ALOGE("Open nandi Error: %s\n", strerror(errno));
			return -1;
		}
	}
#ifdef CONF_SUNXI_PRIVATE_BARE_TAIL_8K
	ALOGD("private bare tail 8K");
	lseek(private_fd, PRIVATE_SIZE - USER_DATA_MAXSIZE, SEEK_SET);
#else
	ALOGD("private bare first 8K");
	lseek(private_fd, 0, SEEK_SET);
#endif
	n = write(private_fd, buf, USER_DATA_MAXSIZE);
	if (n < 0) {
		ALOGE("Read nandi Error: %s\n", strerror(errno));
		close(private_fd);
		return -1;
	}
	fsync(private_fd);
	close(private_fd);
	return 0;
}

/*
 * Read Content from /dev/block/nandi partition,
 * Store content to buf
 */
static int private_bare_read(char *buf)
{
	int private_fd, n;
	private_fd = open(NANDI_PATH, O_RDONLY);
	if (private_fd < 0) {
		ALOGD("fail open %s\n", NANDI_PATH);
		ALOGD("try to open %s\n", NANDI_OLD_PATH);
		private_fd = open(NANDI_OLD_PATH, O_RDONLY);
		if (private_fd < 0) {
			ALOGE("Open nandi Error: %s\n", strerror(errno));
			return -1;
		}
	}
#ifdef CONF_SUNXI_PRIVATE_BARE_TAIL_8K
	ALOGD("private bare tail 8K");
	lseek(private_fd, PRIVATE_SIZE - USER_DATA_MAXSIZE, SEEK_SET);
#else
	ALOGD("private bare first 8K");
	lseek(private_fd, 0, SEEK_SET);
#endif
	n = read(private_fd, buf, USER_DATA_MAXSIZE);
	if (n < 0) {
		ALOGE("Read nandi Error: %s\n", strerror(errno));
		close(private_fd);
		return -1;
	}
	close(private_fd);
	return 0;
}

/*
***************************************************************************************************
*
*
*
*
*
*               修改mac地址、sn
*
*
*
*
*
***************************************************************************************************
*/
static int modify_private_bare_parameter(const char *private_buf,
					 const char *name, char *value)
{
	int j;
	char *user_data_buffer = NULL;
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;
	char cmp_data_name[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (!private_buf || !name || !value) {
		ALOGE("error: the private_buf or name or value is null\n");
		return -1;
	}

	user_data_buffer = (char *)private_buf;
	user_data_head = (USER_DATA_HEAR *) user_data_buffer;
	user_data_p = (USER_PRIVATE_DATA *) (user_data_buffer + sizeof(USER_DATA_HEAR));
	if (strncmp(user_data_head->magic_name, MAGIC, 5)) {
		memset(user_data_buffer, 0xff, USER_DATA_MAXSIZE);
		strcpy(user_data_head->magic_name, MAGIC);
		user_data_head->count = 0;
		ALOGD("init the (user) private space\n");
	}

	if (strncmp(cmp_data_name, (char *)user_data_p, 8)) {
		if (user_data_head->count > 0) {
			for (j = 0;
			     j < user_data_head->count
			     && j < USER_DATA_PARAMETER_MAX_COUNT; j++) {
				if (!strcmp(name, user_data_p->name)) {
					strcpy(user_data_p->value, value);
					user_data_p->valid = 1;
					ALOGD("Saving Environment to (1)\n");
					break;
				}
				user_data_p++;
			}
			if (j == user_data_head->count) {
				strcpy(user_data_p->name, name);
				strcpy(user_data_p->value, value);
				user_data_p->valid = 1;
				user_data_head->count++;
				ALOGD("Saving Environment to (2)\n");
			}
		} else {
			strcpy(user_data_p->name, name);
			strcpy(user_data_p->value, value);
			user_data_p->valid = 1;
			user_data_head->count++;
			ALOGD("Saving Environment to (3)\n");
		}
	} else {
		user_data_head->count = 1;
		user_data_p =
		    (USER_PRIVATE_DATA *) (user_data_buffer +
					   sizeof(USER_DATA_HEAR));
		strcpy(user_data_p->name, name);
		strcpy(user_data_p->value, value);
		user_data_p->valid = 1;
		ALOGD("Saving Environment to (3)\n");
	}
	return 0;
}

/*
 * priavate data list
 */
int sunxi_private_store_list(void)
{
	int j = 0;
	char *pri_buf;
	char name[NAME_SIZE];
	char value[VALUE_SIZE];
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;

	pri_buf = (void *)malloc(USER_DATA_MAXSIZE);
	if (NULL == pri_buf) {
		ALOGE("alloc private buffer fail\n");
		return -1;
	}
	memset(pri_buf, 0xff, USER_DATA_MAXSIZE);
	if (private_bare_read(pri_buf) != 0) {
		free(pri_buf);
		ALOGE("read fail");
		return -1;
	}
	printf("\n[private space]\n");

	user_data_head = (USER_DATA_HEAR *) pri_buf;
	user_data_p = (USER_PRIVATE_DATA *) (pri_buf + sizeof(USER_DATA_HEAR));
	if (strncmp(user_data_head->magic_name, MAGIC, 5)) {
		printf("no key!!\n");
		free(pri_buf);
		return -1;
	}
	if (user_data_head->count > 0) {
		for (j = 0;
		     j < user_data_head->count
		     && j < USER_DATA_PARAMETER_MAX_COUNT; j++) {
			memset(name, 0, sizeof(name));
			memset(value, 0, sizeof(value));
			strncpy(name, user_data_p->name, sizeof(name) - 1);
			strncpy(value, user_data_p->value, sizeof(value) - 1);
			printf("%d: %s = %s\n", j + 1, name, value);
			user_data_p++;
		}
	}
	if (!j)
		printf("no key!!\n");

	free(pri_buf);
	return 0;
}

/*
***************************************************************************************************
*
*
*
*
*
*
*   查看 mac、sn
*
*
*
*
***************************************************************************************************
*/
static int check_private_bare_parameter(const char *private_buf,
					const char *name, char *value)
{
	int j;
	char *user_data_buffer = NULL;
	USER_PRIVATE_DATA *user_data_p = NULL;
	USER_DATA_HEAR *user_data_head = NULL;
	char cmp_data_name[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (!private_buf || !name) {
		ALOGE("error: the private_buf or name is null\n");
		return 0;
	}

	user_data_buffer = (char *)private_buf;
	user_data_head = (USER_DATA_HEAR *) user_data_buffer;
	user_data_p =
	    (USER_PRIVATE_DATA *) (user_data_buffer + sizeof(USER_DATA_HEAR));
	if (strncmp(user_data_head->magic_name, MAGIC, 5)) {
		ALOGE("the user_data is space\n");
		return -1;
	}

	if (strncmp(cmp_data_name, (char *)user_data_p, 8)) {
		if (user_data_head->count > 0) {
			for (j = 0;
			     j < user_data_head->count
			     && j < USER_DATA_PARAMETER_MAX_COUNT; j++) {
				if (!strcmp(name, user_data_p->name)) {
					strcpy(value, user_data_p->value);
					return 0;
				}
				user_data_p++;
			}
		}
	}
	ALOGD("not find key for private");
	return -1;
}

/*
 * Read priavate data from store
 */
int sunxi_private_store_read(const char *item_name, char *buffer,
			     int buffer_len, int *data_len)
{
	int i;
	char *pri_buf;

	ALOGD("private read name = %s", item_name);
	if (!buffer || buffer_len < VALUE_SIZE) {
		ALOGE("input buffer invalid\n");
		return -1;
	}
	pri_buf = (void *)malloc(USER_DATA_MAXSIZE);
	if (NULL == pri_buf) {
		ALOGE("alloc private buffer fail\n");
		return -1;
	}
	memset(pri_buf, 0xff, USER_DATA_MAXSIZE);
	if (private_bare_read(pri_buf) != 0) {
		free(pri_buf);
		ALOGE("read fail");
		return -1;
	}

	if (check_private_bare_parameter(pri_buf, item_name, buffer) == 0) {
		ALOGD("%s = %s\n", item_name, buffer);
		*data_len = strnlen(buffer, VALUE_SIZE);
		free(pri_buf);
		return 0;
	}
	free(pri_buf);
	ALOGE("read _fail2");
	return -1;
}

/*
 * Write private data to store
 */
int sunxi_private_store_write(const char *item_name, char *buffer, int length)
{
	char *pri_buf;

	ALOGD("write %s=%s", item_name, buffer);

	pri_buf = (void *)malloc(USER_DATA_MAXSIZE);
	if (NULL == pri_buf) {
		ALOGE("alloc private buffer fail\n");
		return -1;

	}
	memset(pri_buf, 0xff, USER_DATA_MAXSIZE);
	if (private_bare_read(pri_buf) != 0) {
		free(pri_buf);
		ALOGE("read fail");
		return -1;
	}
	if (length > VALUE_SIZE) {
		free(pri_buf);
		ALOGE("sunxi_private_store_write: write bytes %d > 128bytes",
		      length);
		return -1;
	}
	if (modify_private_bare_parameter(pri_buf, item_name, buffer) != 0) {
		free(pri_buf);
		ALOGE("write fail");
		return -1;
	}

	if (private_bare_write(pri_buf) != 0) {
		ALOGE("write  fail1!");
		free(pri_buf);
		return -1;
	}

	ALOGD("write success!");
	free(pri_buf);
	return 0;
}

/*
 * erase private data to store
 */
int sunxi_private_store_erase(void)
{
	char *pri_buf;

	ALOGD("start erase private");

	pri_buf = (void *)malloc(USER_DATA_MAXSIZE);
	if (NULL == pri_buf) {
		ALOGE("alloc private buffer fail\n");
		return -1;

	}
	memset(pri_buf, 0xff, USER_DATA_MAXSIZE);

	if (private_bare_write(pri_buf) != 0) {
		ALOGE("write  fail1!");
		free(pri_buf);
		return -1;
	}

	ALOGD("erase success!");
	free(pri_buf);
	return 0;
}

int private_bare_key_support(void)
{
#ifdef CONF_SUPPORT_PRIVATE_BARE_KEY
#ifdef CONF_SUNXI_PRIVATE_BARE_TAIL_8K
	ALOGD("sorry,private bare tail key not supported Now");
	ALOGD("please,use private bare first key");
	return -1;
#endif
	int private_fd;
	private_fd = open(NANDI_PATH, O_WRONLY);
	if (private_fd < 0) {
		ALOGD("fail open %s\n", NANDI_PATH);
		ALOGD("try to open %s\n", NANDI_OLD_PATH);
		private_fd = open(NANDI_OLD_PATH, O_WRONLY);
		if (private_fd < 0) {
			ALOGE("Open nandi Error: %s\n", strerror(errno));
			return -1;
		}
	}

	close(private_fd);
	ALOGD("private bare key supported\n");
	return 0;
#else
	ALOGD("private bare key not supported\n");
	return -1;
#endif
}
