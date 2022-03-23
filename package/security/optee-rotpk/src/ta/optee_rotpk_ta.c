#include <tee_ta_api.h>
#include <stdio.h>
#include <utee_syscalls.h>
#include <string.h>

#if 1
static void dump(uint8_t *buf, int ttl_len)
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
#endif

/*
 * Trusted Application Entry Points
 */

/* Called each time a new instance is created */
TEE_Result TA_CreateEntryPoint(void)
{
	printf("TA: create entry!\n");
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
	printf("TA: open session!\n");
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
	char *strsrc = (char *)pParams[0].memref.buffer;
	uint8_t rdbuf[200];
	uint8_t i, rd_len;
	uint8_t keyname[50] = "chipid";
	uint8_t *wrBuf      = (uint8_t *)pParams[1].memref.buffer;
	uint8_t wr_len      = (uint8_t)pParams[2].value.a;

	(void)pSessionContext;
	(void)nParamTypes;
	(void)pParams;
	printf("TA: rec cmd 0x%x\n", nCommandID);
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
		printf("TA: read efuse:%s\n", keyname);

		if (memcmp(keyname, "rotpk", 5) != 0) {
			printf("TA: keyname is %s, should be rotpk.", keyname);
			return TEE_ERROR_BAD_PARAMETERS;
		}

		i = utee_sunxi_read_efuse((const char *)keyname, &rd_len,
					  rdbuf);
		if (i == TEE_SUCCESS) {
			printf("TA: read efuse result:\n");
			dump(rdbuf, rd_len);
		} else {
			printf("TA: read failed, return:%d\n", i);
		}
		memcpy(wrBuf, rdbuf, rd_len);
		pParams[2].value.a=rd_len;

		return i;
	case 0x221:
		if (strsrc[0] != 0) {
			for (i = 0; i < 49; i++) {
				if (strsrc[i] == 0)
					break;
				keyname[i] = strsrc[i];
			}
			keyname[i] = 0;
		} else {
			printf("TA: invalid key name\n");
			return TEE_ERROR_BAD_PARAMETERS;
		}

		if (memcmp(keyname, "rotpk", 5) != 0) {
			printf("TA: keyname is %s, should be rotpk.", keyname);
			return TEE_ERROR_BAD_PARAMETERS;
		}

		printf("TA: keyname:%s,key len:%d,keydata:\n", keyname, wr_len);
		dump(wrBuf, wr_len);

		i = utee_sunxi_write_efuse((const char *)keyname, wr_len,
					   wrBuf);
		if (i != TEE_SUCCESS) {
			printf("TA: burn efuse:%s failed with:%d\n",
			       keyname, i);
			return i;
		}

		i = utee_sunxi_read_efuse((const char *)keyname, &wr_len,
					  rdbuf);
		if (i == TEE_SUCCESS) {
			if (memcmp(wrBuf, rdbuf, wr_len) != 0) {
				printf("TA: burn efuse error!");
			}
		} else {
			printf("TA: read failed, return:%d\n", i);
		}

		return i;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

