#ifndef __MCOLLECTD_H
#define __MCOLLECTD_H

#include "mjson.h"
#include "task.h"
#include "syskey.h"

#define COLLECTD_PATH "/sys/global/info/collectd"
#define COLLECTD_LIB_PATH "/usr/lib/collectd"
#define COLLECTD_CONF_PATH "/mnt/UDISK/collectd.conf"
#define COLLECTD_PID_PATH "/var/run/collectd.pid"

typedef int (*make_conf) (const char *collectd_path, FILE *fp);
int mcollectd_register(make_conf conf_func);

int mcollectd_load_json(void);
int mcollectd_make_conf(void);
int mcollectd_do(void); //will also create boot file
//will kill process, free list,
//and remove boot file but not conf and result file
int mcollectd_exit(void);

#endif
