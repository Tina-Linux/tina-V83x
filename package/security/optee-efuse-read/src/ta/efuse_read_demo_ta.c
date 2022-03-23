#include <tee_ta_api.h>
#include <utee_syscalls.h>
#include <stdio.h>
#include <string.h>

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
/* Called when a command is invoked */
TEE_Result TA_InvokeCommandEntryPoint(void *pSessionContext,
				      uint32_t nCommandID, uint32_t nParamTypes,
				      TEE_Param pParams[4])
{
	char *strsrc = (char *)pParams[0].memref.buffer;
	uint8_t rdbuf[200];
	uint8_t i, rd_len;
	uint8_t keyname[50] = "chipid";

	(void)pSessionContext;
	(void)nParamTypes;
	(void)pParams;
	printf("TA:rec cmd 0x%x\n", nCommandID);
	switch (nCommandID) {
	case 0x210:
		if (strsrc[0] != 0) {
			for (i = 0; i < 49; i++) {
				if (strsrc[i] == 0)
					break;
				keyname[i] = strsrc[i];
			}
			keyname[i] = 0;
		}
		printf("read efuse:%s\n", keyname);

		i = utee_sunxi_read_efuse((const char *)keyname, &rd_len,
					  rdbuf);
		if (i == TEE_SUCCESS) {
			printf("read result:\n");
			dump(rdbuf, rd_len);
		} else {
			printf("read failed, return:%d\n", i);
		}

		return i;
	case 0x220:
		if (strsrc[0] != 0) {
			for (i = 0; i < 49; i++) {
				if (strsrc[i] == 0)
					break;
				keyname[i] = strsrc[i];
			}
			keyname[i] = 0;
		}else{
			memcpy(keyname,"testkey",sizeof("testkey"));
			keyname[49]=0;
		}
		i = utee_sunxi_keybox((const char*)keyname, rdbuf, 16);
		if (i != TEE_SUCCESS) {
			printf("read key:%s from keybox failed with:%d\n",keyname,i);
			return i;
		} else {
			i = utee_sunxi_read_efuse("oem_secure", &rd_len,
						  rdbuf + 16);
			if (i == TEE_SUCCESS) {
				printf("read result:\n");
				dump(rdbuf, rd_len + 16);
			} else {
				printf("read efuse failed with:%d\n", i);
			}
		}
		return i;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
