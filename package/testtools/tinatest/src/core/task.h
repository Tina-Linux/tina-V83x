#ifndef __MTASK_H
#define __MTASK_H
#include <libubox/list.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include "shmem.h"

/* ==========  about log  ============= */
//print log in a colourfull way
#define COLOUR_LOG
//0: disable all debug log
//1: process log - show the process of tinatest
//2: data log - when key data change, show it
//>2: all log
#define DEBUG_LEVEL 0
#include "log.h"
/* ==========  end ============= */

#define MJSON_GLOBAL_ENV_INFO "/sys/global/info"
#define MJSON_GLOBAL_ENV_LIMIT "/sys/global/limit"
#define MJSON_LOCAL_ENV_INFO "/sys/local/info"
#define MJSON_LOCAL_ENV_LIMIT "/sys/local/limit"

#define MODULE_LIB_PATH "/usr/lib/tt-module"
#define MODULE_INIT_FUNC "module_init"

#define TESTCASE_PATH "TESTCASE_PATH"

//these variables work for test which will reboot
#define REBOOT_INIT "/etc/init.d/tt"
#define REBOOT_INIT_LINK "/etc/rc.d/S99tt"
#define REBOOT_INIT_BUSYBOX "/etc/init.d/rcS"
#define REBOOT_LOG "/mnt/UDISK/tinatest.reboot.log"
#define REBOOT_STATUS "/mnt/UDISK/tinatest.reboot.status"
#define CHECK_INIT "/sbin/init"

#define ads(val) (val > 0 ? val : -val)
#define min(val1, val2) (val1 > val2 ? val2 : val1)
#define max(val1, val2) (val1 > val2 ? val1 : val2)

struct global_env {
    struct {
        bool collectd;
        bool outlog;
    } info;
    struct {
        int run_cnt_up_to;
        int tinatest_run_time_sec;
    } limit;
};

struct local_env {
    struct {
        bool date;
        bool resource;
        bool real_time_log;
    } info;

    struct {
        int run_times;
        bool run_alone;
        bool run_parallel;
        bool may_reboot;
        int testcase_run_once_time_sec;
        int testcase_run_time_sec;
        bool timeout_as_failed;
        bool exit_once_failed;
    } limit;
};

enum task_status {
    TASK_END = -1,
    TASK_WAIT = 0,
    TASK_RUN = 1,
};

enum input_t {
    TASK_NO_INPUT = 0,
    TASK_STDIN = 1,
    TASK_FSTDIN = 2,
};

struct task
{
    enum task_status status;    // 测试用例状态
    int *killed;                // 测试用例被信号杀死
    pid_t *pid;                 // 执行测试用例的进程id(此处是动态数组指针)
    pid_t pid_limit;            // 整个测试用例限制时长的进程id
    char *logpath;              // log信息保存的文件
    enum input_t input_t;       // 标准输入类型
    char *finput;               // 文件输入源
    struct {                    // 字符串输入源
        int array_cnt;
        char **array;
    } input;
    int *result;                // 测试的返回结果
    time_t *begin_time;         // 每次开始的时间
    time_t *end_time;           // 每次结束的时间
    struct rusage *res;         // 使用的资源
    time_t begin_date;          // 整体开始时间
    time_t end_date;            // 整体结束时间
    volatile int run_times;     // 执行的次数
    int rebooted_times;         // 重启的次数
    char *command;              // 执行的命令
    char *keypath;              // 测试用例路径
    struct shmem_testcase *shmem;      // 测试用例的共享数据(含进程组id)
    struct local_env env;       // 局部环境
    struct list_head lnode;     // 列表节点
};

// *========================================
// *功能: 初始化task的全局变量/添加测试用例
// *参数: 输入测测试用力数量 & 测试用例路径
// *返回: -1 : 失败; >=0 : 测试用例个数
// *========================================
int task_init(int argc, char **argv);
// *========================================
// *功能: 开始执行测试用例
// *参数:
// *返回: 0 : 成功; -1 : 失败
// *========================================
int task_begin(void);
// *========================================
// *功能: 重启后恢复环境
// *参数: 引起重启的测试用例路径
// *返回: 0 : 成功; -1 : 失败
// *========================================
int task_recover_after_reboot(char *keypath);

#define TASK_RESULT_PASS 0

#endif
