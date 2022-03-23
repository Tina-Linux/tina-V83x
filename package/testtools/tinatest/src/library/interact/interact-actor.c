#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "interact-actor.h"

struct list_head list_actor; // list head for all actors.
int cnt_actor = 0;           // count for all actors.

#define INIT_LNODE() \
    struct actor *act = calloc(sizeof(struct actor), 1); \
    if (NULL == act) { \
        ERROR("malloc failed - %s\n", strerror(errno)); \
        return -1; \
    } \
    act->reply = calloc(MAX_TEXT, 1); \
    if (NULL == act->reply) { \
        free(act); \
        ERROR("malloc failed - %s\n", strerror(errno)); \
        return -1; \
    } \
    if (NULL == list_actor.prev) \
        INIT_LIST_HEAD(&list_actor); \
    list_add_tail(&act->lnode, &list_actor); \
    cnt_actor++; \
    DEBUG(BASE, "register interact: %d\n", cnt_actor);

#define ADD_CMD(CMD) \
    act->func[cmd_ ## CMD] = (cmd_func)CMD;

int interact_register(
        f_ask ask,
        f_tips tips,
        f_istrue istrue,
        f_scan scan,
        f_upfile upfile,
        f_downfile downfile,
        f_showimg showimg)
{
    INIT_LNODE();

    ADD_CMD(ask);
    ADD_CMD(tips);
    ADD_CMD(istrue);
    ADD_CMD(scan);
    ADD_CMD(upfile);
    ADD_CMD(downfile);
    ADD_CMD(showimg);

    return 0;
}

/***********************************************************
 * [below] Function to do command.
 **********************************************************/
static int interact_do_ask(struct actor *act)
{
    return ((f_ask)(act->func[acting->cmd]))(acting->path,
            acting->text, act->reply, MAX_TEXT);
}

static int interact_do_istrue(struct actor *act)
{
    int ret = ((f_istrue)(act->func[acting->cmd]))(acting->path, acting->text);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == (int)true ? STR_TRUE : STR_FALSE);
    return 0;
}

static int interact_do_tips(struct actor *act)
{
    return ((f_tips)(act->func[acting->cmd]))(acting->path, acting->text);
}

static int interact_do_scan(struct actor *act)
{
    const char *key = acting->text;
    const char *tips = acting->text + 512;
    return ((f_upfile)(act->func[acting->cmd]))(acting->path, tips, key);
}

static int interact_do_upfile(struct actor *act)
{
    const char *filepath = acting->text;
    const char *tips = acting->text + 512;
    int ret = ((f_upfile)(act->func[acting->cmd]))(acting->path, filepath, tips);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == (int)true ? STR_TRUE : STR_FALSE);
    return 0;
}

static int interact_do_downfile(struct actor *act)
{
    const char *filename = acting->text;
    const char *tips = acting->text + 512;
    return ((f_upfile)(act->func[acting->cmd]))(acting->path, filename, tips);
}

static int interact_do_showimg(struct actor *act)
{
    const char *filepath = acting->text;
    const char *tips = acting->text + 512;
    int ret = ((f_showimg)(act->func[acting->cmd]))(acting->path, filepath, tips);
    if (-1 == ret)
        return -1;
    sprintf(act->reply, "%s", ret == 1 ? STR_TRUE : STR_FALSE);
    return 0;
}
/***********************************************************
 * [above] Function to do command.
 **********************************************************/

#define interact_do(CMD, need_resp) \
    if (need_resp != false) { \
        pthread_mutex_lock(&acting->mutex); \
        acting->need_respond = true; \
        pthread_mutex_unlock(&acting->mutex); \
    } \
    if (!act->func[acting->cmd]) \
        return -1; \
    return interact_do_ ## CMD(act);

int interact_actor_do(struct actor *act)
{
    switch (acting->cmd) {
    case cmd_ask:
        interact_do(ask, true);
    case cmd_istrue:
        interact_do(istrue, true);
    case cmd_scan:
        interact_do(scan, true);
    case cmd_tips:
        interact_do(tips, false);
    case cmd_upfile:
        interact_do(upfile, true);
    case cmd_downfile:
        interact_do(downfile, true);
    case cmd_showimg:
        interact_do(showimg, true);
    default:
        return -1;
    }
}
