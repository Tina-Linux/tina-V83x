
#include <tee_client_api.h>
#include <efuse_read_demo_ta.h>
#include <stdio.h>
#include <string.h>

static const TEEC_UUID ta_UUID = API_DEMO_UUID;

int main(int argc, char **argv)
{
	TEEC_Context ctx;
	TEEC_Result teecErr;
	TEEC_Session teecSession;
	TEEC_Operation operation;
	int command;

	printf("NA:init context\n");

	teecErr = TEEC_InitializeContext(NULL, &ctx);
	if (teecErr != TEEC_SUCCESS)
		goto failInit;

	printf("NA:open session\n");
	teecErr = TEEC_OpenSession(&ctx, &teecSession, &ta_UUID,
				   TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
	if (teecErr != TEEC_SUCCESS)
		goto failOpen;

	printf("NA:allocate memory\n");
	TEEC_SharedMemory tee_params_p;
	tee_params_p.size  = 20;
	tee_params_p.flags = TEEC_MEM_INPUT;
	if (TEEC_AllocateSharedMemory(&ctx, &tee_params_p) != TEEC_SUCCESS)
		goto failOpen1;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);
	operation.started = 1;

	operation.params[0].memref.parent = &tee_params_p;
	operation.params[0].memref.offset = 0;
	operation.params[0].memref.size   = 0;

	command = 0x210;
	if (argc == 2) {
		memcpy(tee_params_p.buffer, argv[1], 10);
	}else if(argc ==3){
		if(strcmp("-ext_key",argv[1])==0){
			memcpy(tee_params_p.buffer, argv[2], 10);
			command = 0x220;
		}
	}
	printf("NA:invoke command\n");
	teecErr = TEEC_InvokeCommand(&teecSession, command, &operation, NULL);

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
