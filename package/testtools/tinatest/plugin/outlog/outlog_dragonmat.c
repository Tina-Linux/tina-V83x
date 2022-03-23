/* Build: LDFLAGS += -lsocket_db */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <init_entry.h>

#include "outlog.h"
#include "fileops.h"
#include "interact.h"

#define BASE_JSON_PATH OUTLOG_PATH"/dragonmat"
#define WAIT_TILL_CONNECTED BASE_JSON_PATH"/wait_till_connected"
#define EXIT_WHEN_END BASE_JSON_PATH"/exit_when_end"
#define EXIT_CALL BASE_JSON_PATH"/exit_call"
#define EXIT_WHEN_END_CONTROL "/tmp/exit_dragonmat"
// This file is used by DragonMAT to show list.
#define TESTCASES_LIST_INI "/tmp/tinatest.ini"
#define TIMEOUT 60000
#define MSGSND 123
#define MSGRCV 124

volatile int connected;
int msgsnd_id=0;
int msgrcv_id=0;
int picsnd = 0;

typedef struct{
    long msgtype;
    char msgtext[2048];
}msgstru;

/*
 * abstract out the send command public part
 * and it will be called by dragonmat_* interface
 * which need to send msg.
 */
static int dragonmat_snd_common(msgstru *msgs)
{
    if (msgsnd_id == 0) {
        msgsnd_id = msgget(MSGSND, IPC_EXCL);
        if (msgsnd_id < 0) {
            ERROR("msgget failed! - %s\n", strerror(errno));
            return -1;
        }
    }

    if (msgsnd(msgsnd_id, msgs, sizeof(msgstru), 0) < 0) {
        ERROR("msgsnd failed! - %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/*
 * abstract out the revice command public part
 * and it will be called by dragonmat_* interface
 * which need to revice msg.
 */
static int dragonmat_rcv_common(msgstru *msgs)
{
    if (msgrcv_id == 0) {
        msgrcv_id = msgget(MSGRCV, IPC_EXCL);
        if (msgrcv_id < 0) {
            ERROR("msgget failed! - %s\n", strerror(errno));
            return -1;
        }
    }

    if (msgrcv(msgrcv_id, msgs, sizeof(msgstru), msgs->msgtype, 0) < 0) {
        ERROR("msgrcv failed! - %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/*
 * integrated send & revice command
 */
static int dragonmat_reply(msgstru *msgs)
{
    if ((dragonmat_snd_common(msgs)) < 0) {
        ERROR("dragonmat_reply: snd failed! - %s\n", strerror(errno));
        return -1;
    }

    if ((dragonmat_rcv_common(msgs)) < 0) {
        ERROR("dragonmat_reply: rcv failed! - %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int dragonmat_before_all(struct list_head *TASK_LIST)
{
    FILE *fp = fopen(TESTCASES_LIST_INI, "w");
    if (fp == NULL) {
        ERROR("Create %s failed - %s\n", TESTCASES_LIST_INI, strerror(errno));
        return -1;
    }

    struct task *task = NULL;
    char *key = NULL;
    list_for_each_entry(task, TASK_LIST, lnode) {
        key = strrchr(task->keypath, '/') + 1;
        DEBUG(BASE, "dragonmat: add testcase %s\n", key);
        fprintf(fp, "[%s]\n", key);
        fprintf(fp, "display_name = %s\n", key);
        fprintf(fp, "activated = 1\n\n");
    }

    fclose(fp);
    return 0;
}

int dragonmat_before_one(struct task *task)
{
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));

    if (connected == FALSE)
        return 0;

    char *key = strrchr(task->keypath, '/') + 1;
    DEBUG(BASE, "%s start\n", key);
    msgs.msgtype = (long)sendStartCase_fix(key, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("%s: reply failed! - %s\n", __func__, strerror(errno));
        return -1;
    }
    sleep(1);
    return 0;
}

int dragonmat_after_one_end(struct task *task)
{
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
    char *key = strrchr(task->keypath, '/') + 1;
    int result = 0;

    if (connected == FALSE)
        return 0;

    for (int num = 0; num < task->run_times; num++) {
        if (task->result[num] != 0) {
            result = task->result[num];
            break;
        }
    }

    DEBUG(BASE, "%s end with %d\n", key, result);
    msgs.msgtype = (long)sendResult_fix(key, "", result == 0 ? PASS : FAILED, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("%s: reply failed! - %s\n", __func__, strerror(errno));
        return -1;
    }
    sleep(1);
    return 0;
}

int dragonmat_after_all_call(int result)
{
    char *exit_call = NULL;
    int pid, len, ret = 0, status = 0;
    char *buf = NULL;

    exit_call = mjson_fetch_string(EXIT_CALL);
    if (exit_call) {
        len = strlen(exit_call) + 10;
        buf = malloc(len);
        if (!buf)
            return -1;
        snprintf(buf, len, "%s %s", exit_call, result == 0 ? "pass" : "failed");
        if ((pid = fork()) < 0) {
            ret = -1;
            goto out;
        } else if (pid == 0) { //child
            execl("/bin/sh", "/bin/sh", "-c", buf, NULL);
            _exit(-127);
        } else { //father
            waitpid(pid, &status, 0);
            ret = WEXITSTATUS(status);
        }
    }
out:
    free(buf);
    return ret;
}

int dragonmat_after_all_exit(void)
{
    int exit = mjson_fetch_boolean(EXIT_WHEN_END);
    if (exit < 0)
        exit = FALSE;
    DEBUG(DATA, "outlog_dragonmat: exit_when_end : %d\n", exit);

    if (exit == FALSE) {
        DEBUG(BASE, "Don't exit tt\n");
        remove(EXIT_WHEN_END_CONTROL);
        if (mkfifo(EXIT_WHEN_END_CONTROL, 0666) < 0) {
            ERROR("mkfifo %s failed - %s\n", EXIT_WHEN_END_CONTROL, strerror(errno));
            return -1;
        }
        int fd = open(EXIT_WHEN_END_CONTROL, O_RDONLY);
        if (fd < 0) {
            ERROR("open %s failed - %s\n", EXIT_WHEN_END_CONTROL, strerror(errno));
            return -1;
        }
        char control = 0;
        while (1) {
            if (read(fd, &control, 1) != 1) {
                ERROR("read %s failed - %s\n", EXIT_WHEN_END_CONTROL, strerror(errno));
                close(fd);
                return -1;
            }
            if (control == '1') {
                close(fd);
                DEBUG(BASE, "Exit outlog-dragonmat now\n");
                return 0;
            }
        }
    }
    return 0;
}

int dragonmat_after_all(struct list_head *TASK_LIST)
{
    int num;
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
    int result = 0, ret = 0;
    struct task *task = NULL;

    if (connected == FALSE)
        return 0;

    list_for_each_entry(task, TASK_LIST, lnode) {
        for (num = 0; num < task->env.limit.run_times; num++) {
            if (task->result[num] != 0) {
                result = task->result[num];
                break;
            }
        }
        if (result != 0)
            break;
    }

    msgs.msgtype = (long)sendTestEnd_fix(result == 0 ? PASS : FAILED, "null", msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("%s: reply failed! - %s\n", __func__, strerror(errno));
        ret = -1;
        goto out;
    }

    dragonmat_after_all_call(result);
    dragonmat_after_all_exit();

out:
    remove(EXIT_WHEN_END_CONTROL);
    return ret;
}

void *wait_connected(void *arg)
{
    if (waitSocketConnect() < 0)
        pthread_exit("wait Socket Connect failed");
    connected = TRUE;
    pthread_exit("dragonmat connected");
}

int dragonmat_init(void)
{
    connected = FALSE;

    if (init_entry() < 0) {
        ERROR("init entry of socket_db failed\n");
        return -1;
    }

    int wait_till_connected = mjson_fetch_boolean(WAIT_TILL_CONNECTED);
    if (wait_till_connected < 0)
        wait_till_connected = 0;
    DEBUG(DATA, "outlog_dragonmat: wait_till_connected : %d\n", wait_till_connected);

    pthread_t tid;
    if (pthread_create(&tid, NULL, wait_connected, NULL) != 0) {
        ERROR("create pthread to wait connected failed\n");
        return -1;
    }

    char *ret;
    if (wait_till_connected == TRUE && connected != TRUE) {
        DEBUG(BASE, "waiting dragonmat connected\n");
        pthread_join(tid, (void **)&ret);
        if (connected == TRUE)
            DEBUG(BASE, "%s\n", ret);
        else
            ERROR("%s\n", ret);
    } else {
        pthread_detach(tid);
    }

    return 0;
}


/* assignment to struct fileInfo with filepath */
static struct fileInfo dragonmat_fileinit(const char *file_dir, const char *file_base)
{
    struct fileInfo file;
    strcpy(file.filename, file_base);
    strcpy(file.filedir, file_dir);
    file.size = 0;
    file.compress = 0;
    file.checksum = 0;
    return file;
}

static int dragonmat_filesnd(const char *filepath, const char *testcase, const char *tips, msgstru *msgs)
{
    int ret = -1;
    struct fileInfo file;
    cutpath cp = cut_path(filepath);
    const int len_pathdir_0 = strlen(cp.dir) + 1;
    const int len_pathbase = strlen(cp.base);
    char *path = (char *)malloc(sizeof(char) * 256);
    memset(path, '\0', sizeof(char) * 256);
    strncpy(path, cp.dir, len_pathdir_0);

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(cp.dir)) == NULL) {
        ERROR("open dir: %s failed! - %s\n", cp.dir, strerror(errno));
        goto err;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if(strncmp(cp.base, dirp->d_name, len_pathbase) != 0)
            continue;

        file = dragonmat_fileinit(path, dirp->d_name);
        msgs->msgtype = (long)uploadFileRequest_fix(&file, testcase, tips, msgs->msgtext);
        if (dragonmat_reply(msgs) < 0) {
            ERROR("dragonmat_filesnd: upload file failed! - %s\n", strerror(errno));
            goto err;
        }

        if (picsnd == 1) {
//          msgs->msgtype = (long)sendShowResRequest_fix(&file, "image", testcase, tips, msgs->msgtext);
            memset(msgs->msgtext, 0, 2048);
            msgs->msgtype = (long)sendCMDselect_fix(cp.base, tips, dirp->d_name, TIMEOUT, msgs->msgtext);
            if (dragonmat_reply(msgs) < 0){
                ERROR("dragonmat_filesnd: show image failed! - %s\n", strerror(errno));
                goto err;
            }
            ret = getReturn_fix(msgs->msgtext) == 1 ? 0 : 1;
        }
    }
    if (ret < 0)
        ret = 0;
err:
    closedir(dp);

    free(cp.dir);
    free(path);
    return ret;
}

static int dragonmat_filerecv(const char *filename, const char *testcase, const char *tips, msgstru *msgs)
{
    int ret = -1;
    struct fileInfo file;

    file = dragonmat_fileinit("/tmp/", filename);
    msgs->msgtype = (long)downloadFileRequest_fix(&file, testcase, tips, msgs->msgtext);
    if (dragonmat_reply(msgs) < 0) {
        ERROR("dragonmat_filesnd: upload file failed! - %s\n", strerror(errno));
        goto err;
    }
	ERROR("msgs->msgtext:%s\n", msgs->msgtext);
    ret = getReturn_fix(msgs->msgtext) ? 0 : -1;
	ERROR("ret:%d\n", ret);
err:
    return ret;
}

/*
 * FIXME: dragonmat_ask interface are not complete.
 * It will be achieve until libsocket_db add function
 * that get user input string on dragonmat
 */
static int dragonmat_ask(const char *testcase, const char *tips, char *reply, int len)
{

    int ret = -1;

    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
    cutpath cp = cut_path(testcase);

    msgs.msgtype = (long)sendEdit_fix(cp.base, tips, reply, cp.base, TIMEOUT, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("dragonmat_ask: reply failed! - %s\n", strerror(errno));
        goto err;
     }
    ret = getValue_fix(msgs.msgtext, "editvalue", reply, &len) == 1 ? 0 : -1;

err:
    free(cp.dir);
    return ret;
}

/*
 * send scan CMD to user, people scan key
 * on dragonmat.
 * @testcase: testcase name
 * @tips: tips message to user
 * @key : key to user
 *
 * return: 1(Yes) / 0(No)
 */
static int dragonmat_scan(const char *testcase, const char *tips, const char *key)
{
    int ret = -1;

    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*  msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_istrue: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/
    msgs.msgtype = (long)sendCMDscan_fix(cp.base, tips, key, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("dragonmat_istrue: reply failed! - %s\n", strerror(errno));
        goto err;
    }

    ret = getReturn_fix(msgs.msgtext) == 1 ? true : false;
err:
//  free(msgs);
    free(cp.dir);
    return ret;
}


/*
 * send select CMD to user, people will choose (Yes/No)
 * on dragonmat.
 * @testcase: testcase name
 * @tips: tips message to user
 *
 * return: 1(Yes) / 0(No)
 */
static int dragonmat_istrue(const char *testcase, const char *tips)
{
    int ret = -1;

    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*  msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_istrue: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/
    msgs.msgtype = (long)sendCMDselect_fix(cp.base, tips, "", TIMEOUT, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("dragonmat_istrue: reply failed! - %s\n", strerror(errno));
        goto err;
    }

    ret = getReturn_fix(msgs.msgtext) == 1 ? true : false;
err:
//  free(msgs);
    free(cp.dir);
    return ret;
}
/*
 * send tips message to user.
 * @testcase: testcase name
 * @tips: tips message to user
 *
 * return: -1(failed) / 0(success)
 */
static int dragonmat_tips(const char *testcase, const char *tips)
{
    int ret = -1;

    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*
    msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_tips: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/
    msgs.msgtype = (long)sendTip_fix(cp.base, tips, msgs.msgtext);
    if (dragonmat_reply(&msgs) < 0) {
        ERROR("dragonmat_ttips: reply failed! - %s\n", strerror(errno));
        goto err;
    }

    ret = 0;
err:
//  free(msgs);
    free(cp.dir);
    return ret;
}

/*
 * upload file from device to PC and the file will
 * be stored in dragonmat_xxx/result_dir/0/ directory.
 * @testcase: testcase name
 * @filepath: the file directory on device
 * @tips: tips message to user
 *
 * return: -1(failed) / 0(success)
 */
static int dragonmat_upfile(const char *testcase, const char *filepath, const char *tips)
{
    int ret = -1;

    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*
    msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_tips: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/
    if ((dragonmat_filesnd(filepath, cp.base, tips, &msgs)) < 0) {
        ERROR("dragonmat_upfile: filesnd failed! - %s\n", strerror(errno));
        goto err;
    }

    ret = 0;
err:
//  free(msgs);
    free(cp.dir);
    return ret;
}

/*
 * download file from pc to device and the file will
 * be stored in dragonmat_xxx/result_dir/0/ directory.
 * @testcase: testcase name
 * @filepath: the file directory on device
 * @tips: tips message to user
 *
 * return: -1(failed) / 0(success)
 */
static int dragonmat_downloadfile(const char *testcase, const char *filename, const char *tips)
{
    int ret = -1;

    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*
    msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_tips: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/
    if ((dragonmat_filerecv(filename, cp.base, tips, &msgs)) < 0) {
        ERROR("dragonmat_downloadfile: dragonmat_filerecv failed! - %s\n", strerror(errno));
        goto err;
    }

    ret = 0;
err:
//  free(msgs);
    free(cp.dir);
    return ret;
}

/*
 * 1. upload image from device to PC
 * 2. show image
 * 3. ask people
 * @testcase: testcase name
 * @filepath: the image directory on device
 * @tips: tips message to user
 *
 * return: 1(Yes) / 0(No)
 */
static int dragonmat_showimg(const char *testcase, const char *filepath, const char *tips)
{
    int ret = -1;
    picsnd = 1;
    cutpath cp = cut_path(testcase);
    msgstru msgs;
    memset(&msgs, 0, sizeof(msgstru));
/*
    msgstru *msgs = (msgstru *)malloc(sizeof(msgstru));
    if (NULL == msgs) {
        ERROR("dragonmat_tips: malloc failed! - %s\n", strerror(errno));
        goto err;
    }
*/

    /* get specify format filepath */
    if ((ret = dragonmat_filesnd(filepath, cp.base, tips, &msgs)) < 0){
        ERROR("dragonmat_showimg: filesnd failed! - %s\n", strerror(errno));
        goto err;
    }

err:
//  free(msgs);
    picsnd = 0;
    free(cp.dir);

    return ret;
}

void module_init(void)
{
    if (dragonmat_init() < 0)
        return;

    outlog_register_ex(
        dragonmat_before_all,
        dragonmat_before_one,
        NULL,
        NULL,
        dragonmat_after_one_end,
        dragonmat_after_all);

    interact_register(
        dragonmat_ask,
        dragonmat_tips,
        dragonmat_istrue,
        dragonmat_scan,
        dragonmat_upfile,
        dragonmat_downloadfile,
        dragonmat_showimg);
}
