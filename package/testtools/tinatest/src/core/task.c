#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "mjson.h"
#include "mcollectd.h"
#include "syskey.h"
#include "shmem.h"
#include "sync.h"
#include "task.h"
#include "fileops.h"
#include "outlog.h"

bool reboot_mode;        //标志重启模式
int reboot_run_times;   //重启的测试用例执行次数
int rebooted_times;     //重启的测试用例已经重启次数
char *reboot_keypath;   //引起重启的测试用例路径
char *tt_argv;          //调用tinatest的输入参数
char *fstdout, *fstderr;//标准输出/错误的文件

volatile int running_cnt;   //记录正在执行的测试用例个数
volatile int end_cnt;       //记录已执行的测试用例个数
int tasks_cnt;              //记录有效任务个数
int tinatest_pid;           //TinaTest主进程pid
int tinatest_limit_pid;     //TinaTest限制总时间的pid
struct shmem_tinatest *shmem_tt;         //TinaTest共享内存
static pthread_mutex_t running_cnt_lock; //修改正在执行个数的变量锁
static pthread_mutex_t do_task_lock;     //执行测试用例的锁
static pthread_t rtid;

struct global_env g_env; //全局环境
struct local_env l_env;  //局部环境(只有默认和/sys/local的配置,不包含测试用例定制的部分)

static struct list_head TASK_LIST; //任务列表

#define ADD 1
#define SUB 0
#define ERR -1

#define SERIAL "/dev/ttyS0"

// *========================================
// *功能: 从时间数组转换为对应秒
// *参数: 时间数组
// *返回: 秒
// *========================================
inline int task_convert_time(char **str_array, int cnt) {
    int sec = 0;
    int index = 0;
    for (; index < cnt; index++) {
        switch (index) {
        case 0: //sec
            sec += atoi(str_array[index]);
            break;
        case 1: //min
            sec += atoi(str_array[index]) * 60;
            break;
        case 2: //hour
            sec += atoi(str_array[index]) * 60 * 60;
            break;
        case 3: //day
            sec += atoi(str_array[index]) * 24 * 60 * 60;
        }
    }
    return sec > 0 ? sec : 0;
}

// *========================================
// *功能: 执行加载/sys/global
// *参数: genv : 保存结果的变量
// *      key  : 配置项
// *      val  : 配置值
// *      type : 配置类型
// *返回: 0: 失败
// *      1: 成功
// *========================================
int task_load_global_env_do(
        struct global_env *genv,
        char *key,
        union mdata val,
        enum mjson_type type)
{
    //limit
    if (!strcmp(key, SYSKEY_GLOBAL_LIMIT_RUN_CNT_UP_TO) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_cnt_up_to: %d => %d\n",
                genv->limit.run_cnt_up_to,
                val.m_int > 0 ? val.m_int : genv->limit.run_cnt_up_to);
        genv->limit.run_cnt_up_to =
            val.m_int > 0 ? val.m_int : genv->limit.run_cnt_up_to;
    } else if (!strcmp(key, SYSKEY_GLOBAL_LIMIT_TINATEST_RUN_TIME)
            && type == mjson_type_array) {
        DEBUG(DATA, "limit: tinatest_run_time_sec : %d => %d\n",
                genv->limit.tinatest_run_time_sec,
                task_convert_time(val.m_array, val.m_array_cnt));
        genv->limit.tinatest_run_time_sec =
            task_convert_time(val.m_array, val.m_array_cnt);
    } else {
        return 0;
    }

    return 1;
}

// *========================================
// *功能: 加载/sys/global(调用 task_load_global_env_do)
// *参数: genv : 保存结果的变量
// *      keypath : 全局配置项的路径
// *返回: 加载成功的配置项数量
// *========================================
int task_load_global_env(struct global_env *genv, const char *keypath)
{
    int ret = 0;
    DEBUG(BASE, "Loading global env\n");
    //global limit
    mjson_foreach(keypath, info_key, limit_val, limit_mtype)
        ret += task_load_global_env_do(genv, info_key, limit_val, limit_mtype);

    //global info
    return ret;
}

// *========================================
// *功能: 执行加载/sys/local
// *参数: genv : 保存结果的变量
// *      key  : 配置项
// *      val  : 配置值
// *      type : 配置类型
// *返回: 0: 失败
// *      1: 成功
// *========================================
int task_load_local_env_do(
        struct local_env *lenv,
        char *key,
        union mdata val,
        enum mjson_type type)
{
    //info
    if (!strcmp(key, SYSKEY_LOCAL_INFO_DATE) && type == mjson_type_boolean) {
        DEBUG(DATA, "info: date: %d => %d\n", lenv->info.date, val.m_boolean);
        lenv->info.date = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_INFO_RESOURCE) && type == mjson_type_boolean) {
        DEBUG(DATA, "info: resource: %d => %d\n", lenv->info.resource, val.m_boolean);
        lenv->info.resource = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_INFO_REAL_TIME_LOG) && type == mjson_type_boolean) {
        DEBUG(DATA, "info: real_time_log: %d => %d\n", lenv->info.real_time_log, val.m_boolean);
        lenv->info.real_time_log = val.m_boolean;
    }
    //limit
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_TIMES) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_times: %d => %d\n",
                lenv->limit.run_times,
                val.m_int > 0 ? val.m_int : lenv->limit.run_times);
        lenv->limit.run_times =
            val.m_int > 0 ? val.m_int : lenv->limit.run_times;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_ALONE) && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: run_alone: %d => %d\n",
                lenv->limit.run_alone, val.m_boolean);
        lenv->limit.run_alone = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_MAY_REBOOT)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: may_reboot: %d => %d\n",
                lenv->limit.may_reboot, val.m_boolean);
        lenv->limit.may_reboot = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_PARALLEL)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: run_parallel: %d => %d\n",
                lenv->limit.run_parallel, val.m_boolean);
        lenv->limit.run_parallel = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_TESTCASE_RUN_ONCE_TIME)
            && type == mjson_type_array) {
        DEBUG(DATA, "limit: testcase_run_once_time_sec: %d => %d\n",
                lenv->limit.testcase_run_once_time_sec,
                task_convert_time(val.m_array, val.m_array_cnt));
        lenv->limit.testcase_run_once_time_sec =
            task_convert_time(val.m_array, val.m_array_cnt);
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_TESTCASE_RUN_TIME)
            && type == mjson_type_array) {
        DEBUG(DATA, "limit: testcase_run_time_sec: %d => %d\n",
                lenv->limit.testcase_run_time_sec,
                task_convert_time(val.m_array, val.m_array_cnt));
        lenv->limit.testcase_run_time_sec =
            task_convert_time(val.m_array, val.m_array_cnt);
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_TIMEOUT_AS_FAILED)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: timeout_as_pass : %d => %d\n",
                lenv->limit.timeout_as_failed, val.m_boolean == false ? TASK_RESULT_PASS : SIGALRM);
        lenv->limit.timeout_as_failed = val.m_boolean == false ? TASK_RESULT_PASS : SIGALRM;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_EXIT_ONCE_FAILED)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: : exit_once_failed %d => %d\n",
                lenv->limit.exit_once_failed, val.m_boolean);
        lenv->limit.exit_once_failed = val.m_boolean;
    }
    else
        return 0;
    return 1;
}

// *========================================
// *功能: 加载/sys/global(调用 task_load_local_env_do)
// *参数: genv : 保存结果的变量
// *      keypath : 全局配置项的路径
// *返回: 加载成功的配置项数量
// *========================================
int task_load_local_env(struct local_env *lenv, const char *keypath)
{
    int ret = 0;
    if (lenv == NULL || keypath == NULL)
        return ret;
    mjson_foreach(keypath, info_key, info_val, info_mtype)
        ret += task_load_local_env_do(lenv, info_key, info_val, info_mtype);
    return ret;
}

// *========================================
// *功能: 加载/sys, 无配置则采用默认值
// *参数:
// *返回:
// *========================================
void task_init_sysenv(void)
{
    memset(&l_env, 0 , sizeof(struct local_env));
    memset(&g_env, 0 , sizeof(struct global_env));
    // run_cnt_up_to
    DEBUG(DATA, "env: set run_cnt_up_to to default %d\n",
            DEFAULT_GLOBAL_LIMIG_RUN_CNT_UP_TO);
    g_env.limit.run_cnt_up_to = DEFAULT_GLOBAL_LIMIG_RUN_CNT_UP_TO;
    // tinatest_run_time_sec
    DEBUG(DATA, "env: set tinatest_run_time_sec to default %d\n",
            DEFAULT_GLOBAL_LIMIG_TINATEST_RUN_TIME_SEC);
    g_env.limit.tinatest_run_time_sec = DEFAULT_GLOBAL_LIMIG_TINATEST_RUN_TIME_SEC;
    // run_times
    DEBUG(DATA, "env: set run_times to default %d\n", DEFAULT_LOCAL_LIMIT_RUN_TIMES);
    l_env.limit.run_times = DEFAULT_LOCAL_LIMIT_RUN_TIMES;
    // testcase_run_time_sec
    DEBUG(DATA, "env: set testcase_run_time_sec to default %d\n",
            DEFAULT_LOCAL_TESTCASE_RUN_TIME_SEC);
    l_env.limit.testcase_run_time_sec = DEFAULT_LOCAL_TESTCASE_RUN_TIME_SEC;
    // testcase_run_once_time_sec
    DEBUG(DATA, "env: set testcase_run_once_time_sec to default %d\n",
            DEFAULT_LOCAL_TESTCASE_RUN_ONCE_TIME_SEC);
    l_env.limit.testcase_run_once_time_sec = DEFAULT_LOCAL_TESTCASE_RUN_TIME_SEC;
    //load sys env
    DEBUG(BASE, "env: load %s\n", MJSON_GLOBAL_ENV_LIMIT);
    task_load_global_env(&g_env, MJSON_GLOBAL_ENV_LIMIT);
    DEBUG(BASE, "env: load %s\n", MJSON_LOCAL_ENV_LIMIT);
    task_load_local_env(&l_env, MJSON_LOCAL_ENV_LIMIT);
    DEBUG(BASE, "env: load %s\n", MJSON_LOCAL_ENV_INFO);
    task_load_local_env(&l_env, MJSON_LOCAL_ENV_INFO);
    //init collectd (sys/global/info)
    struct mjson_value val = mjson_fetch(COLLECTD_PATH);
    if (val.type == mjson_type_object)
        g_env.info.collectd = true;
    else
        g_env.info.collectd = false;
    DEBUG(DATA, "env: set collectd enable to %d\n", g_env.info.collectd);
    // init outlog (sys/global/info)
    val = mjson_fetch(OUTLOG_PATH);
    if (val.type == mjson_type_object)
        g_env.info.outlog = true;
    else
        g_env.info.outlog = false;
    DEBUG(DATA, "env: set outlog enable to %d\n", g_env.info.outlog);
}

// *========================================
// *功能: 删除/etc/init.d/rcS最后一行
// *参数:
// *返回:
// *========================================
void task_clean_rcS(const char *fpath)
{
    delete_lastline(fpath);
}

// *========================================
// *功能: 清除重启的所有缓存
// *参数:
// *返回:
// *========================================
void task_clean_reboot(void)
{
    struct stat check_init;
    if(lstat(CHECK_INIT,&check_init)<0) {
        ERROR("lstat failed! - %s\n", strerror(errno));
    }
    if((check_init.st_mode & S_IFMT)==S_IFLNK) {
        task_clean_rcS(REBOOT_INIT_BUSYBOX);
    }
    else
    {
        unlink(REBOOT_INIT);
        unlink(REBOOT_INIT_LINK);
    }
    unlink(REBOOT_LOG);
    unlink(REBOOT_STATUS);
    reboot_mode = false;
    reboot_run_times = 0;
    rebooted_times = 0;
    reboot_keypath = NULL;
}

// *========================================
// *功能: 创建重启脚本文件
// *参数:
// *返回: 0: 成功 ; 1: 失败
// *========================================
int task_create_fboot(void)
{
    int ret = -1;
    FILE *fp = NULL;
    struct stat check_init;
    if(lstat(CHECK_INIT,&check_init)<0){
        ERROR("lstat failed! - %s\n", strerror(errno));
        return ret;
    }
    if((check_init.st_mode & S_IFMT)==S_IFLNK) {
        fp = fopen(REBOOT_INIT_BUSYBOX, "at+");
        if (fp == NULL) {
            ERROR("open %s failed\n", REBOOT_INIT_BUSYBOX);
            goto out;
        }
        fprintf(fp, "\ntinatest -r %s %s", reboot_keypath, tt_argv);
        DEBUG(DATA, "reboot call : %s\n", tt_argv);
        fflush(fp);
    }
    else
    {
        if (!access(REBOOT_INIT, F_OK) || !access(REBOOT_INIT_LINK, F_OK)) {
            remove(REBOOT_INIT);
            remove(REBOOT_INIT_LINK);
        }
        fp = fopen(REBOOT_INIT, "w");
        if (fp == NULL) {
            ERROR("open %s failed\n", REBOOT_INIT);
            goto out;
        }
        setbuf(fp, NULL);
        fprintf(fp, "#!/bin/sh /etc/rc.common\n");
        fprintf(fp, "\n");
        fprintf(fp, "START=99\n");
        fprintf(fp, "DEPEND=fstab\n");
        fprintf(fp, "\n");
        fprintf(fp, "start() {\n");
        fprintf(fp, "    tinatest -r %s %s\n", reboot_keypath, tt_argv);
        fprintf(fp, "}\n");
        fprintf(fp, "\n");
        fprintf(fp, "stop() {\n");
        fprintf(fp, "    return 0\n");
        fprintf(fp, "}\n");

        DEBUG(DATA, "reboot call : %s\n", tt_argv);
        fflush(fp);

        chmod(REBOOT_INIT, 0755);

        if (symlink(REBOOT_INIT, REBOOT_INIT_LINK) != 0) {
            ERROR("open %s failed\n", REBOOT_INIT);
            goto out;
        }
    }
    ret = 0;
out:
    fsync(fileno(fp));
    fclose(fp);
    return ret;
}

// *========================================
// *功能: 初始化
// *参数:
// *返回: -1: 失败 ; -127: 无需stdin ; >0: 文件描述符
// *注意: 会被新进程调用,由于此程序时多线程多进程应用,所以此函数只能使用线程安全函数
// *========================================
int task_init_fdin(struct task *task)
{
    int ret = -1;
    if (task->input_t == TASK_STDIN) {
        char **pp = (char **)task->input.array;
        if (pp == NULL)
            goto out;
        int len_array = task->input.array_cnt;
        int fd[2] = {-1, -1};

        if (pipe(fd))
            goto out;
        for (int cnt = 0; cnt < len_array; cnt++) {
            write(fd[1], pp[cnt], strlen(pp[cnt]));
            write(fd[1], "\n", 1);
        }
        close(fd[1]);
        ret = fd[0];
    } else if (task->input_t == TASK_FSTDIN) {
        char *p = (char *)task->finput;
        if (p == NULL)
            goto out;
        if (!access(p, R_OK)) {
            int fd_f = open(p, O_RDONLY);
            if (fd_f < 0)
                goto out; //open failed
            ret = fd_f;
        }
    } else {
        ret = -127; //not -1 but less than 0 means no need to set stdin
        goto out;
    }

out:
    return ret;
}

// *========================================
// *功能: 执行添加测试用例的核心部分
// *参数: keypath: 测试用例路径
// *      command: 调用测试用例的命令
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_add_do(const char *keypath, const char *command)
{
    int ret = -1;
    char *buf = NULL, *p = NULL, *pp[ARRAY_MAX];
    int len = strlen(keypath);
    struct task *new = malloc(sizeof(struct task));
    if (new == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out;
    }
    memset(new, 0 , sizeof(struct task));

    //keypath
    new->keypath = strdup(keypath);
    if (new->keypath == NULL) {
        ERROR("%s: strdup failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    //task_env
    memcpy(&new->env, &l_env, sizeof(struct local_env)); //base on sysenv
    task_load_local_env(&new->env, keypath); //and load local env
    //command
    new->command = strdup(command);
    if (new->command == NULL) {
        ERROR("%s: strdup failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    //*result
    new->result = calloc(new->env.limit.run_times, sizeof(int));
    if (new->result == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    //*pid
    new->pid = calloc(new->env.limit.run_times, sizeof(pid_t));
    if (new->pid == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    //*killed
    new->killed = calloc(new->env.limit.run_times, sizeof(pid_t));
    if (new->killed == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    // pid_limit
    new->pid_limit = 0;
    //*begin_time && *end_time
    new->begin_time = calloc(new->env.limit.run_times, sizeof(time_t));
    if (new->begin_time == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    new->end_time = calloc(new->env.limit.run_times, sizeof(time_t));
    if (new->end_time == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        goto out1;
    }
    //may_reboot & run_alone
    if (new->env.limit.may_reboot == true)
        new->env.limit.run_alone = true;
    //logpath
    if (new->env.info.real_time_log == true) {
        new->logpath = NULL;
        DEBUG(BASE, "%s: real_time_log without logpath\n", new->keypath);
    } else if (new->env.limit.may_reboot == true) {
        new->logpath = strdup(REBOOT_LOG);
    } else {
        int max_len;

        p = buf = strdup(keypath);
        if (buf == NULL) {
            ERROR("%s: strdup failed - %s\n", keypath, strerror(errno));
            goto out1;
        }
        while ((p = strchr(p, '/')) != NULL)
            *p = '-';

        max_len = strlen(keypath) + strlen("/tmp/tt") + strlen(".out") + 1;
        new->logpath = malloc(max_len);
        if (new->logpath == NULL) {
            ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
            goto out1;
        }

        snprintf(new->logpath, max_len, "/tmp/tt%s.out", buf);
    }
    DEBUG(BASE, "%s: logpath is %s\n", new->keypath, new->logpath);
    //resource
    if (new->env.info.resource == true) {
        new->res = malloc(sizeof(struct rusage));
        if (new->res == NULL) {
            ERROR("%s malloc failed - %s\n", new->keypath, strerror(errno));
            goto out1;
        }
        memset(new->res, 0, sizeof(struct rusage));
    }
    //input
    buf = realloc(buf, len + 30);
    snprintf(buf, len + 30, "%s/%s", new->keypath, SYSKEY_TASK_STDIN);
    if ((new->input.array_cnt = mjson_fetch_array(buf, pp, ARRAY_MAX))) {
        new->input.array = malloc(sizeof(char *) * new->input.array_cnt);
        memcpy(new->input.array, pp, sizeof(char *) * new->input.array_cnt);
        new->input_t = TASK_STDIN;
    } else {
        snprintf(buf, len + 30, "%s/%s", new->keypath, SYSKEY_TASK_FSTDIN);
        if ((p = mjson_fetch_string(buf)) != NULL) {
            new->finput = p;
            new->input_t = TASK_FSTDIN;
        } else {
            new->finput = NULL;
            new->input.array = NULL;
            new->input.array_cnt = 0;
            new->input_t = TASK_NO_INPUT;
        }
    }
    //share memory
    new->shmem = shmem_probe(new->keypath);
    if (new->shmem == NULL)
        goto out1;
    //status
    new->status = TASK_WAIT;
    //lnode
    list_add_tail(&new->lnode, &TASK_LIST);

    ret = 0;
    goto out;

out1:
    free(new->res);
    free(new->logpath);
    free(new->end_time);
    free(new->begin_time);
    free(new->killed);
    free(new->pid);
    free(new->result);
    free(new->command);
    free(new->keypath);
    free(new);
out:
    free(buf);
    return ret;
}

// *========================================
// *功能: 递归遍历所有节点,判断路径节点是否有效,调用_task_add_do添加
// *参数: keypath: 测试用例路径
// *返回: 添加测试用例个数
// *========================================
int task_add(const char *keypath)
{
    int ret = 0;

    if (*keypath != '/') {
        ERROR("%s is not absolute path, skip it\n", keypath);
        return 0;
    }
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_error) {
        ERROR("%s: fetch failed, skip it\n", keypath);
        return 0;
    }

    int len = strlen(keypath) * 2 + 30;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed - %s\n", keypath, strerror(errno));
        return 0;
    }
    memset(buf, 0, len);
    if (!strcmp(keypath, "/"))
        snprintf(buf, len, "/%s", SYSKEY_TASK_ENABLE);
    else
        snprintf(buf, len, "%s/%s", keypath, SYSKEY_TASK_ENABLE);
    if (mjson_fetch_boolean(buf) == FALSE){ //disable
        DEBUG(BASE, "%s is disable, skip it and it's childs\n", buf);
        goto out;
    }

    if (!strcmp(keypath, "/"))
        snprintf(buf, len, "/%s", SYSKEY_TASK_COMMAND);
    else
        snprintf(buf, len, "%s/%s", keypath, SYSKEY_TASK_COMMAND);

    char *command = NULL;
    if ((command = mjson_fetch_string(buf)) != NULL) { //find it
        struct task *task = NULL;
        list_for_each_entry(task, &TASK_LIST, lnode) {
            if (!strcmp(task->keypath, keypath))
                goto out;
        }
        if (reboot_mode == true &&
                TASK_LIST.next == &TASK_LIST &&
                TASK_LIST.prev == &TASK_LIST) {
            if (strcmp(reboot_keypath, keypath)) {
                DEBUG(BASE, "in reboot mode, waiting %s not %s, skip it\n", reboot_keypath, keypath);
                goto out;
            }
        }
        DEBUG(BASE, "fetch %s, try to add it to list\n", keypath);
        if (task_add_do(keypath, command) == 0)
            ret = 1;
    } else {
        mjson_foreach(keypath, key, val, mtype) {
            if (mtype == mjson_type_object) {
                if (!strcmp(keypath, "/"))
                    snprintf(buf, len, "/%s", key);
                else
                    snprintf(buf, len, "%s/%s", keypath, key);
                ret += task_add(buf);
            }
        }
    }

out:
    free(buf);
    return ret;
}

// *========================================
// *功能: 与task_add_do对应,清空指定测试用例申请的资源
// *参数: task: 测试用例
// *返回:
// *========================================
void task_del_do(struct task *task)
{
    remove(task->logpath);
    free(task->pid);
    task->pid = NULL;
    free(task->begin_time);
    task->begin_time = NULL;
    free(task->end_time);
    task->end_time = NULL;
    free(task->logpath);
    task->logpath = NULL;
    free(task->result);
    task->result = NULL;
    free(task->res);
    task->res = NULL;
    free(task->keypath);
    list_del(&(task->lnode));
    free(task);
}

// *========================================
// *功能: 清空路径下所有测试用例申请的资源 (调用task_del_do)
// *参数: 测试用例的路径
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_del(const char *keypath)
{
    if (TASK_LIST.prev == NULL && TASK_LIST.next == NULL)
        return -1;

    struct task *pre = NULL, *next = NULL;
    list_for_each_entry_safe(pre, next, &TASK_LIST, lnode) {
        if (!strcmp(pre->keypath, keypath))
            task_del_do(pre);
    }
    return 0;
}

// *========================================
// *功能: 修改变量running_cnt/end_cnt
// *参数: ADD & SUB
// *返回:
// *========================================
void task_mod_running_cnt(int num)
{
    pthread_mutex_lock(&running_cnt_lock);
    switch (num) {
        case ADD: {
                running_cnt++;
                DEBUG(BASE, "add running_cnt\n");
                DEBUG(DATA, "running_cnt: %d\n", running_cnt);
                break;
            }
        case SUB: {
                DEBUG(BASE, "sub running_cnt\n");
                running_cnt--;
                DEBUG(DATA, "running_cnt: %d\n", running_cnt);
            }
        case ERR: {
                end_cnt++;
                DEBUG(DATA, "runned_cnt: %d\n", end_cnt);
                break;
            }
        default: break;
    }
    pthread_mutex_unlock(&running_cnt_lock);
}

// *========================================
// *功能: TinaTest退出的清理
// *参数:
// *返回:
// *========================================
void task_quit_clean(void)
{
    // mcollectd
    if (g_env.info.collectd == true)
        mcollectd_exit();
    // logfile
    struct task *task;
    list_for_each_entry(task, &TASK_LIST, lnode)
        remove(task->logpath);
    // share memory
    shmem_free();
    // tinatest limit
    kill(tinatest_limit_pid, SIGTERM);
}

// *========================================
// *功能: 中止信号处理
// *参数: sig: 信号编号
// *返回:
// *========================================
void task_signal_quit(int sig)
{
    if (getpid() == tinatest_pid) {
        shmem_tt->quit = true;
        struct task *task;
        list_for_each_entry(task, &TASK_LIST, lnode)
            kill(-abs(task->shmem->pgid), SIGKILL);
    } else {
        exit(sig);
    }
}

// *========================================
// *功能: 测试用例为may_reboot,则调用此函数初始化重启相关的东西
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_init_reboot_mode(struct task *task)
{
    FILE *fp = NULL;
    //only option -r will set reboot_mode
    //if reboot_mode is set, it means it had rebooted before or had run demo once at least
    if (reboot_mode == true) { //old task (may reboot)
        DEBUG(DATA, "%s: task.run_times: %d => %d\n",
                task->keypath, task->run_times, reboot_run_times + 1);
        task->run_times = reboot_run_times;
        DEBUG(DATA, "%s: task.rebooted_times: %d => %d\n",
                task->keypath, task->rebooted_times, rebooted_times);
        task->rebooted_times = rebooted_times;
    } else { //new task (may reboot)
        DEBUG(BASE, "%s may reboot and is the first time to run\n", task->keypath);
        reboot_keypath = task->keypath;
        reboot_run_times = task->run_times;
        if (task_create_fboot() != 0) {
            ERROR("create %s(%s) failed, skip it\n", REBOOT_INIT, REBOOT_INIT_LINK);
            goto quit_reboot;
        }
    }
    //refresh or new write to REBOOT_STATUS
    if ((fp = fopen(REBOOT_STATUS, "w")) == NULL) {
        ERROR("%s: open failed, skip it\n", REBOOT_STATUS);
        goto quit_reboot;
    }
    DEBUG(BASE, "%s: write to %s - run times: %d; reboot times: %d; "
            "fstdout: %s; fstderr: %s\n", task->keypath, REBOOT_STATUS,
            task->run_times, task->rebooted_times, fstdout, fstderr);
    if (fprintf(fp, "%d\n%d\n%s\n%s",
                task->run_times, task->rebooted_times, fstdout, fstderr) < 0 ) {
        ERROR("%s: write %s failed, skip it\n",task->keypath, REBOOT_STATUS);
        goto close_fp;
    }

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    return 0;
close_fp:
    fclose(fp);
quit_reboot:
    task_clean_reboot();
    return -1;
}

// *========================================
// *功能: 创建新进程作为整个TinaTest时间控制进程
// *参数:
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_limit_tinatest()
{
    // test process existed
    if (g_env.limit.tinatest_run_time_sec <= 0)
        return 0;

    if ((tinatest_limit_pid = fork()) < 0) {
        ERROR("fork for tinatest limit failed - %s\n", strerror(errno));
        return 1;
    } else if (tinatest_limit_pid == 0) { //child
        // multi thread and multi process
        // !!! should only use signal-safe functions
        // !!! CAN'T use the series of printf
        // !!! see more signal-safe functions by : man 7 signal
        sleep(g_env.limit.tinatest_run_time_sec);
        kill(0, SIGALRM);
        _exit(-SIGALRM);
    }

    DEBUG(DATA, "tinatest_run_time_sec: %d; pid: %d\n",
            g_env.limit.tinatest_run_time_sec, tinatest_limit_pid);
    return 0;
}

// *========================================
// *功能: 创建新进程作为整个测试用例的控制进程
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_limit_tinatest_all(struct task *task)
{
    // test process existed
    if (task->pid_limit != 0 && kill(task->pid_limit, 0) == 0)
        return 0;

    if ((task->pid_limit = fork()) < 0) {
        ERROR("%s: fork for limit failed - %s\n", task->keypath, strerror(errno));
        return 1;
    } else if (task->pid_limit == 0) { //child
        // multi thread and multi process
        // !!! should only use signal-safe functions
        // !!! CAN'T use the series of printf
        // !!! see more signal-safe functions by : man 7 signal
        sleep(task->env.limit.testcase_run_time_sec);
        task->shmem->quit = true;
        if (task->shmem->pgid > 0)
            kill(-task->shmem->pgid, SIGALRM);
        _exit(0);
    }

    DEBUG(DATA, "%s: testcase_run_time_sec: %d; pid_limit %d\n", task->keypath,
            task->env.limit.testcase_run_time_sec, task->pid_limit);
    return 0;
}

// *========================================
// *功能: 创建新进程作为测试用例每次执行的控制进程
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_limit_tinatest_once(struct task *task)
{
    int pid;
    if ((pid = fork()) < 0) {
        ERROR("%s: fork for limit failed - %s\n", task->keypath, strerror(errno));
        return -1;
    } else if (pid == 0) { //child
        // multi thread and multi process
        // !!! should only use signal-safe functions
        // !!! CAN'T use the series of printf
        // !!! see more signal-safe functions by : man 7 signal
        if (setpgid(getpid(), getpid()) < 0)
            _exit(127);
        sleep(task->env.limit.testcase_run_once_time_sec);
        kill(-getpid(), SIGALRM);
        _exit(0);
    }
    if (setpgid(pid, pid) < 0)
        return -1;
    task->shmem->pgid = pid;
    DEBUG(DATA, "%s: pgid: 0 => %d\n", task->keypath, task->shmem->pgid);

    return 0;
}

// *========================================
// *功能: 保存日志到文件同时实时输出日志到屏幕
// *参数: task: 测试用例
// *返回: -1: 失败 ; 非零: 文件句柄
// *备注: 在明确real_time_log不保存log到/tmp后，不再使用此接口，但保留以后可能用得到
// *========================================
int task_multi_log(struct task *task)
{
    int fd[2] = {-1, -1};
    int pid;

    if (pipe(fd) < 0) {
        ERROR("%s: pipe failed - %s\n", task->keypath, strerror(errno));
        goto err;
    }

    if ((pid = fork()) < 0) {
        ERROR("%s: fork for real-time log failed - %s\n", task->keypath, strerror(errno));
        goto err;
    } else if (pid == 0) { //child
        // multi thread and multi process
        // !!! should only use signal-safe functions
        // !!! CAN'T use the series of printf
        // !!! see more signal-safe functions by : man 7 signal
        setpgid(getpid(), abs(task->shmem->pgid));

        close(fd[1]);
        int fout =  open(task->logpath, O_WRONLY | O_APPEND | O_CREAT, 00644);
        if (fout < 0)
            _exit(127);

        if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
            _exit(127);

        if (dup2(STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO)
            _exit(127);

        execl("/usr/bin/tee", "/usr/bin/tee", "-a", task->logpath, NULL);
        _exit(127);
    }
    setpgid(pid, abs(task->shmem->pgid));

    close(fd[0]);
    return fd[1];
err:
    close(fd[0]);
    close(fd[1]);
    return -1;
}

// *========================================
// *功能: 创建新进程执行测试用例
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_testcase(struct task *task)
{
    DEBUG(BASE, "%s: begin to run ...\n", task->keypath);
    do {
        //setenv is not a thread safety function, so we set it before fork
        setenv(TESTCASE_PATH, task->keypath, true);

        //record time everytime run testcase
        task->begin_time[task->run_times] = time(NULL);
        if ((task->pid[task->run_times] = fork()) < 0) {
            kill(-abs(task->shmem->pgid), SIGTERM);
            ERROR("%s: fork for testcase failed - %s\n", task->keypath, strerror(errno));
            goto err;
        } else if (task->pid[task->run_times] == 0) { //child
            // multi thread and multi process
            // !!! should only use signal-safe functions
            // !!! CAN'T use the series of printf
            // !!! see more signal-safe functions by : man 7 signal
            if (setpgid(getpid(), abs(task->shmem->pgid)) < 0)
                _exit(127);

            //stdin
            int fin = task_init_fdin(task);
            if (fin == -1)
                _exit(127);

            //stdout
            int fout = -1;
            if (task->env.info.real_time_log == true)
                //fout = task_multi_log(task);
                fout = STDERR_FILENO;
            else
                fout =  open(task->logpath, O_WRONLY | O_APPEND | O_CREAT, 00644);

            if (fout < 0)
                _exit(127);

            if (dup2(fout, STDOUT_FILENO) != STDOUT_FILENO)
                _exit(127);

            if (dup2(fout, STDERR_FILENO) != STDERR_FILENO)
                _exit(127);

            if (fin > 0 && dup2(fin, STDIN_FILENO) != STDIN_FILENO)
                _exit(127);

            execl("/bin/sh", "/bin/sh", "-c", task->command, NULL);
            _exit(127);
        }
        setpgid(ads(task->pid[task->run_times]), abs(task->shmem->pgid));
        task->run_times++;
    }while(task->env.limit.run_parallel == true &&
            task->run_times < task->env.limit.run_times);

    return 0;
err:
    return -1;
}

// *========================================
// *功能: 测试用例结束所有次数后的回收
// *参数: task: 测试用例
// *返回:
// *========================================
void task_recycle_end(struct task *task)
{
    DEBUG(BASE, "%s: recycling\n", task->keypath);
    // kill limit whole
    if (task->pid_limit > 0)
        kill(task->pid_limit, SIGTERM);
    // kill limit once if parallel
    if (task->env.limit.run_parallel)
        kill(-task->shmem->pgid, SIGTERM);
    // get date
    if (task->env.info.date == true) {
        task->end_date = time(NULL);
        DEBUG(DATA, "%s: end time : %s\n", task->keypath, ctime(&task->end_date));
    }
    // call outlog
    if (g_env.info.outlog == true)
        outlog_call_after_one_end(task);
    if (reboot_mode == true && task->env.limit.may_reboot == true) {
        DEBUG(BASE, "%s: may reboot and has done, to clean reboot file\n",
                task->keypath);
        task_clean_reboot();
        if (running_cnt > 0) {
            DEBUG(BASE, "%s: it may reboot, but it hadn't reboot before\n",
                    task->keypath);
            task_mod_running_cnt(SUB);
        }
    }
    else
        task_mod_running_cnt(SUB);
    if (running_cnt + 1 == g_env.limit.run_cnt_up_to)
        TELL_PARENT(TASK_ADD_RECYCLE_ID);
    task->status = TASK_END;
}

// *========================================
// *功能: 创建新进程执行测试用例(调用 task_do_limit / task_do_testcase)
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do(struct task *task)
{
    int ret = -1;
    pthread_mutex_lock(&do_task_lock);
    if (task->env.limit.may_reboot == true) {
        DEBUG(BASE, "%s may reboot\n", task->keypath);
        if (task_init_reboot_mode(task) < 0) {
            reboot_mode = false;
            ERROR("%s: init for reboot mode failed\n", task->keypath);
            goto out;
        }
        DEBUG(BASE, "%s: init for reboot mode successfully\n", task->keypath);
    }
    //the last times to run may-reboot task
    if (task->run_times >= task->env.limit.run_times) {
        if (reboot_mode == true) {
            DEBUG(BASE, "%s: may reboot and has done the last time\n",
                    task->keypath);
            task->result[task->run_times - 1] = 0;
            task_recycle_end(task);
        }
        ret = 0;
        task_mod_running_cnt(SUB);
        if (end_cnt == tasks_cnt)
            kill(tinatest_limit_pid, SIGTERM);
        goto out;
    }

    //save begin date
    if (task->env.info.date == true && task->begin_date == 0) {
        task->begin_date = time(NULL);
        DEBUG(DATA, "%s: begin date 0 => %s\n",
                task->keypath, ctime(&task->begin_date));
    }

    //run_testcase_limit, if existed, return 0 with nothing to do
    if (task_do_limit_tinatest_all(task) < 0)
        goto out;

    //run limit
    if (task_do_limit_tinatest_once(task) < 0)
        goto out;

    //run testcase
    if (task_do_testcase(task) < 0)
        goto out;

    DEBUG(DATA, "%s: run_times: %d => %d\n",
            task->keypath, task->run_times - 1, task->run_times);
    DEBUG(DATA, "%s: pid[%d]: 0 => %d\n",
            task->keypath, task->run_times - 1, task->pid[task->run_times - 1]);
    DEBUG(BASE, "%s: run successfully (%d/%d)\n",
            task->keypath, task->run_times, task->env.limit.run_times);

    // call outlog func
    if (g_env.info.outlog == true)
        outlog_call_after_one_begin(task);

    ret = 0;
out:
    pthread_mutex_unlock(&do_task_lock);
    return ret;
}

// *========================================
// *功能: 获取测试用例使用的资源
// *参数: task: 测试用例
// *      res: 资源
// *返回:
// *========================================
void task_get_rusage(struct task *task, struct rusage *res)
{
    task->res->ru_utime.tv_sec += res->ru_utime.tv_sec;
    task->res->ru_utime.tv_usec += res->ru_utime.tv_usec;
    task->res->ru_stime.tv_sec += res->ru_stime.tv_sec;
    task->res->ru_stime.tv_usec += res->ru_stime.tv_usec;
    task->res->ru_maxrss = (res->ru_maxrss + task->res->ru_maxrss) / 2;
    task->res->ru_minflt += res->ru_minflt;
    task->res->ru_majflt += res->ru_majflt;
    task->res->ru_inblock += res->ru_inblock;
    task->res->ru_oublock += res->ru_oublock;
    task->res->ru_nvcsw += res->ru_nvcsw;
    task->res->ru_nivcsw += res->ru_nivcsw;
}

// *========================================
// *功能: 测试用例结束一次后的回收
// *参数: task: 测试用例
// *      num: 第几次执行
// *      status: 返回的状态
// *      pid: 执行的pid
// *      res: 测试用例资源
// *返回:
// *========================================
void task_recycle_once(struct task *task, int num, int status, int pid, struct rusage *res)
{
    // <0 means finish
    task->pid[num] = -pid;
    // get usage resource
    if (task->env.info.resource == true)
        task_get_rusage(task, res);
    // get end time everytime end once testcase
    task->end_time[num] = time(NULL);
    // get exit return
    if (WIFEXITED(status))
        task->result[num] = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) {
        task->killed[num] = WTERMSIG(status);
        if (task->killed[num] == SIGALRM)
            task->result[num] = task->env.limit.timeout_as_failed;
        else
            task->result[num] = WTERMSIG(status);
        DEBUG(BASE, "%s: killed by signal %s(%d)\n", task->keypath,
                strsignal(task->killed[num]), task->killed[num]);
    }
    else {
        ERROR("%s: Invail EXIT\n", task->keypath);
        task->result[num] = -1;
    }
    DEBUG(BASE, "%s: %d times exit with %d\n",
            task->keypath, num, task->result[num]);
    if (task->result[num] != 0 && task->env.limit.exit_once_failed == true)
        task->shmem->quit = true;
    // kill other members in group
    // if run in parallel, kill pgid from task_recycle_end
    if (task->env.limit.run_parallel == false && task->shmem->pgid > 0)
        kill(-task->shmem->pgid, SIGTERM);
}

// *========================================
// *功能: 检查测试用例是否结束(执行完所有次数)
// *参数: task: 测试用例
// *返回: 1: 结束 ; 0: 没结束
// *========================================
int task_check_end(struct task *task)
{
    // if we get signal quit, we should not do testcase again
    if (shmem_tt->quit == true || task->shmem->quit == true)
        return true;
    // if parallel, check whether finish all
    if (task->env.limit.run_parallel == true) {
        int tmp;
        DEBUG(BASE, "%s: run parallel, check whether finish all\n",
                task->keypath);
        for (tmp = 0; tmp < task->run_times; tmp++) {
            if (task->pid[tmp] > 0)
                break;
        }
        if (tmp != task->run_times) {
            DEBUG(BASE, "%s: pid[%d] - %d havn't end\n",
                    task->keypath, tmp, task->pid[tmp]);
            return false;
        }
        DEBUG(BASE, "%s: run parallel, finish all\n", task->keypath);
    }
    // check whether end
    if (task->run_times < task->env.limit.run_times)
        return false;
    return true;
}

// *========================================
// *功能: 守护线程,回收死亡的子进程(调用task_recycle_once/task_recycle_end)
// *参数:
// *返回:
// *========================================
void *task_recycle(void *arg)
{
    struct rusage res;
    int status;
    pid_t pid;
    struct task *pre = NULL, *next = NULL;

    DEBUG(BASE, "recycle pthread begin\n");
    if (TELL_PARENT(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("reply to parent failed\n");
        exit(1);
    }

    while (true) {
again:
        if (end_cnt == tasks_cnt)
            goto out;
        pid = wait4(-1, &status, 0, &res);
        if (pid == -1 && errno == ECHILD) {
            sleep(1);
            continue;
        }
        DEBUG(BASE, "get death of %d\n", pid);
        // walk for all tasks
        list_for_each_entry_safe(pre, next, &TASK_LIST, lnode) {
            if (pre->status != TASK_RUN)
                continue;
            // match pgid
            if (pre->shmem->pgid == pid) {
                DEBUG(BASE, "match death of %s's limit\n", pre->keypath);
                pre->shmem->pgid = -pid;
                break;
            }
            // check in every times
            for (int num = 0; num < pre->run_times; num++) {
                DEBUG(BASE, "To match %s : pgid - %d, pid[%d] - %d\n",
                        pre->keypath, pre->shmem->pgid, num, pre->pid[num]);
                // match pid
                if (pid == pre->pid[num]) {
                    DEBUG(BASE, "match death of %s(pid:%d)\n", pre->keypath, pid);
                    // recycle once
                    task_recycle_once(pre, num, status, pid, &res);
                    // call outlog
                    if (g_env.info.outlog == true)
                        outlog_call_after_one_once(pre);
                    // check end
                    if (task_check_end(pre) == true) {
                        task_recycle_end(pre);
                    } else if (pre->env.limit.run_parallel != true) {
                        DEBUG(BASE, "%s: not enough, do again - %d/%d\n",
                                pre->keypath, pre->run_times, pre->env.limit.run_times);
                        if (task_do(pre) != 0)
                            ERROR("%s: do again failed\n", pre->keypath);
                    }
                    goto again;
                }
            }
        }
    }

out:
    // interact
    interact_exit();
    if (g_env.info.outlog == true)
        outlog_call_after_all(&TASK_LIST);
    DEBUG(BASE, "recycle pthread exit(task cnt: %d)\n", running_cnt);
    pthread_exit(0);
}

// *========================================
// *功能: 开始执行前的初始化
// *参数:
// *返回:
// *========================================
int task_begin_init(void)
{
    //get stdout/stderr
    if (fstdout == NULL) {
        fstdout = malloc(100);
        if (fstdout == NULL)
            goto err;
        if (get_fstd(STDOUT_FILENO, fstdout, 100) < 0)
            goto err;
        DEBUG(DATA, "stdout is %s\n", fstdout);
    }
    if (fstderr == NULL) {
        fstderr = malloc(100);
        if (fstderr == NULL)
            goto err;
        if (get_fstd(STDERR_FILENO, fstderr, 100) < 0)
            goto err;
        DEBUG(DATA, "stderr is %s\n", fstderr);
    }

    //init lock for running_cnt
    running_cnt = 0;
    pthread_mutex_init(&running_cnt_lock, NULL);
    pthread_mutex_init(&do_task_lock, NULL);

    // init synchronous communication between father process and child process
    if (TELL_WAIT(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("init sync failed - %s\n", strerror(errno));
        goto err;
    }

    // init collectd
    if (g_env.info.collectd == true) {
        if (mcollectd_load_json() == 0) {
            if (TELL_WAIT(COLLECTD_DO_ID) < 0) {
                ERROR("init sync failed - %s\n", strerror(errno));
                goto err;
            }
            //run global env (collectd)
            // collectd will be call by mcollectd_do
            // so, if reboot before, it should no create conf once again
            if (reboot_mode == false) {
                if (mcollectd_make_conf() < 0) {
                    ERROR("create collectd conf failed\n");
                    goto err;
                }
            }
            if (mcollectd_do() < 0) {
                ERROR("do collectd failed\n");
                goto err;
            }
            DEBUG(BASE, "start callectd successfully\n");
        } else {
            g_env.info.collectd = false;
        }
    }

    // init outlog
    if (g_env.info.outlog == true && outlog_init() < 0) {
        ERROR("init outlog failed\n");
        goto err;
    }

    // init interact (must after outlog)
    if (interact_init() < 0) {
        ERROR("init interact failed\n");
        goto err;
    }

    // catch signal
    if (SIG_ERR == signal(SIGTERM, task_signal_quit))
        goto err;
    if (SIG_ERR == signal(SIGQUIT, task_signal_quit))
        goto err;
    if (SIG_ERR == signal(SIGINT, task_signal_quit))
        goto err;
    if (SIG_ERR == signal(SIGALRM, task_signal_quit))
        goto err;

    // tinatest run time limit
    if (task_do_limit_tinatest()) {
        ERROR("create tinatest time limit process failed\n");
        goto err;
    }

    return 0;
err:
    free(fstdout);
    free(fstderr);
    return -1;
}

int task_begin(void)
{
    // initialize before begin
    if (task_begin_init() < 0)
        goto out;

    // recycle child tasks
    if (pthread_create(&rtid, NULL, task_recycle, NULL) != 0) {
        ERROR("create recycle thread failed - %s\n", strerror(errno));
        goto out;
    }
    if (WAIT_CHILD(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("wait recycle child reply failed\n");
        goto out;
    }
    DEBUG(BASE, "start recycle pthread successfully\n");

    //run task
    DEBUG(BASE, "All is READY!! Begin to run tasks in list\n");
    DEBUG(BASE, "=========================================\n");
    if (g_env.info.outlog == true)
        outlog_call_before_all(&TASK_LIST);
    struct task *task = NULL;
    list_for_each_entry(task, &TASK_LIST, lnode) {
        DEBUG(BASE, "In list node %s\n", task->keypath);
        while (running_cnt >= g_env.limit.run_cnt_up_to) {
            DEBUG(BASE, "up to max tasks running cnt (cur/max) - %d/%d, wait\n",
                    running_cnt, g_env.limit.run_cnt_up_to);
            WAIT_CHILD(TASK_ADD_RECYCLE_ID);
        }
        while (task->env.limit.run_alone == true && running_cnt > 0) {
            DEBUG(BASE, "%s wants to run alone(alive tasks %d), sleep 1s for wait\n",
                    task->keypath, running_cnt);
            sleep(1);
        }

        if (reboot_mode == true && strcmp(reboot_keypath, task->keypath)) {
            DEBUG(BASE, "%s: reboot mode and not this task reboot before, skip it\n",
                    task->keypath);
            task->status = TASK_END;
            task_mod_running_cnt(ERR);
            continue;
        }

        task->status = TASK_RUN;
        if (g_env.info.outlog == true)
            outlog_call_before_one(task);
        if (task_do(task) == 0) {
            DEBUG(BASE, "start %s successfully\n", task->keypath);
            task_mod_running_cnt(ADD);
        } else {
            ERROR("%s: do task failed, skip it\n", task->keypath);
            task->result[task->run_times] = -1;
            task->status = TASK_END;
            task_mod_running_cnt(ERR);
            continue;
        }

        //wait for end of task which want to run alone and not the last testcase
        while (task->env.limit.run_alone == true
                && running_cnt > 0
                && end_cnt + 1 != tasks_cnt) {
            DEBUG(BASE, "%s wants to run alone, so, do next before it finish, wait\n",
                    task->keypath);
            sleep(1);
        }
    }
    DEBUG(BASE, "all tasks have been added\n");
    pthread_join(rtid, NULL);
    task_quit_clean();
    return 0;

out:
    return 1;
}


int task_recover_after_reboot(char *keypath)
{
    int ret = -1;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    // record reboot_keypath
    reboot_keypath = keypath;

    if (is_existed(REBOOT_STATUS) == true) { //existed
        if ((fp = fopen(REBOOT_STATUS, "r")) == NULL) {
            ERROR("open %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        // recover reboot_run_times
        if (getline(&line, &len, fp) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        reboot_run_times = atoi(line) + 1;

        // recover rebooted_times
        if (getline(&line, &len, fp) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        rebooted_times = atoi(line) + 1;

        int bytes = len = 0;
        fstdout = fstderr = NULL;
        // recover fstdout
        if ((bytes = getline(&fstdout, &len, fp)) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        } else {
            if (fstdout[bytes - 1] == '\n')
                fstdout[bytes - 1] = '\0';
            if (is_existed(fstdout) == false) {
                ERROR("Recover fstdout after reboot: Not Found %s, replace as %s\n", fstdout, SERIAL);
                realloc(fstdout, strlen(SERIAL + 4));
                strcpy(fstdout, SERIAL);
            }
        }

        // recover fstderr
        if ((bytes = getline(&fstderr, &len, fp)) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        } else {
            if (fstderr[bytes - 1] == '\n')
                fstderr[bytes - 1] = '\0';
            if (is_existed(fstderr) == false) {
                ERROR("Recover fstdout after reboot: Not Found %s, replace as %s\n", fstderr, SERIAL);
                realloc(fstderr, strlen(SERIAL + 4));
                strcpy(fstderr, SERIAL);
            }
        }

        DEBUG(DATA, "In %s : run times %d; rebooted times %d; "
                "fstdout is %s; fstderr is %s\n",
                REBOOT_STATUS, reboot_run_times, rebooted_times, fstdout, fstderr);

        // recover stdout
        int fd = -1;
        if ((fd = open(fstdout, O_WRONLY | O_APPEND)) < 0) {
            ERROR("open %s for stdout failed - %s\n", fstdout, strerror(errno));
            goto err;
        }
        if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO) {
            ERROR("dup %s as stdout failed - %s\n", fstdout, strerror(errno));
            close(fd);
            goto err;
        }
        close(fd);
        setbuf(stdout, NULL);
        DEBUG(BASE, "redirect stdout to %s successfully\n", fstdout);

        // recover stdout
        if ((fd = open(fstderr, O_WRONLY | O_APPEND)) < 0) {
            ERROR("open %s for stderr failed - %s\n", fstderr, strerror(errno));
            goto err;
        }
        if (dup2(fd, STDERR_FILENO) != STDERR_FILENO) {
            ERROR("dup %s to stderr failed - %s\n", fstderr, strerror(errno));
            goto err;
        }
        close(fd);
        setbuf(stdout, NULL);
        DEBUG(BASE, "redirect stderr to %s successfully\n", fstderr);
        reboot_mode = true;
        ret = 0;
        goto out;
    } else { //noexisted
        ERROR("%s: missing %s\n", reboot_keypath, REBOOT_STATUS);
        goto err;
    }

err:
    reboot_mode = false;
    free(fstdout);
    free(fstderr);
out:
    free(line);
    if (fp != NULL)
        fclose(fp);
    return ret;
}

int task_init(int argc, char **argv)
{
    // init global variables
    reboot_mode = false;
    reboot_run_times = 0;
    rebooted_times = 0;
    reboot_keypath = NULL;
    tt_argv = NULL;
    fstdout = fstderr = NULL;
    running_cnt = 0;
    end_cnt = 0;
    tasks_cnt = 0;
    INIT_LIST_HEAD(&TASK_LIST);
    tinatest_pid = getpid();

    task_init_sysenv();

    // init share memory
    if ((shmem_tt = shmem_init(tasks_cnt)) == NULL) {
        ERROR("init share memory failed\n");
        return -1;
    }

    // add tasks
    if ((tt_argv = malloc(1)) == NULL) {
        ERROR("malloc for tasks_argv failed\n");
        return -1;
    }
    *tt_argv = '\0';
    if (argc == 0) {
        tasks_cnt = task_add("/");
    } else {
        int arglen = 0, i = 0;
        char *p;
        for (; i < argc; i++) {
            arglen += strlen(argv[i]);
            arglen++;
        }
        arglen++;
        tt_argv = malloc(arglen);
        if (!tt_argv) {
            ERROR("alloc for argv failed - %s\n", strerror(errno));
            return -1;
        }
        memset(tt_argv, 0, arglen);
        for (i = 0, p = tt_argv; i < argc; i++) {
            int slen = min(strlen(argv[i]), arglen - 1);
            strncpy(p, argv[i], slen);
            p[slen] = ' ';
            arglen -= slen + 1;
            p += slen + 1;
            if ((tasks_cnt += task_add(argv[i])) == 0)
                ERROR("No any ACTIVE testcase in %s\n", argv[i]);
            else
                DEBUG(BASE, "Found %d ACTIVE testcases in %s\n", tasks_cnt, argv[i]);
        }
        tt_argv[strlen(tt_argv) - 1] = '\0';
        DEBUG(DATA, "tinatest argv is |%s|\n", tt_argv);
    }
    DEBUG(DATA, "cnt of active testcase : %d\n", tasks_cnt);

    return tasks_cnt;
}
