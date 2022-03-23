
#ifndef __CPU_LIB_INCLUDE_H_
#define __CPU_LIB_INCLUDE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>


#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define dprintf(fmt, ...) 	printf(""fmt"",##__VA_ARGS__)
#else
#define dprintf(fmt, args...)
#define fprintf(fmt, args...)
#endif

#undef  u8
#define u8     unsigned char
#undef  s8
#define s8     char
#undef  u32
#define u32    unsigned int
#undef  s32
#define s32    int



#define TEST_EXIT_SUCCESS	(0)
#define	TEST_EXIT_FAILURE	(-1)
#define CPU_NRS_MAX     (8)
#define MAX_LINE_LEN	255
#define SYSFS_PATH_MAX	255
#define PATH_TO_CPU "/sys/devices/system/cpu/"
#define CPU_IS_ONLINE	1
#define diff_abs(a,b) (a-b)?(a-b):(b-a);


struct cpufreq_info
{
	u32 online_cpu;
	u32 boot_freq;
	u32 burst_freq;
	u32 cur_freq;
	u32 max_freq;
	u32 min_freq;
	//u32 transition_latency;
	u32 scaling_cur_freq;
	u32 scaling_max_freq;
	u32 scaling_min_freq;
	char buf[1024];
	unsigned int blen;
};

struct cpufreq_governor_info
{
	u32 above_hispeed_delay;
	u32 boost;
	u32 boostpulse;
	u32 boostpulse_duration;
	u32 boosttop;
	u32 boosttop_duration;
	u32 go_hispeed_load;
	u32 io_is_busy;
	u32 min_sample_time;
	u32 target_loads;
	u32 timer_rate;
	u32 timer_slack;
};

struct cpufreq_list
{
	u32 cpufreq_list[CPU_NRS_MAX];
	u32 cpu_online[CPU_NRS_MAX];
};


struct cpu_stat {
	u32 id;
    u32 utime, ntime, stime, itime;
    u32 iowtime, irqtime, sirqtime;
    u32 totalcpuTime;
	u32 totalbusyTime;
};

struct cpu_usage {
	struct cpu_stat stats[CPU_NRS_MAX+1];
	u32 active;
};

struct cpu_loads {
	s32 iowait;
	s32 idle;
	s32 busy;
};

struct config
{
	char governor[15];	/* cpufreq governor */
	FILE *output;		/* logfile */
	char *output_filename;	/* logfile name, must be freed at the end*/
	struct cpufreq_list cpu_freq;
	struct cpu_usage pre_usages;
	struct cpu_loads cpu_load[CPU_NRS_MAX+1];
	s32 themp;
	s32 thempre[6];
};


/* read access to files which contain one numeric value */

enum cpufreq_value {
	AFFECTED_CPUS = 0,
	CPUINFO_BOOT_FREQ,
	CPUINFO_BURST_FREQ,
	CPUINFO_CUR_FREQ,
	CPUINFO_MAX_FREQ,
	CPUINFO_MIN_FREQ,
	CPUINFO_LATENCY,
	SCALING_CUR_FREQ,
	SCALING_MIN_FREQ,
	SCALING_MAX_FREQ,
	STATS_NUM_TRANSITIONS,
	MAX_CPUFREQ_VALUE_READ_FILES
};

static const char *cpufreq_value_files[MAX_CPUFREQ_VALUE_READ_FILES] = {
	[AFFECTED_CPUS]     = "affected_cpus",
	[CPUINFO_BOOT_FREQ] = "cpuinfo_boot_freq",
	[CPUINFO_BURST_FREQ]="cpuinfo_burst_freq",
	[CPUINFO_CUR_FREQ] = "cpuinfo_cur_freq",
	[CPUINFO_MAX_FREQ] = "cpuinfo_max_freq",
	[CPUINFO_MIN_FREQ] = "cpuinfo_min_freq",
	[CPUINFO_LATENCY]  = "cpuinfo_transition_latency",
	[SCALING_CUR_FREQ] = "scaling_cur_freq",
	[SCALING_MIN_FREQ] = "scaling_min_freq",
	[SCALING_MAX_FREQ] = "scaling_max_freq",
	[STATS_NUM_TRANSITIONS] = "stats/total_trans"
};

enum cpufreq_string {
	SCALING_DRIVER,
	SCALING_GOVERNOR,
	MAX_CPUFREQ_STRING_FILES
};

static const char *cpufreq_string_files[MAX_CPUFREQ_STRING_FILES] = {
	[SCALING_DRIVER] = "scaling_driver",
	[SCALING_GOVERNOR] = "scaling_governor",
};

/* write access */

enum cpufreq_write {
	WRITE_SCALING_MIN_FREQ,
	WRITE_SCALING_MAX_FREQ,
	WRITE_SCALING_GOVERNOR,
	WRITE_SCALING_SET_SPEED,
	MAX_CPUFREQ_WRITE_FILES
};


static const char *cpufreq_write_files[MAX_CPUFREQ_WRITE_FILES] = {
	[WRITE_SCALING_MIN_FREQ] = "scaling_min_freq",
	[WRITE_SCALING_MAX_FREQ] = "scaling_max_freq",
	[WRITE_SCALING_GOVERNOR] = "scaling_governor",
	[WRITE_SCALING_SET_SPEED] = "scaling_setspeed",
};

extern u32 sysfs_cpufreq_get_cpu_list(struct cpufreq_list *list);
extern u32 get_cpufre_params(unsigned int cpu, struct cpufreq_info *ins);
extern s32 read_proc_precpu_stat(struct cpu_usage *usages);
extern s32 read_proc_thermal_stat(void);
extern u32 find_cpu_online(u32 cpu);
extern u32 get_cpufreq_by_cpunr(u32 cpu);
extern s32 read_sysfs_tempe_state(char *buf);
extern void dump_cpuinfo(char *buf, struct cpu_stat *cur_cpu);


#endif
