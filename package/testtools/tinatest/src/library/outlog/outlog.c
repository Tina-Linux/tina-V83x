#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>

#include "outlog.h"

static struct list_head list_before_all;
static struct list_head list_before_one;
static struct list_head list_after_one_begin;
static struct list_head list_after_one_once;
static struct list_head list_after_one_end;
static struct list_head list_after_all;

static int cnt_sum;
static int cnt_before_all;
static int cnt_before_one;
static int cnt_after_one_begin;
static int cnt_after_one_once;
static int cnt_after_one_end;
static int cnt_after_all;

static char *module_name;

#define MODULE_LNODE(MODE) \
struct lnode_ ## MODE { \
    MODE func; \
    char * name; \
    struct list_head lnode; \
}

MODULE_LNODE(before_all);
MODULE_LNODE(before_one);
MODULE_LNODE(after_one_begin);
MODULE_LNODE(after_one_once);
MODULE_LNODE(after_one_end);
MODULE_LNODE(after_all);

#define MODULE_CALL_FUNC(MODE, ARGS_TYPE, ARGS_NAME) \
int outlog_call_ ## MODE (ARGS_TYPE *ARGS_NAME) \
{ \
    int ret = 0; \
    struct lnode_ ## MODE *m = NULL; \
    DEBUG(BASE, "In outlog function call " #MODE "\n"); \
    if (cnt_ ## MODE <= 0) { \
        DEBUG(BASE, "no module register in " # MODE "\n"); \
        return 0; \
    } \
    list_for_each_entry(m, &list_ ## MODE, lnode) { \
        DEBUG(BASE, "%s: call " # MODE "\n", m->name); \
        if (m->func(ARGS_NAME) != 0 && ret == 0) { \
            ERROR("%s: call " # MODE " failed\n", m->name); \
            ret = -1; \
        } \
    } \
    return ret; \
}

MODULE_CALL_FUNC(before_all, struct list_head, TASK_LIST);
MODULE_CALL_FUNC(before_one, struct task, task);
MODULE_CALL_FUNC(after_one_begin, struct task, task);
MODULE_CALL_FUNC(after_one_once, struct task, task);
MODULE_CALL_FUNC(after_one_end, struct task, task);
MODULE_CALL_FUNC(after_all, struct list_head, TASK_LIST);

static int outlog_load_lib(char *path)
{
    char *dl_handle = NULL;
    void (*init_func)(void);
    DEBUG(BASE, "load shared lib %s\n", path);
    // open library
    dl_handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    if (dl_handle == NULL) {
        ERROR("dlopen %s failed - %s\n", path, dlerror());
        goto err;
    }
    // fetch library
    init_func = dlsym(dl_handle, MODULE_INIT_FUNC);
    if (init_func == NULL) {
        ERROR("dlsym %s for %s failed - %s\n", path, MODULE_INIT_FUNC, dlerror());
        goto err;
    }
    // cnt
    cnt_sum += 1;
    // run register function
    init_func();

    return 0;
err:
    ERROR("Load lib failed - %s\n", path);
    dlclose(dl_handle);
    return -1;
}

static inline void init_list(void)
{
    // init cnt
    cnt_sum = 0;
    cnt_before_all = 0;
    cnt_before_one = 0;
    cnt_after_one_begin = 0;
    cnt_after_one_once = 0;
    cnt_after_one_end = 0;
    cnt_after_all = 0;
    // init all list
    INIT_LIST_HEAD(&list_before_all);
    INIT_LIST_HEAD(&list_before_one);
    INIT_LIST_HEAD(&list_after_one_begin);
    INIT_LIST_HEAD(&list_after_one_once);
    INIT_LIST_HEAD(&list_after_one_end);
    INIT_LIST_HEAD(&list_after_all);
}

int outlog_init(void)
{
    int ret = -1;
    init_list();

    DEBUG(BASE, "initializing outlog\n");
    char *buf = malloc(sizeof(OUTLOG_PATH) + 100);
    if (buf == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto err;
    }
    mjson_foreach(OUTLOG_PATH, key, val, mtype) {
        // check type
        DEBUG(BASE, "outlog-%s is set\n", key);
        module_name = key;
        // check library existed
        snprintf(buf, sizeof(OUTLOG_PATH) + 100, "%s/outlog_%s.so", MODULE_LIB_PATH, module_name);
        if (access(buf, R_OK | F_OK) != 0) {
            ERROR("%s: Not found or can't read %s\n", key, buf);
            continue;
        }
        // load library
        if (outlog_load_lib(buf) < 0) {
            ERROR("outlog module %s will not run\n", key);
            continue;
        }
        module_name = NULL;
    }

    ret = 0;
    free(buf);
err:
    return ret;
}

int outlog_register(
        before_all b_all,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all)
{
    return outlog_register_ex(
            b_all,
            NULL,
            NULL,
            a_one_once,
            a_one_end,
            a_all);
}

#define INIT_MODULE_LNODE(MODE, FUNC) \
    struct lnode_ ## MODE *m = malloc(sizeof(struct lnode_ ## MODE)); \
    if (m == NULL) { \
        ERROR("malloc failed - %s\n", strerror(errno)); \
        return -1; \
    } \
    m->func = FUNC; \
    m->name = module_name; \
    list_add_tail(&m->lnode, &list_ ## MODE); \
    cnt_ ## MODE ++; \
    DEBUG(BASE, "%s: register to" # MODE "\n", module_name);


int outlog_register_ex(
        before_all b_all,
        before_one b_one,
        after_one_begin a_one_begin,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all)
{
    // do register
    if (b_all != NULL) {
        INIT_MODULE_LNODE(before_all, b_all);
    }
    if (b_one != NULL) {
        INIT_MODULE_LNODE(before_one, b_one);
    }
    if (a_one_begin != NULL) {
        INIT_MODULE_LNODE(after_one_begin, a_one_begin);
    }
    if (a_one_once != NULL) {
        INIT_MODULE_LNODE(after_one_once, a_one_once);
    }
    if (a_one_end != NULL) {
        INIT_MODULE_LNODE(after_one_end, a_one_end);
    }
    if (a_all != NULL) {
        INIT_MODULE_LNODE(after_all, a_all);
    }
    return 0;
}
