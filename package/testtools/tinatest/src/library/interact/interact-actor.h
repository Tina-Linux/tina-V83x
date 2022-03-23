#ifndef __INTERACT_ACTOR_H
#define __INTERACT_ACTOR_H
#include "outlog.h"
#include <sys/select.h>

enum cmd {
    cmd_tips = 0,
    cmd_ask,
    cmd_istrue,
    cmd_scan,
    cmd_upfile,
    cmd_downfile,
    cmd_showimg,
    cmd_cnt, //作为cmd个数计数,命令务必写在此前
    cmd_err,
};

/*
 * struction for a actor.
 *
 * @func: all cmds function pointer.
 * @reply: the buf for reply from outlog to testcase.
 * @pth: the pthread key for actor thread.
 * @lnode: list node.
 */
typedef void (*cmd_func)(void *);   // a general pointer for function.
struct actor {
    cmd_func func[cmd_cnt];
    char *reply;
    pthread_t pth;
    struct list_head lnode;
};

/*
 * struction for interaction between core and outlog thread,
 * which stores the infomation of doing-interaction.
 *
 * @id: id for acting
 * @end_cnt: count for finishing actor.
 * @need_respond: which, set by outlog, means core should respond to testcase.
 * @cmd: id for cmd
 * @path: the testcase name which couse this acting.
 * @text: the text word from testcase to core/outlog.
 * @reply: the buf for relpy from outlog to testcase.
 * @rfd: a set of fd work for select.
 * @timeout: select timeout.
 * @pfd: fd between core and outlog.
 * @mutex: mutex lock while accessing this struction: acting.
 */
struct acting {
    /* init when ask */
    int id;
    int end_cnt;
    bool need_respond;
    enum cmd cmd;
    const char *path;
    const char *text;
    struct {
        char *buf;
        int len;
    } reply;

    /* init when init */
    fd_set rfd;
#define ACTING_TIMEOUT_SEC 180
    struct timeval timeout;
    int pfd[2];
    pthread_mutex_t mutex;
};
extern struct acting *acting;

extern int interact_actor_do(struct actor *act);
extern struct list_head list_actor;
extern int cnt_actor;

#endif
