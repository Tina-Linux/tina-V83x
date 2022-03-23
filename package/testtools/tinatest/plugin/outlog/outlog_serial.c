#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "outlog.h"
#include "fileops.h"

static FILE *ttys_out;
static FILE *ttys_in;

static int serial_before_all(struct list_head *TASK_LIST)
{
    struct task *task;
    fprintf(ttys_out, "\n");
    fprintf(ttys_out, "============================");
    fprintf(ttys_out, " tasks list ");
    fprintf(ttys_out, "============================\n");
    list_for_each_entry(task, TASK_LIST, lnode)
        fprintf(ttys_out, "%s\n", task->keypath);
    fprintf(ttys_out, "============================");
    fprintf(ttys_out, "    end     ");
    fprintf(ttys_out, "============================\n");

    return 0;
}

static int serial_after_one_end(struct task *task)
{
    int ret = -1;
    struct tm *time;
    fprintf(ttys_out, "\n");
    fprintf(ttys_out, "----------------------------");
    fprintf(ttys_out, " %s ", task->keypath);
    fprintf(ttys_out, "----------------------------\n");
    fprintf(ttys_out, "* task path : %s\n", task->keypath);
    fprintf(ttys_out, "* task command : %s\n", task->command);
    fprintf(ttys_out, "* run times(real/max) : %d/%d\n",
            task->run_times, task->env.limit.run_times);
    fprintf(ttys_out, "* run parallel : %s\n",
            task->env.limit.run_parallel == true ? "yes" : "no");
    fprintf(ttys_out, "* run alone : %s\n",
            task->env.limit.run_alone == true ? "yes" : "no");
    fprintf(ttys_out, "* may reboot : %s\n",
            task->env.limit.may_reboot == true ? "yes" : "no");
    fprintf(ttys_out, "* run once time limit : %ds\n",
            task->env.limit.testcase_run_once_time_sec);
    fprintf(ttys_out, "* run time limit : %ds\n",
            task->env.limit.testcase_run_time_sec);
    fprintf(ttys_out, "* timeout with : %s\n",
            task->env.limit.timeout_as_failed == TASK_RESULT_PASS ? "pass" : "failed");
    fprintf(ttys_out, "* exit once failed : %s\n",
            task->env.limit.exit_once_failed == true ? "yes" : "no");
    fprintf(ttys_out, "* real-time log: %s\n",
            task->env.info.real_time_log == true ? "yes" : "no");
    if (task->env.limit.may_reboot == true)
        fprintf(ttys_out, "* reboot/run times : %d/%d\n",
                task->rebooted_times, task->run_times);
    if (task->env.info.date == true) {
        fprintf(ttys_out, "* begin date : %s", ctime(&task->begin_date));
        fprintf(ttys_out, "* end date : %s", ctime(&task->end_date));
    }
    fprintf(ttys_out, "* result :\n");
    fprintf(ttys_out, "*     num    pid    pgid    return     begin       end    note\n");
    for (int num = 0; num < task->run_times; num++) {
        fprintf(ttys_out, "* %7d%7d%8d%10d", num, -task->pid[num], abs(task->shmem->pgid),
                task->result[num]);
        time = gmtime(&task->begin_time[num]);
        fprintf(ttys_out, "  %2.2d:%2.2d:%2.2d", time->tm_hour, time->tm_min, time->tm_sec);
        time = gmtime(&task->end_time[num]);
        fprintf(ttys_out, "  %2.2d:%2.2d:%2.2d", time->tm_hour, time->tm_min, time->tm_sec);
        if (task->killed[num])
            fprintf(ttys_out, "    killed by signal %s(%d)\n",
                    strsignal(task->killed[num]), task->killed[num]);
        else
            fprintf(ttys_out, "\n");
    }
    if (task->env.info.resource == true
            && task->res != NULL) {
        fprintf(ttys_out, "* task resource :\n");
        fprintf(ttys_out, "*     user cpu time = %ld.%ld\n",
                task->res->ru_utime.tv_sec, task->res->ru_utime.tv_usec/1000);
        fprintf(ttys_out, "*     system cpu time = %ld.%ld\n",
                task->res->ru_stime.tv_sec, task->res->ru_stime.tv_usec/1000);
        fprintf(ttys_out, "*     maximum resident size = %ldkB\n", task->res->ru_maxrss);
        fprintf(ttys_out, "*     page faults break times (without I/O) = %ld\n",
                task->res->ru_minflt);
        fprintf(ttys_out, "*     page faults break times (with I/O) = %ld\n",
                task->res->ru_majflt);
        fprintf(ttys_out, "*     input times = %ld\n", task->res->ru_inblock);
        fprintf(ttys_out, "*     output times = %ld\n", task->res->ru_oublock);
        fprintf(ttys_out, "*     wait resource actively times = %ld\n", task->res->ru_nvcsw);
        fprintf(ttys_out, "*     wait resource passively times = %ld\n", task->res->ru_nivcsw);
    }

    // log
    if (task->logpath != NULL) {
        int fd = open(task->logpath, O_RDONLY);
        if (fd < 0) {
            ERROR("%s open %s failed\n", task->keypath, task->logpath);
            goto out;
        }
        fprintf(ttys_out, "* run log :\n");
        fprintf(ttys_out, "****************\n");
        cp_fd(fd, fileno(ttys_out));
        close(fd);
    } else {
        fprintf(ttys_out, "* run log: real_time_log was set, None!\n");
    }

    fprintf(ttys_out, "----------------------------");
    fprintf(ttys_out, " end ");
    fprintf(ttys_out, "----------------------------\n");

    ret = 0;
out:
    return ret;
}

static int serial_after_all(struct list_head *TASK_LIST)
{
    int result = 0;
    int num = 0;
    fprintf(ttys_out, "\n");
    fprintf(ttys_out, "============================");
    fprintf(ttys_out, " tasks result ");
    fprintf(ttys_out, "============================\n");
    struct task *task = NULL;
    list_for_each_entry(task, TASK_LIST, lnode) {
        for (result = 0, num = 0; num < task->env.limit.run_times; num++) {
            if (task->result[num] != 0) {
                result = task->result[num];
                break;
            }
        }
        fprintf(ttys_out, "%s - ", task->keypath);
        if (result != 0)
            fprintf(ttys_out, "NO (failed in %d times with %d back)\n", num, task->result[num]);
        else
            fprintf(ttys_out, "YES\n");
    }
    fprintf(ttys_out, "============================");
    fprintf(ttys_out, "     end      ");
    fprintf(ttys_out, "============================\n");

    return 0;
}

static int serial_interact_common(const char *testcase,
        const char *ask, char *reply, int len)
{
    int ret = 0;
    fprintf(ttys_out, "\n========================");
    fprintf(ttys_out, " Testcase Information ");
    fprintf(ttys_out, " ========================\n");
    fprintf(ttys_out, "<%s>:\n", testcase);
    fprintf(ttys_out, "%s\n", ask);
    if (len != 0 && reply != NULL) {
        /* No Use But Fix Static Check */
        fseek(ttys_out, 0, SEEK_END);
        if (fgets(reply, len, ttys_in) == NULL) {
            ERROR("fgets failed - %s\n", strerror(errno));
            ret = -1;
        }
        len = strlen(reply);
        if (reply[len - 1] == '\n')
            reply[len - 1] = '\0';
    }
    fprintf(ttys_out, "================================");
    fprintf(ttys_out, " END ");
    fprintf(ttys_out, "==================================\n");

    return ret;
}

static int serial_ask(const char *testcase, const char *ask,
        char *reply, int len)
{
    int ret = serial_interact_common(testcase, ask, reply, len);

    return ret;
}

static int serial_tips(const char *testcase, const char *tips)
{
    char resp[10] = {0};
    int ret = serial_interact_common(testcase, tips, resp, 10);

    return ret;
}

static int serial_istrue(const char *testcase, const char *ask)
{
    int ret = -1;
    int len = strlen(ask);
    char *buf = calloc(1, len + 10);
    if (buf == NULL)
        goto out;

    if (buf[len - 1] == '\n')
        buf[len - 1] = '\0';
    sprintf(buf, "%s [Y|n]: ", ask);

    char resp[MAX_TEXT] = {0};
    for (len = 0; len < 20; len++) {
        serial_interact_common(testcase, buf, resp, MAX_TEXT);
        switch (resp[0]) {
        case 'Y':
        case 'y':
        case '\0':
            ret = true;
            goto out;
        case 'N':
        case 'n':
            ret = false;
            goto out;
        default:
            continue;
        }
    }

out:
    free(buf);
    return ret;
}

void module_init(void) {
    char tty_buf[100];
    if (get_tty(tty_buf, 100) < 0)
        return;

    ttys_out = fopen(tty_buf, "w");
    if (ttys_out == NULL)
        return;
    setlinebuf(ttys_out);

    ttys_in = fopen(tty_buf, "r");
    if (ttys_in == NULL)
        return;

    outlog_register(
            serial_before_all,
            NULL,
            serial_after_one_end,
            serial_after_all);

    interact_register(
            serial_ask,
            serial_tips,
            serial_istrue,
	    NULL,
            NULL,
            NULL,
            NULL);
}
