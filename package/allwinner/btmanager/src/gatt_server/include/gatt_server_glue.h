#ifndef BT_GATT_SERVER_GLUE_H
#define BT_GATT_SERVER_GLUE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UINT8     unsigned char
#define UINT16    unsigned short int
#define UINT32    unsigned int
#define INT8      signed char
#define INT16     short int
#define INT32     int
#define CHAR      char

#define AG_MAX_BDADDR_LEN  18

// start a thread
INT32 CSM_init();
// quit the thread
INT32 CSM_deinitGatts() ;
// setcb
INT32 CSM_addService(CHAR * service_uuid, UINT8 is_primary, INT32 number) ;

INT32 CSM_addChar(INT32 service_handle,CHAR * uuid, INT32 properties, INT32 permissions) ;

INT32 CSM_addDesc(INT32 service_handle,CHAR * uuid, INT32 permissions) ;

INT32 CSM_startService(INT32 service_handle) ;

INT32 CSM_stopService(INT32 service_handle) ;

INT32 CSM_unregisterService() ;

INT32 CSM_sendResponse(INT32 trans_id,INT32 status,INT32 handle,CHAR * p_value,INT32 value_len,INT32 auth_req) ;

int CSM_sendWriteResponse(unsigned trans_id, unsigned int attr_handle, unsigned int state);

INT32 CSM_sendIndication(INT32 handle,INT32 fg_confirm, CHAR * p_value, INT32 value_len) ;

INT32 CSM_deleteService(INT32 service_handle);

INT32 CSM_deinitGatts();

#ifdef __cplusplus
}
#endif

#endif //AG_GATT_API_H
