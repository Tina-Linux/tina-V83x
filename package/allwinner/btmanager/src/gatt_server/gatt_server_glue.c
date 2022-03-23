/*
  * Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
  * Author        laumy <liumingyuan@allwinnertech.com>
  * Version       0.0.1
  *
  * Author:        kevin
  * Version:       1.0.0
  * Modification: modified return value
  * Data: 19/10/27
  */

#include <pthread.h>
#include "gatt_server_glue.h"
#include "gatt_impl.h"
#include "src/shared/util.h"
#include "gatt_blue_cmd.h"
#include "log.h"

int CSM_init()
{
	return gatt_thread_create();
}

int CSM_addService(CHAR * service_uuid,
				UINT8 is_primary, INT32 number)
{
	void *argv[3];
	int result;

	argv[0] = service_uuid;
	argv[1] = (void *)(UINT_TO_PTR(is_primary));
	argv[2] = (void *)(UINT_TO_PTR(number));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_ADD_SERVICE, 3, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_ADD_SERVICE, result);
		return -1;
	}

	return 0;
}

int CSM_addChar(INT32 service_handle,
				CHAR * uuid, INT32 properties, INT32 permissions)
{

	void *argv[4];
	int result;

	argv[0] = (void *)(UINT_TO_PTR(service_handle));
	argv[1] = uuid;
	argv[2] = (void *)(UINT_TO_PTR(properties));
	argv[3] = (void *)(UINT_TO_PTR(permissions));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_ADD_CHAR, 4, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_ADD_CHAR, result);
		return -1;
	}

	return 0;
}

int CSM_addDesc(INT32 service_handle,
				CHAR * uuid, INT32 permissions)
{
	void *argv[3];
	int result;

//	argv[0] =(void *)(UINT_TO_PTR(server_if));
	argv[0] = (void *)(UINT_TO_PTR(service_handle));
	argv[1] = uuid;
	argv[2] = (void *)(UINT_TO_PTR(permissions));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_ADD_DESC, 3, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_ADD_DESC, result);
		return -1;
	}

	return 0;
}

int CSM_startService(INT32 service_handle)
{
	void *argv[1];
	int result;

	argv[0] = (void *)(UINT_TO_PTR(service_handle));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_START_SERVICE, 1, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_START_SERVICE, result);
		return -1;
	}

	return 0;

}

int CSM_stopService(INT32 service_handle)
{
	void *argv[1];
	int result;

//	argv[0] =(void *)(UINT_TO_PTR(server_if));
	argv[0] = (void *)(UINT_TO_PTR(service_handle));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_STOP_SERVICE, 1, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_STOP_SERVICE, result);
		return -1;
	}

	return 0;

}

int CSM_deleteService(INT32 service_handle)
{
	void *argv[1];
	int result;

//	argv[0] =(void *)(UINT_TO_PTR(server_if));
	argv[0] = (void *)(UINT_TO_PTR(service_handle));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_DEL_SERVICE, 1, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_DEL_SERVICE, result);
		return -1;
	}

	return 0;

}

int CSM_sendResponse(INT32 trans_id,INT32 status,
						INT32 handle, CHAR * p_value,INT32 value_len,
						INT32 auth_req)
{
	void *argv[6];
	int result;

//	argv[0] =(void *)(UINT_TO_PTR(conn_id));
	argv[0] = (void *)(UINT_TO_PTR(trans_id));
	argv[1] = (void *)(UINT_TO_PTR(status));
	argv[2] = (void *)(UINT_TO_PTR(handle));
	argv[3] = (void *)(p_value);
	argv[4] = (void *)(UINT_TO_PTR(value_len));
	argv[5] = (void *)(UINT_TO_PTR(auth_req));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_SEND_RSP, 6, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_SEND_RSP, result);
		return -1;
	}

	return 0;
}

int CSM_sendWriteResponse(unsigned trans_id, unsigned int attr_handle, unsigned int state)
{
	return gatt_send_write_rsp(trans_id, attr_handle, state);
}

int CSM_sendIndication(INT32 handle,
						INT32 fg_confirm, CHAR * p_value,
						INT32 value_len)
{
	void *argv[4];
	int result;
	argv[0] = (void *)(UINT_TO_PTR(handle));
	argv[1] = (void *)(UINT_TO_PTR(fg_confirm));
	argv[2] = (void *)(p_value);
	argv[3] = (void *)(UINT_TO_PTR(value_len));
	result = gatt_bta_submit_command_wait(GATTCMD_OP_SEND_IND, 4, (void **)argv);
	if (result < 0) {
		pr_error("Run %04x command failed, %d",
			 GATTCMD_OP_SEND_RSP, result);
		return -1;
	}

	return 0;
}

int CSM_unregisterService()
{
	gatt_thread_quit();
	return 0;
}

int CSM_deinitGatts()
{
	gatt_thread_quit();
	return 0;
}
