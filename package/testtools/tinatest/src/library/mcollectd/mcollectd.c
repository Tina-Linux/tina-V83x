#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ftw.h>

#include "mcollectd.h"
#include "task.h"
#include "sync.h"
#include "fileops.h"

static struct list_head COLLECTD_MODULE_LIST;
static pid_t collectd_pid;
static int module_cnt;
static char *dl_handle;

struct mcollectd_module {
    make_conf conf_func;
    char *dl_handle;
    struct list_head lnode;
};

// api for register a new collect module
int mcollectd_register(make_conf conf_func)
{
    if (conf_func == NULL) {
        ERROR("Invalid call with NULL argument\n");
        return -1;
    }
    struct mcollectd_module *m = malloc(sizeof(struct mcollectd_module));
    if (m == NULL) {
        ERROR("malloc for new struct mcollestd_module failed\n");
        return -1;
    }

    m->conf_func = conf_func;
    m->dl_handle = dl_handle;
    list_add_tail(&m->lnode, &COLLECTD_MODULE_LIST);
    return 0;
}

static int mcollectd_register_module(const char *libpath)
{
    void (*init_func)(void);

    DEBUG(BASE, "init shared lib %s\n", libpath);

    dl_handle = dlopen(libpath, RTLD_LAZY | RTLD_LOCAL);
    if (dl_handle == NULL) {
        ERROR("dlopen %s failed - %s\n", libpath, dlerror());
        goto err;
    }
    init_func = dlsym(dl_handle, MODULE_INIT_FUNC);
    if (init_func == NULL) {
        ERROR("dlsym %s for %s failed - %s\n", libpath, MODULE_INIT_FUNC, dlerror());
        goto err;
    }
    module_cnt += 1;
    // run register function
    init_func();
    // dl_handle has saved in func init_func，so, clear global variable
    dl_handle = NULL;

    return 0;
err:
    dlclose(dl_handle);
    return -1;
}

int mcollectd_load_json(void)
{
    char *buf = NULL;
    if (is_dir(MODULE_LIB_PATH) < 0) {
        ERROR("check %s failed\n", MODULE_LIB_PATH);
        goto err;
    }
    int len = strlen(COLLECTD_PATH);
    buf = calloc(10, len);
    if (buf == NULL) {
        ERROR("malloc for path buf failed\n");
        goto err;
    }

    // init list
    INIT_LIST_HEAD(&COLLECTD_MODULE_LIST);

    // register module
    module_cnt = 0;
    mjson_foreach(COLLECTD_PATH, key, val, mtype) {
        int ret;
        // check type
        if (mtype != mjson_type_object)
            continue;
        // check enable
        snprintf(buf, 10 * len, "%s/%s/%s",
                COLLECTD_PATH, key, SYSKEY_GLOBAL_INFO_COLLECTD_ENABLE);
        ret = mjson_fetch_boolean(buf);
        if (ret != (int)true)
            continue;
        DEBUG(BASE, "%s is true\n", buf);
        // check library existed
        snprintf(buf, 10 * len, "%s/%s.so", COLLECTD_LIB_PATH, key);
        if (access(buf, R_OK | F_OK) != 0) {
            ERROR("%s: Not found or can't read\n", buf);
            continue;
        }
        snprintf(buf, 10 * len, "%s/mcollectd_%s.so", MODULE_LIB_PATH, key);
        if (access(buf, R_OK | F_OK) != 0) {
            ERROR("%s: Not found or can't read\n", buf);
            continue;
        }
        // register module
        if (mcollectd_register_module(buf) < 0) {
            ERROR("%s: register failed\n", buf);
            continue;
        }
        DEBUG(BASE, "register module %s\n", key);
    }

    if (module_cnt <= 0) {
        DEBUG(BASE, "no collectd module is added\n");
        goto err;
    }

    free(buf);
    return 0;

err:
    free(buf);
    return -1;
}

int mcollectd_make_conf()
{
    FILE *fp = NULL;
    int len = strlen(COLLECTD_PATH), ret = -1;
    char *buf = NULL, *_buf = NULL;
    buf = calloc(10, len);
    if (buf == NULL) {
        ERROR("malloc for path buf failed\n");
        goto out;
    }
    _buf = calloc(10, len);
    if (_buf == NULL) {
        ERROR("malloc for path buf failed\n");
        goto out;
    }

    // get Interval seccond from json
    int interval_sec;
    strncpy(_buf, COLLECTD_PATH, 10 * len);
    snprintf(buf, 10 * len, "%s/%s", _buf, SYSKEY_GLOBAL_INFO_COLLECTD_INTERVAL_SEC);
    interval_sec = mjson_fetch_int(buf);
    if (interval_sec <= 0) {
        DEBUG(BASE, "Invalid num for collectd_interval_sec, set it as default %d\n",
                DEFAULT_GLOBAL_COLLECTD_INTERVAL_SEC);
        interval_sec = DEFAULT_GLOBAL_COLLECTD_INTERVAL_SEC;
    }

    // make config
    fp = fopen(COLLECTD_CONF_PATH, "w");
    if (fp == NULL) {
        ERROR("open %s failed\n", COLLECTD_CONF_PATH);
        goto out;
    }
    fprintf(fp, "PIDFile \"%s\"\n", COLLECTD_PID_PATH);
    fprintf(fp, "Interval %d\n", interval_sec);

    struct mcollectd_module *m = NULL;
    list_for_each_entry(m, &COLLECTD_MODULE_LIST, lnode) {
        fprintf(fp, "\n");
        if (m->conf_func(COLLECTD_PATH, fp) < 0) {
            ERROR("make collectd config failed\n");
            goto out;
        }
        dlclose(m->dl_handle);
    }

    ret = 0;
out:
    free(buf);
    free(_buf);
    fflush(fp);
    fclose(fp);
    return ret;
}

// collect pid in colletcd_pid
int mcollectd_do(void)
{
    int ret = -1, len = sizeof(COLLECTD_CONF_PATH) + 50;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto out;
    }
    snprintf(buf, len, "collectd -f -C %s &>/dev/null", COLLECTD_CONF_PATH);

    if ((collectd_pid = fork()) < 0) {
        ERROR("fork for collectd failed - %s\n", strerror(errno));
        goto out;
    } else if (collectd_pid == 0) { //child
        if (WAIT_PARENT(COLLECTD_DO_ID) < 0)
            _exit(127);

        execl("/bin/sh", "/bin/sh", "-c", buf, NULL);
        _exit(127);
    }

    setpgid(collectd_pid, collectd_pid);
    if (TELL_CHILD(COLLECTD_DO_ID) < 0) {
        ERROR("reply to child for running collectd failed\n");
        kill(collectd_pid, SIGKILL);
        goto out;
    }

    DEBUG(BASE, "collectd's pgid is %d\n", collectd_pid);
    ret = 0;

out:
    free(buf);
    return ret;
}

//will kill process, free list and boot file but not conf and result file
int mcollectd_exit(void)
{
    remove(COLLECTD_CONF_PATH);
    remove(COLLECTD_PID_PATH);
    kill(-collectd_pid, SIGKILL);
    return 0;
}
