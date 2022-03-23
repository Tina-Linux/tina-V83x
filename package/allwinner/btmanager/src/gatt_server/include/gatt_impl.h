
#ifndef GATT_IMPL_H
#define GATT_IMPL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum gattcmd_ops {
    GATTCMD_OP_ADD_SERVICE = 0,
    GATTCMD_OP_ADD_CHAR,
    GATTCMD_OP_ADD_DESC,
    GATTCMD_OP_START_SERVICE,
    GATTCMD_OP_STOP_SERVICE,
    GATTCMD_OP_SEND_RSP,
    GATTCMD_OP_SEND_IND,
    GATTCMD_OP_DEL_SERVICE,
    GATTCMD_OP_UNREG_SERVICE,
};

int gatt_send_write_rsp(unsigned int id, unsigned int handle, unsigned int status);
int gatt_thread_create(void);
void gatt_thread_quit(void);

#ifdef __cplusplus
}
#endif

#endif //AG_GATT_API_H
