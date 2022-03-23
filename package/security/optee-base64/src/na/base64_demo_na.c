
#include <tee_client_api.h>

#include <stdio.h>
#include <string.h>

#include <base64_demo_ta.h>

//UUID for helloworld ta
static const TEEC_UUID helloworld_UUID = BASE64_DEMO_TA_UUID;

const char help_info[] = { "usage:\n"
			   "\t-e in(base16)\n"
			   "\t\texample: -e ab23\n"
			   "\t-d in(str) exp:\n"
			   "\t\texample: -d a8==\n" };

int str_to_hex(uint8_t *str, uint8_t *out_buf);
int str_to_hex(uint8_t *str, uint8_t *out_buf)
{
	uint8_t tmpHex;
	uint8_t ret = 0;
	while (*str) {
		tmpHex = 0;
		if ((*str >= '0') && (*str <= '9')) {
			tmpHex = *str - '0';
		} else if ((*str >= 'a') && (*str <= 'f')) {
			tmpHex = *str - 'a' + 10;
		} else {
			return -1;
		}
		tmpHex <<= 4;
		str++;
		ret++;
		if (*str == 0) {
			*out_buf = tmpHex;
			break;
		} else if ((*str >= '0') && (*str <= '9')) {
			tmpHex += *str - '0';
		} else if ((*str >= 'a') && (*str <= 'f')) {
			tmpHex += (*str - 'a' + 10);
		} else {
			return -1;
		}
		*out_buf = tmpHex;
		out_buf++;
		str++;
	}
	return ret;
}

void dump(uint8_t *buf, int ttl_len);
void dump(uint8_t *buf, int ttl_len)
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
int main(int argc, char **argv)
{
	TEEC_Context ctx;
	TEEC_Result teecErr;
	TEEC_Session teecSession;
	TEEC_Operation operation;
	uint8_t in_buf[200];
	int expected_len = 0;
	int input_len    = 0;
	int runCMD;
	if (argc != 3) {
		printf("%s", help_info);
		return -1;
	} else {
		if (!strcmp("-e", argv[1])) {
			expected_len = strlen(argv[2]);
			/*2char 1byte, 3inbyte 4outbyte*/
			if (200 < expected_len * 4 / 6) {
				printf("input too long\n");
				return -1;
			}
			input_len = str_to_hex((uint8_t *)argv[2], in_buf);
			if (input_len <= 0) {
				printf("illegal char in input\n");
				return -1;
			}
			printf("input bytes:\n");
			dump(in_buf, input_len);
			runCMD = CMD_ENCODE;
		} else if (!strcmp("-d", argv[1])) {
			input_len = strlen(argv[2]);
			/*4inbyte 3outbyte*/
			if (200 < input_len) {
				printf("input too long\n");
				return -1;
			}
			strcpy((char *)in_buf, argv[2]);
			runCMD = CMD_DECODE;
		} else {
			printf("%s", help_info);
			return -1;
		}
	}

	teecErr = TEEC_InitializeContext(NULL, &ctx);
	if (teecErr != TEEC_SUCCESS)
		goto failInit;

	printf("NA:open session\n");
	teecErr = TEEC_OpenSession(&ctx, &teecSession, &helloworld_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen;

	printf("NA:allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = input_len;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen1;
	memcpy(tee_params_p.buffer, in_buf, input_len);

	TEEC_SharedMemory tee_params_q;
	tee_params_q.size  = 200;
	tee_params_q.flags = TEEC_MEM_OUTPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_q) != TEEC_SUCCESS)
		goto failShmp;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes =
		TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
				 TEEC_VALUE_OUTPUT, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	operation.params[1].memref.parent = &tee_params_q;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size   = 0;

	operation.params[2].value.a = 0;

	teecErr = TEEC_InvokeCommand(&teecSession, runCMD, &operation, NULL);
	if (runCMD == CMD_DECODE) {
		printf("decode result:\n");
		dump((uint8_t *)tee_params_q.buffer,
		     operation.params[2].value.a);
	} else if (runCMD == CMD_ENCODE) {
		printf("encode result:\n%s\n", (char *)tee_params_q.buffer);
	}
	TEEC_ReleaseSharedMemory(&tee_params_q);
failShmp:
	TEEC_ReleaseSharedMemory(&tee_params_p);
failOpen1:
	TEEC_CloseSession(&teecSession);
failOpen:
	TEEC_FinalizeContext(&ctx);
failInit:
	printf("NA:finish with %d\n", teecErr);
	if (teecErr == TEEC_SUCCESS)
		return 0;
	else
		return -1;
}
