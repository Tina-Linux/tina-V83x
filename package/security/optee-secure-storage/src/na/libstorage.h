#ifndef _LIBSTORAGE_H__
#define _LIBSTORAGE_H__
/* This UUID is generated with the ITU-T UUID generator at
   http://www.itu.int/ITU-T/asn1/uuid.html */
#include <string.h>
#include <stdio.h>

#include <tee_client_api.h>

TEEC_Result optee_fs_create(TEEC_Context ctx, TEEC_Session *sess, void *file_name, uint32_t file_size,
			   uint32_t flags, uint32_t *obj, uint32_t storage_id);
TEEC_Result optee_fs_open(TEEC_Context ctx, TEEC_Session *sess, void *file_name, uint32_t file_size,
			   uint32_t flags, uint32_t *obj, uint32_t storage_id);
TEEC_Result optee_fs_read(TEEC_Context ctx, TEEC_Session *sess, uint32_t obj, void *data,
			   uint32_t data_size, uint32_t *count);
TEEC_Result optee_fs_write(TEEC_Context ctx, TEEC_Session *sess, uint32_t obj, void *data,
			    uint32_t data_size);
TEEC_Result optee_fs_unlink(TEEC_Session *sess, uint32_t obj);

#endif
