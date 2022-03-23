#include <tee_ta_api.h>
#include <stdio.h>

/*
 * Trusted Application Entry Points
 */

/* Called each time a new instance is created */
TEE_Result TA_CreateEntryPoint(void)
{
		printf("TA:creatyentry!\n");
	return TEE_SUCCESS;
}

/* Called each time an instance is destroyed */
void TA_DestroyEntryPoint(void)
{
}

/* Called each time a session is opened */
TEE_Result TA_OpenSessionEntryPoint(uint32_t nParamTypes,
				    TEE_Param pParams[4],
				    void **ppSessionContext)
{
	(void)nParamTypes;
	(void)pParams;
	(void)ppSessionContext;
		printf("TA:open session!\n");
	return TEE_SUCCESS;
}

/* Called each time a session is closed */
void TA_CloseSessionEntryPoint(void *pSessionContext)
{
	(void)pSessionContext;
}

/* Called when a command is invoked */
TEE_Result TA_InvokeCommandEntryPoint(void *pSessionContext,
				      uint32_t nCommandID, uint32_t nParamTypes,
				      TEE_Param pParams[4])
{
	char * strsrc = (char*)pParams[0].memref.buffer;
	(void)pSessionContext;
	(void)nParamTypes;
	(void)pParams;

	printf("TA:rec cmd 0x%x\n",nCommandID);
	switch (nCommandID) {
	case 0x210:
		if(strsrc[0]!=0){
			printf("TA:hello %10s\n",strsrc);
		}else{
			printf("TA:hello world!\n");
		}
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

