#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "private.h"
#include "secure_storage.h"
#include "test.h"
#include "log.h"

#define TEST_SST_INIT			(0)
#define TEST_SST_READ			(1)
#define TEST_SST_WRITE		(2)
#define TEST_SST_VERIFY		(3)
#define TEST_PRI_INIT			(4)
#define TEST_PRI_READ			(5)
#define TEST_PRI_WRITE		(6)
#define TEST_PRI_VERIFY		(7)
#define TEST_SST_REBOOT		(8)
#define TEST_PRI_REBOOT		(9)

unsigned int TEST_TIMES;

struct test_data {
	char name[64];
	char key[128];
};

struct test_data my_key[12] = {
	{"SN", "BE5G31404000000110"},
	{"PN", "V1GW0101GW0100"},
	{"IMEI", "004201000902E5G11404404000000110"},
	{"TID", "010121011000001"},
	{"EMAC", "0C:C6:55:04:86:FF"},
	{"WMAC", "0C:C6:55:04:86:9B"},
	{"SN1", "BE5G31404000000110"},
	{"PN1", "V1GW0101GW0100"},
	{"IMEI1", "004201000902E5G11404404000000110"},
	{"TID1", "010121011000001"},
	{"EMAC1", "0C:C6:55:04:86:FF"},
	{"WMAC1", "0C:C6:55:04:86:9B"}
};

int test_sst_init(void)
{
	int ret;
	int i;

	ALOGD("(private_data_write]: write data to private done\n");
	ret = secure_storage_support();
	if (!ret) {		/*support */
		if (sunxi_secure_storage_init() < 0) {
			ALOGE
			    ("[private_data_write]:Secure storage init fail\n");
			return -1;
		}
		for (i = 0; i < sizeof(my_key) / sizeof(struct test_data); i++) {
			if (sunxi_secure_object_write
			    (my_key[i].name, my_key[i].key,
			     sizeof(my_key[i].key)) < 0) {
				ALOGE
				    ("[private_data_write]: Secure storage write fail\n");
				sunxi_secure_storage_exit(0);
				return -1;
			}
		}

		if (sunxi_secure_storage_exit(1) < 0) {
			ALOGE
			    ("[private_data_write]:Secure storage exit fail\n");
			return -1;
		}
		ALOGD
		    ("(private_data_write]: write data to secure storage done\n");
		return 0;
	}

	ALOGE("error: test_sst_init");
	return 1;
}

int test_sst_verify(void)
{
	int i;
	char key_buf[128];
	int ret;
	int key_length;

	memset(key_buf, 0, sizeof(key_buf));

	ret = secure_storage_support();
	if (!ret) {		/*support */
		if (sunxi_secure_storage_init() < 0) {
			ALOGE("[private_data_read]:Secure storage init fail\n");
			goto error_test_sst_verify;
		}

		for (i = 0; i < sizeof(my_key) / sizeof(struct test_data); i++) {
			if (sunxi_secure_object_read
			    (my_key[i].name, key_buf, sizeof(key_buf),
			     &key_length) < 0) {
				ALOGE
				    ("[private_data_read]: Secure storage read fail\n");
				sunxi_secure_storage_exit(0);
				goto error_test_sst_verify;
			}
			if (strncmp
			    (my_key[i].key, key_buf, strlen(my_key[i].key))) {
				ALOGD("error: key name: %s\n", my_key[i].name);
				ALOGD("orig key: %s\n", my_key[i].key);
				ALOGD("sst key: %s\n", key_buf);
				goto error_test_sst_verify;
			}
			ALOGD("verify ok: %s = %s\n", my_key[i].name, key_buf);
		}
		sunxi_secure_storage_exit(0);
		return 0;
	}

error_test_sst_verify:
	sunxi_secure_storage_exit(0);
	ALOGE("error: test_sst_verify");
	return 1;
}

int test_sst_read(void)
{
	int i;
	int ret = -1;

	TEST_TIMES = 0;

#if 0
	test_sst_init();
#endif

	while (1) {
		ret = test_sst_verify();
		if (ret) {
			goto error_test_sst_read;
		}
		ALOGD("test times: %d	result: ok\n", TEST_TIMES);
		ALOGD("\n");
		TEST_TIMES++;
		sleep(1);
	}

error_test_sst_read:
	ALOGD("test times: %d	result: fail\n", TEST_TIMES);
	ALOGD("\n");
	return 1;
}

int test_sst_write(void)
{
	int ret;
	TEST_TIMES = 0;

	while (1) {
		ret = test_sst_init();
		if (!ret) {
			ALOGD("test times: %d	result: ok\n", TEST_TIMES);
			ALOGD("\n");
		} else {
			ALOGD("test times: %d	result: fail\n", TEST_TIMES);
			ALOGD("\n");
			goto error_test_sst_write;
		}
		{
			ret = test_sst_verify();
			if (ret) {
				goto error_test_sst_write;
			}
		}
		ALOGD("\n");
		TEST_TIMES++;
		sleep(1);
	}
	return 0;

error_test_sst_write:
	return 1;
}

int test_private_init(void)
{
	int i, ret;
	int key_length;

	ALOGD("private write start!\n");
	for (i = 0; i < sizeof(my_key) / sizeof(struct test_data); i++) {
		ret =
		    sunxi_private_store_write(my_key[i].name, my_key[i].key,
					      sizeof(my_key[i].key));
		if (ret)
			return 1;
	}

	return 0;
}

int test_private_verify(void)
{
	int i;
	char key_buf[128];
	int ret;
	int key_length;

	memset(key_buf, 0, sizeof(key_buf));

	for (i = 0; i < sizeof(my_key) / sizeof(struct test_data); i++) {
		ret =
		    sunxi_private_store_read(my_key[i].name, key_buf,
					     sizeof(key_buf), &key_length);
		if (ret) {
			ALOGD("error: read %s for private\n", my_key[i].name);
			goto error_test_private_verify;
		}
		if (strncmp(my_key[i].key, key_buf, strlen(my_key[i].key))) {
			ALOGD("error: key name: %s\n", my_key[i].name);
			ALOGD("orig key: %s\n", my_key[i].key);
			ALOGD("private key: %s\n", key_buf);
			goto error_test_private_verify;
		}
		ALOGD("verify ok: %s = %s\n", my_key[i].name, key_buf);
	}

	return 0;

error_test_private_verify:
	return 1;
}

int test_private_read(void)
{
	int i;
	int ret = -1;

	TEST_TIMES = 0;

#if 0
	test_private_init();
#endif

	while (1) {
		ret = test_private_verify();
		if (ret) {
			goto error_test_private_read;
		}
		ALOGD("test times: %d	result: ok\n", TEST_TIMES);
		ALOGD("\n");
		TEST_TIMES++;
		sleep(1);
	}

error_test_private_read:
	ALOGD("test times: %d	result: fail\n", TEST_TIMES);
	ALOGD("\n");
	return 1;
}

int test_private_write(void)
{
	int ret;
	TEST_TIMES = 0;
	while (1) {
		ret = test_private_init();
		if (!ret) {
			ALOGD("test times: %d	result: ok\n", TEST_TIMES);
		} else {
			ALOGD("test times: %d	result: fail\n", TEST_TIMES);
			goto error_test_private_write;
		}
		{
			ret = test_private_verify();
			if (ret) {
				goto error_test_private_write;
			}
		}
		ALOGD("\n");
		TEST_TIMES++;
		sleep(1);
	}
	return 0;

error_test_private_write:
	return 1;
}
