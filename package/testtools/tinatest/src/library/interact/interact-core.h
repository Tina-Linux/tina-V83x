#ifndef __INTERACT_CORE_H
#define __INTERACT_CORE_H

#include <sys/msg.h>
#include "outlog.h"
#include "interact-actor.h"

/* env variable */
#define MSG_CORE_TO_CASE "MSG_CORE_TO_CASE"
#define MSG_CASE_TO_CORE "MSG_CASE_TO_CORE"

/*
 * msg struction between testcase and core
 *
 * @addr: the address(pid) of testcase
 * @cmd: interact command.
 * @path: path of testcase
 * @text: message buf.
 */
struct msg {
    long addr;
    enum cmd cmd;
    char path[MAX_PATH];
    char text[MAX_TEXT];
};
extern struct msg msg, *pmsg;

/*
 * all attributes for message queue
 *
 * @key: message queue key;
 * @msgid: message queue id;
 * @msgno: the count for interactive request(seens as acting id)
 * @msg_core_addr: the core address for message queue.
 * @len: the valid length of message between core and testcase for message queue.
 */
struct msgattr {
    key_t key;
#define IPC_KEY "/etc/tinatest.json"
    /* core send to testcases */
    int msgid_core_to_case;
#define CORE_TO_CASE_OFFSET 0x00
    /* core receive from testcases */
    int msgid_case_to_core;
#define CASE_TO_CORE_OFFSET 0x01
    unsigned long msgno;
    long msg_core_addr;
    int len;
};
extern struct msgattr msgattr;

#endif
