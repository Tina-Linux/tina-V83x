#ifndef __SUN50IW3P1_H__
#define __SUN50IW3P1_H__
/* cpu spec files defined */
#define CPU_NUM_MAX 4
#define CPU0LOCK    "/sys/devices/system/cpu/cpu0/cpufreq/boot_lock"
#define ROOMAGE     "/sys/devices/platform/soc/cpu_budget_cooling/roomage"
#define CPUFREQ_AVAIL "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies"
#define CPUFREQ     "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq"
#define CPUFREQ_MAX "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPUFREQ_MIN "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"
#define CPUONLINE   "/sys/devices/system/cpu/online"
#define CPUHOT      "/sys/kernel/autohotplug/enable"
#define CPU0GOV     "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
/* gpu spec files defined */
#define GPUFREQ     "/sys/devices/1c40000.gpu/dvfs/android"
#define GPUCOMMAND "/sys/devices/platform/gpu/scenectrl/command"
/* ddr spec files defined */
#define DRAMFREQ_AVAIL "/sys/class/devfreq/dramfreq/available_frequencies"
#define DRAMFREQ    "/sys/class/devfreq/dramfreq/cur_freq"
#define DRAMFREQ_MAX    "/sys/class/devfreq/dramfreq/max_freq"
#define DRAMFREQ_MIN    "/sys/class/devfreq/dramfreq/min_freq"
#define DRAMMODE "/sys/class/devfreq/dramfreq/adaptive/pause"
/* task spec files defined */
#define TASKS       "/dev/cpuctl/tasks"
/* touch screen runtime suspend */
#define TP_SUSPEND  "/sys/devices/soc.0/1c2ac00.twi/i2c-0/0-0040/runtime_suspend"

#endif
