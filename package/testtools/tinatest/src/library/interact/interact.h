/*
 * It's external *.h, including apis for testcase/tinatest/outlog
 */
#ifndef __INTERACT_H
#define __INTERACT_H

#define MAX_PATH 256
#define MAX_TEXT 2048
#define MAX_KEY 512
#define MAX_FPATH 512
#define MAX_TEXT_DFPATH (MAX_TEXT - MAX_FPATH)
#define MAX_TEXT_KEY (MAX_TEXT - MAX_KEY)
#define STR_TRUE "true"
#define STR_FALSE "false"

/*
 * api for testcase
 */
int task(const char *ask, char *reply, int len);
int ttips(const char *tips);
int ttrue(const char *tips);
int tscan(const char *key, const char *tips);
int tupfile(const char *filepath, const char *tips);
int tdownfile(const char *filename, const char *tips);
int tshowimg(const char *filepath, const char *tips);

/*
 * api for TinaTest
 */
int interact_init();
int interact_exit();

/*
 * api for outlog(actor)
 */
typedef int (*f_ask) (const char *testcase, const char *ask,
        char *reply, int len);
typedef int (*f_tips) (const char *testcase, const char *tips);
typedef int (*f_istrue) (const char *testcase, const char *tips);
typedef int (*f_scan) (const char *testcase, const char *tips, const char *key);
typedef int (*f_upfile) (const char *testcase, const char *filepath,
        const char *tips);
typedef int (*f_downfile) (const char *testcase, const char *filename,
        const char *tips);
typedef int (*f_showimg) (const char *testcase, const char *filepath,
        const char *tips);
int interact_register(
        f_ask ask,
        f_tips tips,
        f_istrue istrue,
        f_scan scan,
        f_upfile upfile,
        f_downfile downfile,
        f_showimg showimg);

#endif
