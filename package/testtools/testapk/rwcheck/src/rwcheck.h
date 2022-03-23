#ifndef __RWCHECK_H
#define __RWCHECK_H

#include <stdio.h>

#define KB ( 1024 )
#define MB ( KB * 1024 )
#define GB ( MB * 1024 )

#define min(val1, val2) (val1 > val2 ? val2 : val1)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

enum mode {
    /* 文件大小为剩余空间的50%,直至文件小于4k */
    auto_mode = 0,
    /* 每个文件均等大小 */
    fixed_mode,
    /* 文件以2的倍数递增 */
    up_mode,
};

struct size_unit {
    unsigned long long sumbytes;
    unsigned long long  size;  //size match units, humanized view
    char units[3];    //KB,MB or GB
};

struct fs_t {
    long int magic;
    const char str[20];
};

struct task {
    unsigned long long total_mem_mb;
    unsigned long long free_mem_mb;
    unsigned long long free_flash_kb;
    unsigned long long total_flash_kb;
    unsigned long long min_free_b;
    enum mode mode;
    char *checkdir;
    char *infile;
    int with_args_infile;
    unsigned int times;
    unsigned int percent;
    struct size_unit begin_size;
    struct size_unit end_size;
    struct size_unit fixed_size;
    struct size_unit buf_size;
    const struct fs_t *fs_type;
    int jobs;
};

#define VERSION "v1.1.0"
#define COMPILE "Compiled in " __DATE__ " at " __TIME__
#endif
