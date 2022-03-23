/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include <tee_client_api.h>
//#include <ta_storage.h>
//#include <tee_api_defines.h>
//#include <tee_api_defines_extensions.h>
//#include <tee_api_types.h>

#include "libstorage.h"

#define TA_STORAGE_CMD_OPEN			0
#define TA_STORAGE_CMD_READ			1
#define TA_STORAGE_CMD_WRITE			2
#define TA_STORAGE_CMD_UNLINK			3
#define TA_STORAGE_CMD_CREATE			4

#define TEEC_OPERATION_INITIALIZER {0}
TEEC_Result optee_fs_create(TEEC_Context ctx, TEEC_Session *sess, void *file_name, uint32_t file_size,
			   uint32_t flags, uint32_t *obj, uint32_t storage_id)
{
	TEEC_Operation op = TEEC_OPERATION_INITIALIZER;
	TEEC_Result res;
	uint32_t org;

	TEEC_SharedMemory	share_id;
	share_id.size = file_size;
	share_id.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	if (TEEC_AllocateSharedMemory(&ctx, &share_id) != TEEC_SUCCESS){
		printf("%s: allocate  share memory fail", __func__);
		return TEEC_ERROR_OUT_OF_MEMORY;
	}

	memcpy(share_id.buffer, file_name, file_size);

	op.started = 1;
	op.params[0].memref.parent= &share_id;
	op.params[0].memref.size = 0;
	op.params[1].value.a = flags;
	op.params[1].value.b = 0;
	op.params[2].value.a = storage_id;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
					 TEEC_VALUE_INOUT, TEEC_VALUE_INPUT,
					 TEEC_NONE);

	res = TEEC_InvokeCommand(sess, TA_STORAGE_CMD_CREATE, &op, &org);

	if (res == TEEC_SUCCESS)
		*obj = op.params[1].value.b;

	TEEC_ReleaseSharedMemory(&share_id);

	return res;
}

TEEC_Result optee_fs_open(TEEC_Context ctx, TEEC_Session *sess, void *file_name, uint32_t file_size,
			   uint32_t flags, uint32_t *obj, uint32_t storage_id)
{
	TEEC_Operation op = TEEC_OPERATION_INITIALIZER;
	TEEC_Result res;
	uint32_t org;

	TEEC_SharedMemory	share_id;
	share_id.size = file_size;
	share_id.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	if (TEEC_AllocateSharedMemory(&ctx, &share_id) != TEEC_SUCCESS){
		printf("%s: allocate  share memory fail", __func__);
		return TEEC_ERROR_OUT_OF_MEMORY;
	}

	memcpy(share_id.buffer, file_name, file_size);

	op.started = 1;
	op.params[0].memref.parent= &share_id;
	op.params[0].memref.size = 0;
	op.params[1].value.a = flags;
	op.params[1].value.b = 0;
	op.params[2].value.a = storage_id;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
					 TEEC_VALUE_INOUT, TEEC_VALUE_INPUT,
					 TEEC_NONE);

	res = TEEC_InvokeCommand(sess, TA_STORAGE_CMD_OPEN, &op, &org);

	if (res == TEEC_SUCCESS)
		*obj = op.params[1].value.b;

	TEEC_ReleaseSharedMemory(&share_id);

	return res;
}

TEEC_Result optee_fs_read(TEEC_Context ctx, TEEC_Session *sess, uint32_t obj, void *data,
			   uint32_t data_size, uint32_t *count)
{
	TEEC_Result res;
	TEEC_Operation op = TEEC_OPERATION_INITIALIZER;
	uint32_t org;

	TEEC_SharedMemory	share_id;
	share_id.size = data_size;
	share_id.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	if (TEEC_AllocateSharedMemory(&ctx, &share_id) != TEEC_SUCCESS){
		printf("%s: allocate  share memory fail", __func__);
		return TEEC_ERROR_OUT_OF_MEMORY;
	}

	memcpy(share_id.buffer, data, data_size);

	op.started = 1;

	op.params[0].memref.parent = &share_id;
	op.params[0].memref.size = 0;
	op.params[1].value.a = obj;
	op.params[1].value.b = 0;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
					 TEEC_VALUE_INOUT, TEEC_NONE,
					 TEEC_NONE);

	res = TEEC_InvokeCommand(sess, TA_STORAGE_CMD_READ, &op, &org);

	if (res == TEEC_SUCCESS)
		*count = op.params[1].value.b;

	memcpy(data, share_id.buffer, data_size);
	TEEC_ReleaseSharedMemory(&share_id);

	return res;
}

TEEC_Result optee_fs_write(TEEC_Context ctx, TEEC_Session *sess, uint32_t obj, void *data,
			    uint32_t data_size)
{
	TEEC_Operation op = TEEC_OPERATION_INITIALIZER;
	uint32_t org;
	TEEC_Result res;

	TEEC_SharedMemory	share_id;
	share_id.size = data_size;
	share_id.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	if (TEEC_AllocateSharedMemory(&ctx, &share_id) != TEEC_SUCCESS){
		printf("%s: allocate  share memory fail", __func__);
		return TEEC_ERROR_OUT_OF_MEMORY;
	}
	memcpy(share_id.buffer, data, data_size);

	op.started = 1;
	op.params[0].memref.parent = &share_id;
	op.params[0].memref.size = 0;
	op.params[1].value.a = obj;
	op.params[1].value.b = 0;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
					 TEEC_VALUE_INPUT, TEEC_NONE,
					 TEEC_NONE);

	res = TEEC_InvokeCommand(sess, TA_STORAGE_CMD_WRITE, &op, &org);
	memcpy(data, share_id.buffer, data_size);
	TEEC_ReleaseSharedMemory(&share_id);
	return res;
}

TEEC_Result optee_fs_unlink(TEEC_Session *sess, uint32_t obj)
{
	TEEC_Operation op = TEEC_OPERATION_INITIALIZER;
	uint32_t org;

	op.params[0].value.a = obj;
	op.started = 1;

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	return TEEC_InvokeCommand(sess, TA_STORAGE_CMD_UNLINK, &op, &org);
}

