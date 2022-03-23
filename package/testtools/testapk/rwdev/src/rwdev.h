#ifndef __RWCHECK_H
#define __RWCHECK_H

#include <stdio.h>
#include <libubox/list.h>
#include "crc16.h"
#include "log.h"
#include "speed.h"

#define min(val1, val2) (val1 > val2 ? val2 : val1)
#define is_power_of_2(val) !(val & (val - 1))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define KB ( 1024 )
#define MB ( KB * 1024 )
#define GB ( MB * 1024 )

#define ACTION_WRITE (1 << 0)
#define ACTION_READ (1 << 1)
#define ACTION_VERIFY (1 << 2)

typedef struct size_unit {
    unsigned long sumbytes;
    unsigned long size; //size match units, humanized view
    char units[3];      //KB,MB or GB
} unit;

struct data_list {
    char *data;
    struct list_head lnode;
};

struct size_list {
    unit size;
    struct list_head lnode;
};

struct task {
    char action;
    char *dev;
    char *buf;
    bool def_data;
    unit testsize;
    unsigned short wr_crc;
    unsigned short rd_crc;
    unsigned int times;
    float speed;
    struct list_head data_head;
    struct list_head size_head;
    struct size_list *size;
    struct data_list *data;
};

#define VERSION "v0.0.4"
#define COMPILE "Compiled in " __DATE__ " at " __TIME__
#endif
