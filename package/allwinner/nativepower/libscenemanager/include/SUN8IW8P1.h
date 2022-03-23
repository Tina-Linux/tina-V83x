#ifndef __SUN8IW8P1_H__
#define __SUN8IW8P1_H__
/* cpu spec files defined */
#define CPU_NUM_MAX 4
#define CPU0LOCK    "/sys/devices/system/cpu/cpu0/cpufreq/boot_lock"
#define ROOMAGE     "/sys/devices/platform/sunxi-budget-cooling/roomage"
#define CPUFREQ_AVAIL ""
#define CPUFREQ     "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq"
#define CPUFREQ_MAX "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPUFREQ_MIN "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"
#define CPUONLINE   "/sys/devices/system/cpu/online"
#define CPUHOT      "/sys/kernel/autohotplug/enable"
#define CPU0GOV     "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
/* gpu spec files defined */
#define GPUFREQ     "/sys/devices/platform/pvrsrvkm/dvfs/android"
/* ddr spec files defined */
#define DRAMFREQ_AVAIL ""
#define DRAMFREQ    "/sys/devices/platform/sunxi-ddrfreq/devfreq/sunxi-ddrfreq/cur_freq"
#define DRAMFREQ_MAX    "/sys/devices/platform/sunxi-ddrfreq/devfreq/sunxi-ddrfreq/scaling_max_freq"
#define DRAMFREQ_MIN    "/sys/devices/platform/sunxi-ddrfreq/devfreq/sunxi-ddrfreq/scaling_min_freq"
#define DRAMMODE    "/sys/class/devfreq/sunxi-ddrfreq/dsm/enable"
/* task spec files defined */
#define TASKS       "/dev/cpuctl/tasks"

#endif
