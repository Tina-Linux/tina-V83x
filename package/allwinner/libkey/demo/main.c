#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "private.h"
#include "test.h"
#include "api.h"
#include "secure_storage.h"

#define TEST_SST_INIT		(0)
#define TEST_SST_READ		(1)
#define TEST_SST_WRITE		(2)
#define TEST_SST_VERIFY		(3)
#define TEST_PRI_INIT		(4)
#define TEST_PRI_READ		(5)
#define TEST_PRI_WRITE		(6)
#define TEST_PRI_VERIFY		(7)
#define TEST_SST_REBOOT		(8)
#define TEST_PRI_REBOOT		(9)

void help(void)
{
	printf("-w <name> <data>: write key to sst and private\n");
	printf("-r <name> <data>: read key from sst or private\n");
	printf("-t 0: write some key to secure storage\n");
	printf("-t 1: repetitve read + verify some key from secure storage\n");
	printf("-t 2: repetitve write + verify some key from secure storage\n");
	printf("-t 3: verify some key from secure storage\n");
	printf("-t 4: write some key to private\n");
	printf("-t 5: repetitve read + verify some key from private\n");
	printf("-t 6: repetitve write + verify some key from private\n");
	printf("-t 7: verify some key from private\n");
	printf("-l  : list\n");
	printf("-d  : printf hex\n");
	printf("-h  : help\n");
}

int main(int argc, char **argv)
{
	char buf[256];
	int ret, retLen;

	if (argc < 2) {
		goto error;
	}
	if (!strncmp(argv[1], "-r", 2)) {
		if (argv[2]) {	/* api read */
			ret = key_data_read(argv[2], buf, 256, &retLen);
			if (ret < 0) {
				printf("[api] Read %s fail\n", argv[2]);
			} else {
				printf("[api] %s = %s\n", argv[2], buf);
			}

			ret = secure_storage_support();
			if (!ret) {	/*support */
				if (sunxi_secure_storage_init() == 0) {
					memset(buf, 0x0, 256);
					ret =
					    sunxi_secure_object_read(argv[2],
								     buf, 256,
								     &retLen);
					if (ret < 0) {
						printf("[sst] Read %s fail\n",
						       argv[2]);
					} else {
						printf("[sst] %s = %s\n",
						       argv[2], buf);
					}

					if (sunxi_secure_storage_exit(0) < 0) {
						printf
						    ("Secure storage exit fail\n");
						return -1;
					}
				}
			}
			/* private read */
			ret = private_bare_key_support();
			if (!ret) {	/* support */
				memset(buf, 0x0, 256);
				ret =
				    sunxi_private_store_read(argv[2], buf, 256,
							     &retLen);
				if (ret < 0) {
					printf("[pri] Read %s fail\n", argv[2]);
				} else {
					printf("[pri] %s = %s\n", argv[2], buf);
				}
			}
		}
	} else if (!strncmp(argv[1], "-w", 2)) {
		if (argv[2] && argv[3]) {
			ret =
			    key_data_write(argv[2], argv[3],
					   strnlen(argv[3], 128));
			if (ret < 0) {
				printf("Write %s fail\n", argv[2]);
				goto error;
			}
		}
	} else if (!strncmp(argv[1], "-t", 2)) {
		int mode = 0;
		if (!argv[2]) {
			printf("error: <cmd> -t <num>\n");
			goto error;
		}

		mode = atoi(argv[2]);
		printf("test mode = %d\n", mode);
		switch (mode) {
		case TEST_SST_INIT:
			test_sst_init();
			break;
		case TEST_SST_READ:
			test_sst_read();
			break;
		case TEST_SST_WRITE:
			test_sst_write();
			break;
		case TEST_SST_VERIFY:
			test_sst_verify();
			break;
		case TEST_PRI_INIT:
			test_private_init();
			break;
		case TEST_PRI_READ:
			test_private_read();
			break;
		case TEST_PRI_WRITE:
			test_private_write();
			break;
		case TEST_PRI_VERIFY:
			test_private_verify();
			break;
		case TEST_SST_REBOOT:
			if (!test_sst_verify()) {
				system("reboot");
			}
			break;
		case TEST_PRI_REBOOT:
			if (!test_private_verify()) {
				system("reboot");
			}
			break;
		default:
			printf("error: no test item\n");
		}
		printf("test done\n");
	} else if (!strncmp(argv[1], "-e", 2)) {
		int mode = 0;
		if (!argv[2]) {
			printf("error: <cmd> -e <num>\n");
			goto error;
		}

		mode = atoi(argv[2]);
		printf("test mode = %d\n", mode);
		switch (mode) {
		case 0:
			if (!secure_storage_support())
				sunxi_secure_object_erase();
			break;
		case 1:
			if (!private_bare_key_support())
				sunxi_private_store_erase();
			break;
		default:
			;
		}
	} else if (!strncmp(argv[1], "-d", 2)) {
		printf("no support\n");
	} else if (!strncmp(argv[1], "-l", 2)) {
		if (!secure_storage_support())
			sunxi_secure_object_list();
		if (!private_bare_key_support())
			sunxi_private_store_list();
	} else if (!strncmp(argv[1], "-h", 2)) {
		help();
	}
	return 0;

error:
	printf("-----------------------\n");
	printf("----  Test fail  ------\n");
	printf("----  Test fail  ------\n");
	printf("----  Test fail  ------\n");
	printf("-----------------------\n");

	return -1;

}
