#include <stdio.h>
#include <string.h>
#include "librotpk.h"

static void usage(char *app)
{
	printf("usage: %s [options] [hex-string]\n", app);
	printf(" [options]:\n");
	printf(" \tr\t read rotpk from efuse.\n");
	printf(" \tw\t write rotpk to efuse.\n");
	printf(" hex-string: input hex-string to burn to efuse.\n");
}


int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		usage(argv[0]);
		return -1;
	}

	char buf_tmp[] = "90fa80f15449512a8a042397066f5f780b6c8f892198e8d1baa42eb6ced176f3";
	char *cmd = argv[1];
	char *buf_in = NULL;
	char buf_out[100] = {0};
	int ret = 0;

	if (argc == 3)
		buf_in = argv[2];
	else {
		buf_in = buf_tmp;
	}

	if (strcmp(cmd,"w") == 0) {
		printf("NA: buf_in: %s, size: %d\n", buf_in, strlen(buf_in));

		ret = write_rotpk_hash(buf_in);
		if (ret) {
			printf("NA: write rotpk hash error.\n");
			return -1;
		}
	} else if (strcmp(cmd,"r") == 0) {
		ret = read_rotpk_hash(buf_out);
		if (ret) {
			printf("==NA: read rotpk hash.==\n");
			sunxi_dump((uint8_t *)buf_out, ret);
			printf("==NA: read rotpk hash end!==\n");
		}
	} else
		usage(argv[0]);

	return 0;
}
