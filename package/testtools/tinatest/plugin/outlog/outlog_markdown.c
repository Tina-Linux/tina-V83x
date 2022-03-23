#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "outlog.h"
#include "fileops.h"
#include "mjson.h"

#define BASE_JSON_PATH "/global/info/outlog/markdown/outdir"
#define DEFAULT_SAVEDIR "/mnt/UDISK/md"
#define DEFAULT_RESULT "/mnt/UDISK/md/result-list"
#define DEFAULT_REPORT_NAME "report.md"
#define PREFIX_LOG "log"
#define PREFIX_ATTR "attr"
#define SUFFIX_MD ".md"

#define TITLE_OVERVIEW "overview"
#define TITLE_ATTRIBUTE "attribute"
#define TITLE_INPUT "input"
#define TITLE_RESOURCE "resource"
#define TITLE_LOG "log"

#define RESULT_PASS "<font color=green>Pass</font>"
#define RESULT_FAILED "**<font color=red>Failed</font>**"

static char *outdir;

// get base dir
static char *md_get_outdir(void)
{
    char *outdir = NULL;
    if ((outdir = mjson_fetch_string(BASE_JSON_PATH)) == NULL)
        outdir = DEFAULT_SAVEDIR;

    if (outdir[strlen(outdir) - 1] == '/')
        outdir[strlen(outdir) - 1] = '\0';

    return outdir;
}

// get outlog and outattr
static int md_get_outname(char *keypath, char *outlog, int loglen, char *outattr, int attrlen)
{
    int ret = -1;
    if (keypath == NULL) {
        ERROR("keypath is NULL\n");
        goto out;
    }

    char *p = NULL;

    // outlog
    snprintf(outlog, loglen, PREFIX_LOG);
    p = outlog + strlen(PREFIX_LOG);
    strncpy(p, keypath, loglen - strlen(PREFIX_LOG));
    while ((p = strchr(p, (int)'/')) != NULL)
        *p = (char)'-';
    p = outlog + strlen(outlog);
    strncpy(p, SUFFIX_MD, strlen(SUFFIX_MD));

    // outattr
    snprintf(outattr, attrlen, PREFIX_ATTR);
    p = outattr + strlen(PREFIX_ATTR);
    strncpy(p, keypath, attrlen - strlen(PREFIX_ATTR));
    while ((p = strchr(p, (int)'/')) != NULL)
        *p = (char)'-';
    p = outattr + strlen(outattr);
    strncpy(p, SUFFIX_MD, strlen(SUFFIX_MD));

    ret = 0;
out:
    return ret;
}

static inline char *strtoupper(char *str)
{
    int len = strlen(str);
    for (int num = 0; num < len; num++)
        str[num] = toupper(str[num]);

    return str;
}

static int md_get_result(const char *keypath)
{
    int ret = -1;
    if (is_existed(DEFAULT_RESULT) == false) {
        ERROR("%s is non-existent\n", DEFAULT_RESULT);
        goto err;
    }

    FILE *fp = fopen(DEFAULT_RESULT, "r");
    if (fp == NULL) {
        ERROR("open %s failed\n", DEFAULT_RESULT);
        goto err;
    }

    char *line = NULL, *p = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {
        p = strchr(line, ' ');
        *p = '\0';
        if (strcmp(keypath, line) == 0) {
            p++;
            ret = atoi(p);
            break;
        }
    }

    free(line);
    fclose(fp);
err:
    return ret;
}

static int md_save_result(const char *keypath, int result)
{
    int ret = -1;
    FILE *fp = fopen(DEFAULT_RESULT, "a");
    if (fp == NULL) {
        ERROR("open %s failed\n", DEFAULT_RESULT);
        goto err;
    }
    fprintf(fp, "%s %d\n", keypath, result);
    fclose(fp);

    ret = 0;
err:
    return ret;
}

static int md_out_log(struct task *task, char *path)
{
    int ret = -1;
    FILE *fpout = NULL;

    /* out path */
    fpout = fopen(path, "w");
    if (fpout == NULL) {
        ERROR("open %s failed - %s\n", path, strerror(errno));
        goto err;
    }

    /* log title */
    {
        char *buf = malloc(sizeof(TITLE_LOG) + 3);
        if (buf == NULL) {
            ERROR("malloc failed - %s\n", strerror(errno));
            goto err;
        }
        memcpy(buf, TITLE_LOG, sizeof(TITLE_LOG));
        fprintf(fpout, "### %s\n\n", strtoupper(buf));
        free(buf);
    }

    if (NULL == task->logpath) {
        fprintf(fpout, "    real_time_log was set, None!  \n");
    } else {
        char *line = NULL;
        size_t len = 0, bytes = 0;
        FILE *fpin = fopen(task->logpath, "r");
        if (fpin == NULL) {
            ERROR("open %s failed - %s\n", task->logpath, strerror(errno));
            goto err;
        }

        while ((bytes = getline(&line, &len, fpin)) != -1) {
            line[bytes - 1] = '\0';
            fprintf(fpout, "    %s  \n", line);
        }
        free(line);
        fclose(fpin);
    }

    ret = 0;
err:
    fclose(fpout);
    return ret;
}

static inline void md_out_attr_attribute(struct task *task, FILE *fp)
{
    char *buf = malloc(sizeof(TITLE_ATTRIBUTE) + 3);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return;
    }
    memcpy(buf, TITLE_ATTRIBUTE, sizeof(TITLE_ATTRIBUTE));

    // title
    fprintf(fp, "### %s\n\n", strtoupper(buf));
    free(buf);

    fprintf(fp, "| class | key | value |\n");
    fprintf(fp, "| --- | --- | --- |\n");
    fprintf(fp, "| task | command | %s |\n", task->command);
    if (task->env.info.date == true) {
        struct tm *tm = gmtime(&task->begin_date);
        fprintf(fp, "| task | begin date | %d-%d-%d %d:%d:%d |\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        tm = gmtime(&task->end_date);
        fprintf(fp, "| task | end date | %d-%d-%d %d:%d:%d |\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    switch (task->input_t) {
        case TASK_NO_INPUT: {
                fprintf(fp, "| task | input | none |\n");
                break;
            }
        case TASK_STDIN: {
                fprintf(fp, "| task | input | stdin |\n");
                break;
            }
        case TASK_FSTDIN: {
                fprintf(fp, "| task | input | file-stdin |\n");
                break;
            }
    }
    if (task->env.limit.may_reboot == true) {
        fprintf(fp, "| task | reboot times (real/max) | %d/%d |\n",
                task->rebooted_times, task->env.limit.run_times);
    }
    fprintf(fp, "| info | date | %s |\n",
            task->env.info.date == true ? "true" : "false" );
    fprintf(fp, "| info | resource | %s |\n",
            task->env.info.resource == true ? "true" : "false" );
    fprintf(fp, "| info | real_time_log | %s |\n",
            task->env.info.real_time_log == true ? "true" : "false" );
    fprintf(fp, "| limit | run_times (real/max) | %d/%d |\n",
            task->run_times, task->env.limit.run_times);
    fprintf(fp, "| limit | run_alone | %s |\n",
            task->env.limit.run_alone == true ? "true" : "false" );
    fprintf(fp, "| limit | run_parallel | %s |\n",
            task->env.limit.run_parallel == true ? "true" : "false" );
    fprintf(fp, "| limit | may_reboot | %s |\n",
            task->env.limit.may_reboot == true ? "true" : "false" );
    fprintf(fp, "| limit | testcase_run_once_time | %d |\n",
            task->env.limit.testcase_run_once_time_sec);
    fprintf(fp, "\n");
}

static void md_out_attr_overview(struct task *task, FILE *fp)
{
    int result = 0;
    struct tm *time;
    char *buf = malloc(sizeof(TITLE_OVERVIEW) + 3);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return;
    }
    memcpy(buf, TITLE_OVERVIEW, sizeof(TITLE_OVERVIEW));

    // title
    fprintf(fp, "### %s\n\n", strtoupper(buf));
    free(buf);

    fprintf(fp, "| num | pid | pgid | return | begin | end | killed by signal |\n");
    fprintf(fp, "| :---: | :---: | :---: | :---: | :---: | :---: | :---: |\n");
    for (int num = 0; num < task->run_times; num++) {
        fprintf(fp, "| %d | %d | %d | %d |", num, -task->pid[num],
                abs(task->shmem->pgid), task->result[num]);

        time = gmtime(&task->begin_time[num]);
        fprintf(fp, " %2.2d:%2.2d:%2.2d |", time->tm_hour, time->tm_min, time->tm_sec);

        time = gmtime(&task->end_time[num]);
        fprintf(fp, " %2.2d:%2.2d:%2.2d |", time->tm_hour, time->tm_min, time->tm_sec);

        if (task->killed[num])
            fprintf(fp, " %s(%d) |\n",
                    strsignal(task->killed[num]), task->killed[num]);
        else
            fprintf(fp, " none |\n");

        // get result
        if (task->result[num] != 0)
            result = task->result[num];
    }
    fprintf(fp, "\n");

    md_save_result(task->keypath, result);
}

static void md_out_attr_resource(struct task *task, FILE *fp)
{
    char *buf = malloc(sizeof(TITLE_RESOURCE) + 3);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return;
    }
    memcpy(buf, TITLE_RESOURCE, sizeof(TITLE_RESOURCE));

    // title
    fprintf(fp, "### %s\n\n", strtoupper(buf));
    free(buf);

    fprintf(fp, "| item | value |\n");
    fprintf(fp, "| ---- | ---- |\n");
    fprintf(fp, "| user cpu time | %ld.%ld |\n",
            task->res->ru_utime.tv_sec, task->res->ru_utime.tv_usec/1000);
    fprintf(fp, "| system cpu time | %ld.%ld |\n",
            task->res->ru_stime.tv_sec, task->res->ru_stime.tv_usec/1000);
    fprintf(fp, "| maximum resident size | %ldkB |\n",
            task->res->ru_maxrss);
    fprintf(fp, "| page faults break times (without I/O) | %ld |\n",
            task->res->ru_minflt);
    fprintf(fp, "| page faults break times (with I/O) | %ld |\n",
            task->res->ru_majflt);
    fprintf(fp, "| input times | %ld |\n", task->res->ru_inblock);
    fprintf(fp, "| output times | %ld |\n", task->res->ru_oublock);
    fprintf(fp, "| wait resource actively times | %ld |\n", task->res->ru_nvcsw);
    fprintf(fp, "| wait resource passively times | %ld |\n", task->res->ru_nivcsw);
    fprintf(fp, "\n");
}

static void md_out_attr_input(struct task *task, FILE *fp)
{
    char *buf = malloc(sizeof(TITLE_INPUT) + 3);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        return;
    }
    memcpy(buf, TITLE_INPUT, sizeof(TITLE_INPUT));

    // title
    fprintf(fp, "### %s\n\n", strtoupper(buf));
    free(buf);

    switch (task->input_t) {
        case TASK_NO_INPUT: {
            break;
        }
        case TASK_STDIN: {
            char **p = (char **)task->input.array;
            int cnt = task->input.array_cnt;
            for (int num = 0; num < cnt; num++)
                fprintf(fp, "    %s  \n", p[num]);
            break;
        }
        case TASK_FSTDIN: {
            char *line = NULL;
            size_t len = 0, bytes = 0;
            FILE *fin = fopen((char *)task->finput, "r");
            if (fin != NULL) {
                while ((bytes = getline(&line, &len, fin)) != -1) {
                    line[bytes - 1] = '\0';
                    fprintf(fp, "    %s  \n", line);
                }
            } else {
                fprintf(fp, "ERROR");
                ERROR("open %s failed - %s\n", (char *)task->finput, strerror(errno));
            }
            free(line);
            fclose(fin);
            break;
        }
    }
    fprintf(fp, "\n");
}

static int md_out_attr(struct task *task, char *path)
{
    int ret = -1;
    FILE *fp = NULL;
    fp = fopen(path, "w");
    if (fp == NULL) {
        ERROR("open %s failed - %s\n", path, strerror(errno));
        goto err;
    }

    // title
    fprintf(fp, "## %s\n", task->keypath);

    // result
    md_out_attr_overview(task, fp);
    // attr
    md_out_attr_attribute(task, fp);
    // resource
    if (task->env.info.resource == true
            && task->res != NULL) {
        md_out_attr_resource(task, fp);
    }
    // input
    if (task->input_t != TASK_NO_INPUT) {
        md_out_attr_input(task, fp);
    }

    ret = 0;
err:
    fclose(fp);
    return ret;
}

static inline void md_del_char(char *s, int c)
{
    int j,k;
    for(j=k=0; s[j]!='\0'; j++)
        if(s[j]!=(char )c)
            s[k++]=s[j];
    s[k]= '\0';
}

static int md_report_detail(struct list_head *TASK_LIST, FILE *fp)
{
    int ret = -1;
    char *outlog = NULL, *outattr = NULL, *path = NULL;
    int outlen = 1000;
    int len = strlen(outdir) + outlen;
    int from = -1, to = fileno(fp);

    outlog = calloc(1, outlen);
    outattr = calloc(1, outlen);
    path = calloc(1, len);
    if (outlog == NULL || outattr == NULL || path == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }

    fprintf(fp, "# DETAIL\n");
    struct task *task = NULL;
    list_for_each_entry(task, TASK_LIST, lnode) {
        if (md_get_outname(task->keypath, outlog, outlen, outattr, outlen) < 0)
            continue;

        // outattr
        snprintf(path, len, "%s/%s", outdir, outattr);
        if (is_existed(path) == false) {
            ERROR("not found %s\n", path);
            goto err;
        }
        from = open(path, O_RDONLY);
        if (from < 0) {
            ERROR("open %s failed - %s\n", path, strerror(errno));
            goto err;
        }

        fprintf(fp, "\n");
        fflush(fp);
        cp_fd(from, to);
        close(from);

        // outlog
        snprintf(path, len, "%s/%s", outdir, outlog);
        if (is_existed(path) == false) {
            ERROR("not found %s\n", path);
            goto err;
        }
        from = open(path, O_RDONLY);
        if (from < 0) {
            ERROR("open %s failed - %s\n", path, strerror(errno));
            goto err;
        }

        fprintf(fp, "\n");
        fflush(fp);
        cp_fd(from, to);
        close(from);

        from = -1;
    }

    ret = 0;
err:
    free(path);
    free(outlog);
    free(outattr);
    close(from);
    return ret;
}

static int md_report_summary(struct list_head *TASK_LIST, FILE *fp)
{
    int result = 0;
    int cnt = 0;
    int ret = -1;
    int len = 1000;
    char *suffix = NULL, *link = NULL;

    suffix = malloc(len);
    link = malloc(len);
    if (suffix == NULL || link == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }

    fprintf(fp, "# REPORT\n\n");
    fprintf(fp, "| testcase | result | overview | attribute | resource | input | log |\n");
    fprintf(fp, "| :--- | :---: | :---: | :---: | :---: | :---: | :---: |\n");

    struct task *task = NULL;
    list_for_each_entry(task, TASK_LIST, lnode) {
        // get result
        // we can't use task->result because it was reset after rebooting
        result = md_get_result(task->keypath);
        DEBUG(DATA, "%s: result is %d\n", task->keypath, result);

        // init suffix
        if (cnt == 0)
            *suffix = '\0';
        else
            snprintf(suffix, len, "-%d", cnt);

        // init link
        snprintf(link, len, "%s", task->keypath);
        md_del_char(link, '/');

        // testcase && result && overview && attribute
        fprintf(fp, "| [%s](#%s) | %s | [view](#%s%s) | [attr](#%s%s) |",
                task->keypath, link, result == 0 ? RESULT_PASS : RESULT_FAILED,
                TITLE_OVERVIEW, suffix,
                TITLE_ATTRIBUTE, suffix);

        // resource
        if (task->env.info.resource == true
                && task->res != NULL)
            fprintf(fp, " [res](#%s%s) |", TITLE_RESOURCE, suffix);
        else
            fprintf(fp, " none |");

        // input
        if (task->input_t != TASK_NO_INPUT)
            fprintf(fp, " [input](#%s%s) |", TITLE_INPUT, suffix);
        else
            fprintf(fp, " none |");

        // log
        fprintf(fp, " [log](#%s%s) |", TITLE_LOG, suffix);

        fprintf(fp, "\n");
        cnt++;
    }

    fprintf(fp, "\n");
    ret = 0;
err:
    free(link);
    free(suffix);
    return ret;
}

static int md_report(struct list_head *TASK_LIST, char *path)
{
    int ret = -1;
    FILE *fp = NULL;
    fp = fopen(path, "w");
    if (fp == NULL) {
        ERROR("open %s failed - %s\n", path, strerror(errno));
        goto err;
    }

    // summary
    if (md_report_summary(TASK_LIST, fp) < 0)
        ERROR("markdown: report summary failed\n");
    // detail
    if (md_report_detail(TASK_LIST, fp) < 0)
        ERROR("markdown: report detail failed\n");

    ret = 0;
err:
    fclose(fp);
    return ret;
}


int md_after_one(struct task *task)
{
    int ret = -1;
    char *path = NULL;
    char *outlog = NULL, *outattr = NULL;
    if (task == NULL) {
        ERROR("no task infomations\n");
        goto err;
    }
    int outlen = strlen(task->keypath) + 30;
    outlog = calloc(1, outlen);
    outattr = calloc(1, outlen);
    if (outlog == NULL || outattr == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }

    DEBUG(BASE, "markdown: after one : %s\n", task->keypath);
    // init outlog outattr
    if (md_get_outname(task->keypath, outlog, outlen, outattr, outlen) < 0)
        goto err;
    DEBUG(DATA, "%s: outlog set to %s/%s\n", task->keypath, outdir, outlog);
    DEBUG(DATA, "%s: outattr set to %s/%s\n", task->keypath, outdir, outattr);

    int len = strlen(outdir) + 10 +  strlen(task->keypath) +
        (sizeof(PREFIX_LOG) > sizeof(PREFIX_ATTR) ? sizeof(PREFIX_LOG) : sizeof(PREFIX_ATTR));
    path = malloc(len);
    if (path == NULL) {
        ERROR("%s: malloc failed - %s\n", task->keypath, strerror(errno));
        goto err;
    }

    // get outattr
    snprintf(path, len, "%s/%s", outdir, outattr);
    DEBUG(BASE, "%s: print attrtbutes to %s\n", task->keypath, path);
    if (md_out_attr(task, path) < 0) {
        ERROR("%s: print attrtbutes failed\n", task->keypath);
        goto err;
    }

    // get outlog
    snprintf(path, len, "%s/%s", outdir, outlog);
    DEBUG(BASE, "%s: print log to %s\n", task->keypath, path);
    if (md_out_log(task, path) < 0) {
        ERROR("%s: print log failed\n", task->keypath);
        goto err;
    }

    DEBUG(BASE, "markdown: after one : %s exit\n", task->keypath);

    ret = 0;
err:
    free(path);
    free(outlog);
    free(outattr);
    return ret;
}

int md_after_all(struct list_head *TASK_LIST)
{
    int ret = -1;
    char *path = NULL;
    if (TASK_LIST == NULL) {
        ERROR("TASK_LIST NULL\n");
        goto err;
    }

    DEBUG(BASE, "markdown: after all\n");
    int len = strlen(outdir) + sizeof(DEFAULT_REPORT_NAME) + 5;
    path = malloc(len);
    if (path == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }
    snprintf(path, len, "%s/%s", outdir, DEFAULT_REPORT_NAME);

    if (md_report(TASK_LIST, path) < 0) {
        ERROR("print report failed\n");
        goto err;
    }
    DEBUG(BASE, "markdown: after all exit\n");

    ret = 0;
err:
    remove(DEFAULT_RESULT);
    free(path);
    return ret;
}

void module_init(void) {
    outdir = md_get_outdir();
    DEBUG(DATA, "outlog_markdown: outdir is %s\n", outdir);
    if (mkdir_p(outdir) < 0) {
        ERROR("mkdir %s failed \n", outdir);
        ERROR("init outlog module markdown failed\n");
        return;
    }

    outlog_register(
            NULL,
            NULL,
            md_after_one,
            md_after_all);
}
