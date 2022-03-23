/*
 * "interact" is a core for interaction between outlog and testcase.
 * We seen a outlog plugin as a "actor", which will work in private thread.
 * In other word, we will create a thread for a outlog plugin.
 *
 * testcase <-> core <-> outlog(actor)
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

#include <sys/select.h>
#include "interact-core.h"

pthread_t pth_main; // the thread for core code.
struct acting *acting;
struct msg msg = {0}, *pmsg = &msg;
struct msgattr msgattr = {
    .msgno = 0,
    .len = sizeof(struct msg) - sizeof(long),
};

#define interact_init_queue_do(type, offset) \
{ \
    key_t key = msgattr.key + offset; \
    msgattr.msgid_ ##type = msgget(key, IPC_CREAT | IPC_EXCL); \
    if (msgattr.msgid_ ##type < 0) { \
        if (errno != EEXIST) \
            goto err; \
        /* if msg existed already, remove it firstly */ \
        msgattr.msgid_ ##type = msgget(key, 0); \
        if (msgattr.msgid_ ##type < 0) \
            goto err; \
        if (msgctl(msgattr.msgid_ ##type, IPC_RMID, NULL) < 0) \
            goto err; \
        msgattr.msgid_ ##type = msgget(key, IPC_CREAT | IPC_EXCL); \
        if (msgattr.msgid_ ##type < 0) \
            goto err; \
    } \
}

static int interact_init_queue()
{
    msgattr.msgno = 0;
    msgattr.msg_core_addr = getpid();

    /* get msg key */
    msgattr.key = ftok(IPC_KEY, 0);
    if (-1 == msgattr.key)
        goto err;

    interact_init_queue_do(core_to_case, CORE_TO_CASE_OFFSET);
    interact_init_queue_do(case_to_core, CASE_TO_CORE_OFFSET);

    return 0;
err:
    ERROR("init queue failed - %s\n", strerror(errno));
    return -1;
}

#define SETENV(var, val) { \
    char buf[100] = {0}; \
    snprintf(buf, 100, "%ld", (long)val); \
    DEBUG(DATA, "setenv %s = %s\n", var, buf); \
    if (setenv(var, buf, true) < 0) { \
        ERROR("setenv failed - %s\n", strerror(errno)); \
        return -1; \
    } \
}

static inline int interact_msgsnd()
{
    DEBUG(DATA, "interact core send msg(%dB) to case:\n", msgattr.len);
    DEBUG(DATA, "\taddr\t: %ld\n", pmsg->addr);
    DEBUG(DATA, "\tcmd\t: %d\n", pmsg->cmd);
    DEBUG(DATA, "\tpath\t: %s\n", pmsg->path);
    DEBUG(DATA, "\ttext\t: %s\n", pmsg->text);
    if (msgsnd(msgattr.msgid_core_to_case, pmsg, msgattr.len, 0) < 0) {
        ERROR("msgsnd failed - %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static int interact_msgsnd_err()
{
    pmsg->cmd = cmd_err;
    pmsg->text[0] = '\0';
    return interact_msgsnd();
}

static int interact_msgsnd_ok()
{
    return interact_msgsnd();
}

static void *interact_main(void *arg)
{
    int ret = 0;
    int msgsnd_stu = -1;
    char resp[4] = {0}; /* just save respond from fd */
    while (true) {
        int read_cnt = 0;
        /* rcv msg */
        ret = msgrcv(msgattr.msgid_case_to_core, pmsg, msgattr.len, 0,
                MSG_NOERROR);
        if (ret < 0) {
            if (EIDRM == errno || EINTR == errno) {
                DEBUG(BASE, "interact msg exit\n");
                pthread_exit(0);
            }
            ERROR("msgrcv failed - %s\n", strerror(errno));
            continue;
        }
        if (pmsg->cmd >= cmd_cnt) {
            ERROR("interact invoid cmd from %s\n", pmsg->path);
            continue;
        }
        DEBUG(DATA, "interact core recv msg(%dB) from case:\n", ret);
        DEBUG(DATA, "\taddr\t: %ld\n", pmsg->addr);
        DEBUG(DATA, "\tcmd\t: %d\n", pmsg->cmd);
        DEBUG(DATA, "\tpath\t: %s\n", pmsg->path);
        DEBUG(DATA, "\ttext\t: %s\n", pmsg->text);
        /* fill acting */
        pthread_mutex_lock(&acting->mutex);
        acting->id = ++msgattr.msgno;
        acting->end_cnt = 0;
        acting->need_respond = false;
        acting->cmd = pmsg->cmd;
        acting->path = pmsg->path;
        acting->text = pmsg->text;
        acting->reply.buf = pmsg->text;
        acting->reply.len = MAX_TEXT;
        /* we should set timeout again because select will change it */
        acting->timeout.tv_sec = ACTING_TIMEOUT_SEC;
        acting->timeout.tv_usec = 0;
        /* we should set sets again because select will change it if timeout */
        FD_ZERO(&acting->rfd);
        FD_SET(acting->pfd[0], &acting->rfd);
        pthread_mutex_unlock(&acting->mutex);
        /* get back */
        ret = select(acting->pfd[0] + 1, &acting->rfd, NULL,
                NULL, &acting->timeout);
        if (ret == 0) {
            msgsnd_stu = 0;
            DEBUG(BASE, "interact select timeout\n");
        } else if (ret < 0) {
            msgsnd_stu = 0;
            ERROR("interact select failed - %s\n", strerror(errno));
        } else if ((read_cnt = read(acting->pfd[0], resp, 4)) != 4) {
            msgsnd_stu = 0;
            if (read_cnt == 3)
                ERROR("All outlog plugin failed\n");
            else if (read_cnt < 0)
                ERROR("interact read failed - %s\n", strerror(errno));
        } else {
            msgsnd_stu = 1;
        }
        pthread_mutex_lock(&acting->mutex);
        if (acting->need_respond == true) {
            switch (msgsnd_stu) {
                case 0: interact_msgsnd_err(); break;
                case 1: interact_msgsnd_ok();
            }
        }
        acting->id = 0; // disable acting
        msgsnd_stu = -1;
        pthread_mutex_unlock(&acting->mutex);
    }
    return NULL;
}

static void *interact_actor(void *arg)
{
    int id = 0, ret = 0;
    struct actor *act = (struct actor *)arg;
    while (true) {
        pthread_mutex_lock(&acting->mutex);
        /* "id == acting->id" in case of actor_do failed, avoid to do again */
        if (acting->id == 0 || id == acting->id) {
            pthread_mutex_unlock(&acting->mutex);
            usleep(150 * 1000);
            continue;
        }
        id = acting->id;
        pthread_mutex_unlock(&acting->mutex);

        act->reply[0] = '\0';
        ret = interact_actor_do(act);

        pthread_mutex_lock(&acting->mutex);
        /* out-of-date reply */
        if (acting->id != id) {
            DEBUG(BASE, "out-of-data reply\n");
            pthread_mutex_unlock(&acting->mutex);
            continue;
        }
        /* backcall failed */
        acting->end_cnt++;
        /*
         * enc_cnt >= cnt_actor: means no any outlog done
         * so, enc_cnt < cnt_actor, we do nothing but exit
         * to wait for other actor.
         */
        if (ret != 0) {
            if (acting->end_cnt >= cnt_actor) {
                acting->id = 0; // disable acting
                write(acting->pfd[1], "err", 3);
            }
            pthread_mutex_unlock(&acting->mutex);
            continue;
        }
        /* valid reply, copy valid reply */
        memset(acting->reply.buf, 0, acting->reply.len);
        strncpy(acting->reply.buf, act->reply,
                min(strlen(act->reply), msgattr.len));
        acting->id = 0; // disable acting
        write(acting->pfd[1], "done", 4);
        pthread_mutex_unlock(&acting->mutex);
    }
    return NULL;
}

static int interact_init_acting()
{
    acting = malloc(sizeof(struct acting));
    if (acting == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return -1;
    }
    memset(acting, 0, sizeof(struct acting));

    pthread_mutex_init(&acting->mutex, NULL);

    if (pipe(acting->pfd) < 0) {
        ERROR("pipe failed - %s\n", strerror(errno));
        return -1;
    }

    acting->reply.len = MAX_TEXT;
    return 0;
}

/*
 * create msg queue, all threads.
 */
int interact_init()
{
    if (!cnt_actor)
        return 0;

    /* init msg squeue */
    if (interact_init_queue() != 0)
        return -1;

    /* init acting struction */
    if (interact_init_acting() != 0)
        return -1;

    /* setenv */
    SETENV(MSG_CORE_TO_CASE, msgattr.msgid_core_to_case);
    SETENV(MSG_CASE_TO_CORE, msgattr.msgid_case_to_core);

    /* create main*/
    if (pthread_create(&pth_main, NULL, interact_main, NULL) < 0) {
        ERROR("pthread_create failed - %s\n", strerror(errno));
        return -1;
    }
    pthread_detach(pth_main);

    /* create actor */
    struct actor *act = NULL;
    list_for_each_entry(act, &list_actor, lnode) {
        /* create actor thread and pass actor struction to thread */
        if (pthread_create(&act->pth, NULL, interact_actor, act) < 0) {
            ERROR("pthread_create failed - %s\n", strerror(errno));
            return -1;
        }
        /* set detach, by this, thread will recycle themselves */
        pthread_detach(act->pth);
    }
    return 0;
}

/*
 * delete msg queue, all threads before exit.
 */
int interact_exit()
{
    if (!cnt_actor)
        return 0;

    /* the follow while loop fix bug:
     *  if the last testcase call interact-api before exit, which
     *  has no reponse like ttips, interact-api send msg and return,
     *  then testcase exit and tinatest exit.
     *  The interact-core will be kill if tinatest exit, which cause
     *  ttips for outlog-actor will not be done.
     * To fix it, we check whether there are msg, and check whether
     * interact-core is doing.
     */
    struct msg tmp_msg;
    while (true) {
        if (msgrcv(msgattr.msgid_case_to_core, &tmp_msg, msgattr.len,
                msgattr.msg_core_addr, MSG_NOERROR | IPC_NOWAIT) < 0) {
            if (errno == ENOMSG) {
                while (acting->id != 0)
                    usleep(100 * 1000);
                break;
            }
            ERROR("msgrcv failed - %s\n", strerror(errno));
            break;
        }
        // send msg back to queue.
        if (msgsnd(msgattr.msgid_case_to_core, &tmp_msg, msgattr.len, 0) < 0) {
            ERROR("msgsnd failed - %s\n", strerror(errno));
            break;
        }
        usleep(200 * 1000);
    }
    pthread_cancel(pth_main);

    struct actor *pre = NULL;
    struct actor *next = NULL;
    list_for_each_entry_safe(pre, next, &list_actor, lnode) {
        pthread_cancel(pre->pth);
        free(pre->reply);
        free(pre);
    }
    free(acting);
    msgctl(msgattr.msgid_core_to_case, IPC_RMID, NULL);
    msgctl(msgattr.msgid_case_to_core, IPC_RMID, NULL);

    return 0;
}
