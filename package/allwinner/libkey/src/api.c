#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "api.h"
#include "private.h"
#include "secure_storage.h"

#define SEC_BLK_SIZE                        (4096)

int key_data_write(const char *item_name, char *buffer, int length)
{
	int ret;
	char buffer_bak[SEC_BLK_SIZE];

	if (!item_name || !buffer) {
		ALOGE("[private_data_write] err: item_name or buffer is null");
		return -1;
	}

	if (length > SEC_BLK_SIZE) {
		ALOGE
		    ("[private_data_write] err: key length bigger than 4KB, %d",
		     length);
		return -1;
	}
	memset(buffer_bak, 0, SEC_BLK_SIZE);
	strncpy(buffer_bak, buffer, length);

	ret = private_bare_key_support();
	if (!ret) {		/*support */
		ret = sunxi_private_store_write(item_name, buffer_bak, length);
		if (ret) {
			ALOGE("[private_data_write] :Write private data fail\n");
		} else {
			ALOGD("(private_data_write]: write data to private done\n");
		}
	}

	ret = secure_storage_support();
	if (!ret) {		/*support */
		if (sunxi_secure_storage_init() < 0) {
			ALOGE("[private_data_write]:Secure storage init fail\n");
			return -1;
		}
		if (sunxi_secure_object_write(item_name, buffer_bak, length) < 0) {
			ALOGE("[private_data_write]: Secure storage write fail\n");
			sunxi_secure_storage_exit(0);
			return -1;
		}

		if (sunxi_secure_storage_exit(0) < 0) {
			ALOGE("[private_data_write]:Secure storage exit fail\n");
			return -1;
		}
		ALOGD("[private_data_write]: write data to secure storage done\n");
	}
	return 0;
}

/*
 * Read private area firstly.
 * If the private area read fail, then try the secure storage
 */
int key_data_read(const char *item_name, char *buffer, int buffer_len,
		  int *data_len)
{
	int ret;

	ret = secure_storage_support();
	if (!ret) {		/*support */
		if (sunxi_secure_storage_init() < 0) {
			ALOGE("[private_data_read]:Secure storage init fail\n");
			goto Next;
		}
		memset(buffer, 0x0, buffer_len);
		if (sunxi_secure_object_read
		    (item_name, buffer, buffer_len, data_len) < 0) {
			ALOGE("[private_data_read]: Secure storage read fail\n");
			sunxi_secure_storage_exit(0);
			goto Next;
		}

		if (sunxi_secure_storage_exit(0) < 0) {
			ALOGE("[private_data_read]:Secure storage exit fail\n");
			goto Next;
		}

		ALOGD("[private_data_read]: read data to secure storage done\n");
		return 0;
	}

Next:
	ret = private_bare_key_support();
	if (!ret) {		/*support */
		memset(buffer, 0x0, buffer_len);
		ret = sunxi_private_store_read(item_name, buffer, buffer_len, data_len);
		if (ret < 0) {
			ALOGE("[private_data_read] :read private data fail\n");
		}

		ALOGD("[private_data_read]: read data to private done\n");
	}
	return ret;
}
