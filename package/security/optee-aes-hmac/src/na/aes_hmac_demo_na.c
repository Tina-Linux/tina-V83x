#include <stdio.h>
#include <string.h>
#include "libaeshmac.h"

static void usage(char *app)
{
	printf("usage: \n");
	printf(" '%s r' - read rotpk from efuse\n", app);
	printf(" '%s w' - write default rotpk value to efuse\n", app);
	printf(" '%s w hex-string' - write <hex-string> to efuse\n", app);
	printf(" '%s demo' - an aes/hamc demo\n", app);
}

#define nv_read_key(...)
static void init_tee(void)
{
	char secret_uuid_base64_unique[96] = {
0xe3, 0xd5, 0x52, 0xff, 0x66, 0x21, 0x87, 0x53, 0xd3, 0xb3, 0x18, 0x81, 0x85, 0xeb, 0xea, 0x97,
0x9c, 0x02, 0xb2, 0x43, 0xec, 0xac, 0xaf, 0x5c, 0xa9, 0x23, 0x52, 0xe4, 0x5a, 0xa7, 0x0b, 0xf0,
0x7d, 0xdc, 0x44, 0x6a, 0xe6, 0xf2, 0x5d, 0x05, 0x48, 0x17, 0xb3, 0x89, 0x7b, 0xcc, 0xe9, 0x19,
0x86, 0xcb, 0xb0, 0x3f, 0x66, 0xa0, 0x8d, 0x93, 0x3e, 0xb6, 0x82, 0x02, 0xd7, 0xc2, 0xc8, 0x14,
0x7b, 0xab, 0xf9, 0x06, 0x9a, 0x82, 0x59, 0x2a, 0x15, 0xc2, 0xb3, 0x9e, 0x7b, 0xdf, 0x5c, 0x4a,
0x65, 0x8b, 0xaa, 0x67, 0x32, 0x69, 0x64, 0x2d, 0xa1, 0x4a, 0xab, 0x45, 0x80, 0x30, 0x8d, 0xd4,
		};

/*
	char secret_uuid_base64_unique[96] = {
		0x86, 0x40, 0xbe, 0x5c, 0x3a, 0x1f, 0xdc, 0xcb, 0xa0, 0x76,
		0xa4, 0xbc, 0x07, 0x17, 0x23, 0x23, 0x43, 0xc7, 0xaa, 0x7d,
		0x1b, 0x1f, 0x3c, 0x2a, 0x5a, 0x1e, 0x34, 0x90, 0xd4, 0xeb,
		0xbc, 0x6e, 0xb9, 0x12, 0x7e, 0x7d, 0x95, 0x6f, 0x98, 0xe5,
		0x5d, 0x29, 0xcc, 0x2e, 0x88, 0x0c, 0x20, 0xa1, 0x75, 0x09,
		0x5d, 0x60, 0xb1, 0x7f, 0x80, 0xc9, 0x54, 0xb5, 0x37, 0x16,
		0x6b, 0xdc, 0xbb, 0xf3, 0x8d, 0xa9, 0x05, 0x08, 0x1e, 0xa1,
		0xd6, 0x77, 0xcb, 0x78, 0xce, 0x8e, 0x3c, 0x32, 0x3b, 0xa6,
		0x4b, 0xc0, 0x69, 0x7e, 0xa8, 0x17, 0x8e, 0xe6, 0xba, 0x06,
		0x1d, 0xff, 0xa6, 0x4e, 0x38, 0x30,
	};
*/
	nv_read_key(secret_uuid_base64_unique, 96);

	set_info(0, secret_uuid_base64_unique, 96, 0);
}

#define OP_WRITE_ROTPK 0
#define OP_READ_ROTPK 1
#define OP_AES_HMAC_DEMO 2

int main(int argc, char **argv)
{
	int ret = 0;
	char *cmd = argv[1];
	int opflag = -1;

	// for efuse rotpk read & write
	char rotpk_tmp[] = "90fa80f15449512a8a042397066f5f780b6c8f892198e8d1baa42eb6ced176f3";
	char *rotpk_buf_in = NULL;
	char rotpk_buf_out[100] = {0};

	// for ui_encrypt_secretaes1_by_unique_key
	char raw_data[96]="VGhpcyBpcyBhbGkgdGVzdCBkZW1vISEh";
	char enc_data[96];
	char dec_data[96];
	int out_len;

	// for ui_gen_digest_inside
	char in_data[256];
	char out_data[256];

	if ((argc != 3) && (argc != 2)) {
		usage(argv[0]);
		return -1;
	}

	/* set opration flag */
	if (!strcmp(cmd, "w")) {
		opflag = OP_WRITE_ROTPK;
		if (argc == 3)
			rotpk_buf_in = argv[2];
		else {
			rotpk_buf_in = rotpk_tmp;
		}
	} else if (!strcmp(cmd, "r")) {
		opflag = OP_READ_ROTPK;
	} else if (!strcmp(cmd, "demo")) {
		opflag = OP_AES_HMAC_DEMO;
	} else {
		opflag = -1;
		usage(argv[0]);
		return -1;
	}

	ret = aes_hmac_init();
	if (ret) {
		printf("tee init failed!");
		return ret;
	}

	switch (opflag) {
	case OP_WRITE_ROTPK:
		printf("NA: rotpk_buf_in: %s, size: %d\n", rotpk_buf_in, strlen(rotpk_buf_in));

		ret = write_rotpk_hash(rotpk_buf_in);
		if (ret) {
			printf("NA: write rotpk hash error.\n");
			goto out;
		}
		break;
	case OP_READ_ROTPK:
		ret = read_rotpk_hash(rotpk_buf_out);
		if (ret) {
			printf("==NA: read rotpk hash.==\n");
			data_dump((uint8_t *)rotpk_buf_out, ret);
			printf("==NA: read rotpk hash end!==\n");
		}
		break;
	case OP_AES_HMAC_DEMO:
		// 0 : derive unique key from chipid
		ret = generate_unique_key();
		if (ret) {
			printf("generate unique key error!");
			goto out;
		}

		// 1 - encrypt/decrypte by unique key
		printf("raw_data:\n");
		data_dump((uint8_t *)raw_data, 96);

		ret = ui_encrypt_secretaes1_by_unique_key(0, raw_data, 96, enc_data, &out_len, 1);
		if (ret) {
			printf("NA: ui encrypt error.\n");
			goto out;
		}
		printf("enc_data:\n");
		data_dump((uint8_t *)enc_data, 96);

		ret = ui_encrypt_secretaes1_by_unique_key(0, enc_data, 96, dec_data, &out_len, 0);
		if (ret) {
			printf("NA: ui decrypt error.\n");
			goto out;
		}
		printf("dec_data:\n");
		data_dump((uint8_t *)dec_data, 96);

		if (memcmp(raw_data, dec_data, 96) == 0) {
			printf("data encrypt and decryt success!\n");
		}

		// 2 - generate key1 from nvram and unique_key
		init_tee();

		// 3 - generate key2 from key1, generate key3 from key2, do hmac with key3
		ret = ui_gen_digest_inside(0, in_data, 256, out_data, &out_len, 0);
		if (ret) {
			printf("NA: ui digest error.\n");
			goto out;
		}
		printf("input:\n");
		data_dump((uint8_t *)in_data, 256);
		printf("output:\n");
		data_dump((uint8_t *)out_data, out_len);

		ret = ui_gen_digest_inside(0, in_data, 128, out_data, &out_len, 0);
		if (ret) {
			printf("NA: ui digest error.\n");
			goto out;
		}
		printf("input:\n");
		data_dump((uint8_t *)in_data, 128);
		printf("output:\n");
		data_dump((uint8_t *)out_data, out_len);

		ret = ui_gen_digest_inside(0, in_data + 128, 128, out_data, &out_len, 0);
		if (ret) {
			printf("NA: ui digest error.\n");
			goto out;
		}
		printf("input:\n");
		data_dump((uint8_t *)(in_data + 128), 128);
		printf("output:\n");
		data_dump((uint8_t *)out_data, out_len);

		break;
	default:
		ret = -1;
		break;
	}

out:
	aes_hmac_finalize();

	return ret;
}
