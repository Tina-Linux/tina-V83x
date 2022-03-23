
#include <tee_client_api.h>
#include <efuse_read_rotpk_ta.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "librotpk.h"

static const TEEC_UUID ta_UUID = ROTPK_DEMO_UUID;

void sunxi_dump(uint8_t *buf, int ttl_len)
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

int write_rotpk_hash(const char *buf)
{
	TEEC_Context ctx;
	TEEC_Result teecErr;
	TEEC_Session teecSession;
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

	printf("NA: init context\n");

	teecErr = TEEC_InitializeContext(NULL, &ctx);
	if (teecErr != TEEC_SUCCESS)
		goto failInit;

	printf("NA: open session\n");
	teecErr = TEEC_OpenSession(&ctx, &teecSession, &ta_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen;

	printf("NA: allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen1;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 128;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_q) != TEEC_SUCCESS)
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
	command			    = 0x221;

	printf("NA: invoke command\n");
	teecErr = TEEC_InvokeCommand(&teecSession, command, &operation, NULL);

	TEEC_ReleaseSharedMemory(&tee_params_q);
failOpen2:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen1:
	TEEC_CloseSession(&teecSession);
failOpen:
	TEEC_FinalizeContext(&ctx);
failInit:
	printf("NA: finish with %d\n", teecErr);

	if (teecErr == TEEC_SUCCESS)
		return 0;
	else
		return teecErr;
	return 0;
}

int read_rotpk_hash(char *buf)
{
	TEEC_Context ctx;
	TEEC_Result teecErr;
	TEEC_Session teecSession;
	TEEC_Operation operation;
	int command;

	printf("NA: read efuse hash\n");
	printf("NA: init context\n");

	teecErr = TEEC_InitializeContext(NULL, &ctx);
	if (teecErr != TEEC_SUCCESS)
		goto failInit;

	printf("NA: open session\n");
	teecErr = TEEC_OpenSession(&ctx, &teecSession, &ta_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen;

	printf("NA: allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen1;

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 128;
	tee_params_q.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_q) != TEEC_SUCCESS)
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

	command = 0x210;
	memcpy(tee_params_p.buffer, "rotpk", sizeof("rotpk"));

	printf("NA: invoke command\n");
	teecErr = TEEC_InvokeCommand(&teecSession, command, &operation, NULL);
	if (teecErr == 0) {
//		sunxi_dump((uint8_t *)tee_params_q.buffer, operation.params[2].value.a);
		memcpy(buf, tee_params_q.buffer, operation.params[2].value.a);
	}

	TEEC_ReleaseSharedMemory(&tee_params_q);
failOpen2:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen1:
	TEEC_CloseSession(&teecSession);
failOpen:
	TEEC_FinalizeContext(&ctx);
failInit:
	printf("NA: finish with %d\n", teecErr);

	return operation.params[2].value.a;
}


