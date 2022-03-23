#include <tee_ta_api.h>
#include <stdio.h>
#include <base64.h>
#include <base64_demo_ta.h>

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
TEE_Result TA_OpenSessionEntryPoint(uint32_t nParamTypes, TEE_Param pParams[4],
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
	uint8_t *input_buf, *output_buf;
	uint32_t input_size, output_size, output_len;
	size_t result_len = 0;
	input_buf	 = (uint8_t *)pParams[0].memref.buffer;
	input_size	= pParams[0].memref.size;
	output_buf	= (uint8_t *)pParams[1].memref.buffer;
	output_size       = pParams[1].memref.size;

	(void)pSessionContext;
	(void)nParamTypes;

	printf("TA:rec cmd 0x%x\n", nCommandID);
	printf("input size:%d\n", input_size);
	switch (nCommandID) {
	case CMD_DECODE:
		if (!EVP_DecodedLength(&result_len, input_size)) {
			return TEE_ERROR_GENERIC;
		}
		if (result_len > output_size) {
			return TEE_ERROR_BAD_PARAMETERS;
		}
		if (!EVP_DecodeBase64(output_buf, &result_len, output_size,
				      input_buf, input_size)) {
			return TEE_ERROR_GENERIC;
		}
		output_len	 = result_len;
		pParams[2].value.a = output_len;
		return TEE_SUCCESS;
	case CMD_ENCODE:
		if (!EVP_EncodedLength(&result_len, input_size)) {
			return TEE_ERROR_GENERIC;
		}
		if (result_len > output_size) {
			return TEE_ERROR_BAD_PARAMETERS;
		}
		output_len = EVP_EncodeBlock(output_buf, input_buf, input_size);
		pParams[2].value.a = output_len;
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
