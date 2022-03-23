#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include <tee_client_api.h>
#include <aes_hmac_demo_ta.h>

#include "libaeshmac.h"

void data_dump(uint8_t *buf, int ttl_len)
{
	int len;
	for (len = 0; len < ttl_len; len++) {
		printf("0x%02x ", ((char *)buf)[len]);
		if (len % 8 == 7) {
			printf("\n");
		}
	}
	printf("\n");
}

static int hexstr_to_byte(const char* source, uint8_t* dest, int sourceLen)
{
	short i;
	uint8_t highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2) {
		highByte = toupper(source[i]);
		lowByte  = toupper(source[i + 1]);

		if (highByte < '0' || (highByte > '9' && highByte < 'A' ) || highByte > 'F') {
			printf("input buf[%d] is %c, not in 0123456789ABCDEF\n", i, source[i]);
			return -1;
		}

		if (lowByte < '0' || (lowByte > '9' && lowByte < 'A' ) || lowByte > 'F') {
			printf("input buf[%d] is %c, not in 0123456789ABCDEF\n", i+1, source[i+1]);
			return -1;
		}

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;


		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return 0;
}

static const TEEC_UUID ta_UUID = AES_HMAC_DEMO_UUID;
TEEC_Context aes_hmac_demo_ctx;
TEEC_Session aes_hmac_demo_session;

int aes_hmac_init(void)
{
	TEEC_Result teecErr;

	printf("NA: init context\n");
	teecErr = TEEC_InitializeContext(NULL, &aes_hmac_demo_ctx);
	if (teecErr != TEEC_SUCCESS)
		return -1;

	printf("NA: open session\n");
	teecErr = TEEC_OpenSession(&aes_hmac_demo_ctx, &aes_hmac_demo_session, &ta_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		TEEC_FinalizeContext(&aes_hmac_demo_ctx);

	return (teecErr == TEEC_SUCCESS) ? 0 : -1;
}

void aes_hmac_finalize(void)
{
	printf("NA: close session\n");
	TEEC_CloseSession(&aes_hmac_demo_session);

	printf("NA: finalize context\n");
	TEEC_FinalizeContext(&aes_hmac_demo_ctx);
}

int write_rotpk_hash(const char *buf)
{
	TEEC_Result teecErr;
	TEEC_Operation operation;
	int command;

	uint8_t rotpk_hash[32];
	int ret = 0;

	if (strlen(buf) < 64) {
		printf("NA: ERROR: input buf size is %d, should bigger than 64 bytes\n", strlen(buf));
		return -1;
	}

	ret = hexstr_to_byte(buf, rotpk_hash, 64);
	if (ret == -1) {
		printf("ERROR: input buf is hexString\n");
		return -1;
	}

	printf("NA: write efuse hash\n");

	printf("NA: allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 128;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_q) != TEEC_SUCCESS)
		goto failOpen2;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
				 TEEC_VALUE_INOUT, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[1].memref.parent = &tee_params_q;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size   = 0;

	memcpy(tee_params_p.buffer, "rotpk", sizeof("rotpk"));
	memcpy(tee_params_q.buffer, rotpk_hash, sizeof(rotpk_hash));
	operation.params[2].value.a = sizeof(rotpk_hash);
	command			    = AES_HMAC_CMD_ROTPK_WRITE;

	printf("NA: invoke command\n");
	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);

	TEEC_ReleaseSharedMemory(&tee_params_q);
failOpen2:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen:
	printf("NA: %s finish with %d\n", __func__, teecErr);

	if (teecErr == TEEC_SUCCESS)
		return 0;
	else
		return teecErr;
	return 0;
}

int read_rotpk_hash(char *buf)
{
	TEEC_Result teecErr;
	TEEC_Operation operation;
	int command;

	printf("NA: read efuse hash\n");

	printf("NA: allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 128;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_q) != TEEC_SUCCESS)
		goto failOpen2;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
				 TEEC_VALUE_INOUT, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[1].memref.parent = &tee_params_q;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size   = 0;

	command = AES_HMAC_CMD_ROTPK_READ;
	memcpy(tee_params_p.buffer, "rotpk", sizeof("rotpk"));

	printf("NA: invoke command\n");
	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);
	if (teecErr == 0) {
//		sunxi_dump((uint8_t *)tee_params_q.buffer, operation.params[2].value.a);
		memcpy(buf, tee_params_q.buffer, operation.params[2].value.a);
	}

	TEEC_ReleaseSharedMemory(&tee_params_q);
failOpen2:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen:
	printf("NA: %s finish with %d\n", __func__, teecErr);

	return operation.params[2].value.a;
}

int generate_unique_key(void)
{
	TEEC_Result teecErr;
	TEEC_Operation operation;
	int command;

	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p);
	if (teecErr != TEEC_SUCCESS)
		return teecErr;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	command = AES_HMAC_CMD_DRIVE_UNIQUE_KEY;

	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);

	TEEC_ReleaseSharedMemory(&tee_params_p);

	printf("NA: %s finish with %d\n", __func__, teecErr);

	return teecErr;
}

int ui_encrypt_secretaes1_by_unique_key(pid_t pid, char* data_in,
		int data_in_len, char *data_out, int *data_out_len, int encrypt)
{
	TEEC_Result teecErr = -1;
	TEEC_Operation operation;
	int command;

	(void)pid;

	if (encrypt != 0 && encrypt != 1)
		return -1;

	printf("NA:allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = data_in_len;
	tee_params_p.flags = TEEC_MEM_INPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen1;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = data_in_len;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_q);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen2;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
				 TEEC_VALUE_INOUT, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[1].memref.parent = &tee_params_q;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size   = 0;

	if (encrypt == 1)
		command = AES_HMAC_CMD_UI_ENCRYPT;
	else
		command = AES_HMAC_CMD_UI_DECRYPT;

	memcpy(tee_params_p.buffer, data_in, data_in_len);
	operation.params[2].value.a = data_in_len;
	printf("NA:invoke command\n");
	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);
	*data_out_len = operation.params[2].value.b;
	memcpy(data_out, tee_params_q.buffer, *data_out_len);

	TEEC_ReleaseSharedMemory(&tee_params_q);
failOpen2:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen1:
	printf("NA: %s finish with %d\n", __func__, teecErr);
	if (teecErr == TEEC_SUCCESS)
		return 0;
	else
		return teecErr;
}

int ui_gen_digest_inside(pid_t pid,
				char *data_in, int data_in_len, char *data_out,
				int *data_out_len,
				int digest_type)
{
	TEEC_Result teecErr;
	TEEC_Operation operation;
	int command;

	(void)pid;
	(void)digest_type;

	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = data_in_len;
	tee_params_p.flags = TEEC_MEM_INPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p);
	if (teecErr != TEEC_SUCCESS)
		goto failInBuf;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 64;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_q);
	if (teecErr != TEEC_SUCCESS)
		goto failOutBuf;

	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
				 TEEC_VALUE_INOUT, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[1].memref.parent = &tee_params_q;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size   = 0;

	operation.params[2].value.a = data_in_len;
	memcpy(tee_params_p.buffer, data_in, data_in_len);

	command = AES_HMAC_CMD_UI_DIGEST;
	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);
	*data_out_len = operation.params[2].value.b;
	memcpy(data_out, tee_params_q.buffer, *data_out_len);

	TEEC_ReleaseSharedMemory(&tee_params_q);

failOutBuf:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failInBuf:
	return teecErr;
}

int set_info(pid_t pid, char *in_data,
		    int in_len, int info_type)
{
	TEEC_Result teecErr;
	TEEC_Operation operation;
	int command;

	(void)pid;
	(void)info_type;

	printf("NA:allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = in_len;
	tee_params_p.flags = TEEC_MEM_INPUT;
	teecErr = TEEC_AllocateSharedMemory(&aes_hmac_demo_ctx, &tee_params_p);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen1;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[2].value.a = in_len;

	memcpy(tee_params_p.buffer, in_data, in_len);
	printf("NA:invoke command\n");
	command = AES_HMAC_CMD_SET_INFO;
	teecErr = TEEC_InvokeCommand(&aes_hmac_demo_session, command, &operation, NULL);

	TEEC_ReleaseSharedMemory(&tee_params_p);

failOpen1:
	printf("NA: %s finish with %d\n", __func__, teecErr);
	return teecErr;
}


