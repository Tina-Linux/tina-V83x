/*
 *  Copyright (C) 2019 Realtek Semiconductor Corporation.
 *
 *  Author: Alex Lu <alex_lu@realsil.com.cn>
 */
#ifndef GATT_BLUE_CMD_H
#define GATT_BLUE_CMD_H
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#if __cplusplus
extern "C" {
#endif

#define BTA_WAKE_CODE    0x78

#define REQ_PENDING    0
#define REQ_DONE    1
struct bta_command {
    unsigned int id;
    uint16_t opcode;
    uint16_t req_status;
    int req_result;
    int argc;
    void **argv;
    uint32_t rparams[2];
};

typedef int (*bcmd_handler_t)(uint16_t opcode, int argc, void **argv);

void gatt_set_bta_cmd_fd(int fd[2]);
void gatt_set_bta_mainloop_id(pthread_t tid);
void gatt_bta_register_handler(bcmd_handler_t handler);

int gatt_bta_submit_command_wait(uint16_t opcode, int argc, void **argv);
void btgatt_run_command(void);

void gatt_bta_clear_commands(void);

#if __cplusplus
};
#endif

#endif
