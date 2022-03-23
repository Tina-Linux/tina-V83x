#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "cpu_lib.h"


static u32 sysfs_read_file(const s8 *path, s8 *buf, size_t buflen)
{
	int fd;
	ssize_t numread;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return 0;

	numread = read(fd, buf, buflen - 1);
	if (numread < 1) {
		close(fd);
		return 0;
	}

	buf[numread] = '\0';
	close(fd);

	return (u32) numread;
}

static u32 sysfs_write_file(const s8 *path, const s8 *buf, size_t buflen)
{
	int fd;
	ssize_t numwrite;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return 0;

	numwrite = write(fd, buf, buflen);
	if (numwrite < 1) {
		close(fd);
		return 0;
	}

	close(fd);

	return (u32) numwrite;

}


static u32 sysfs_cpufreq_read_file(u32 cpu, const s8 *fname,
					    s8 *buf, size_t buflen)
{
	s8 path[SYSFS_PATH_MAX];

	snprintf(path, sizeof(path), PATH_TO_CPU "cpu%u/cpufreq/%s",
			 cpu, fname);
	return sysfs_read_file(path, buf, buflen);
}


static u32 sysfs_cpufreq_write_file(u32 cpu,
					     const s8 *fname,
					     const s8 *value, size_t len)
{
	s8 path[SYSFS_PATH_MAX];

	snprintf(path, sizeof(path), PATH_TO_CPU "cpu%u/cpufreq/%s",
			 cpu, fname);

	return sysfs_write_file(path, value, len);
}

static u32 sysfs_cpufreq_get_one_value(u32 cpu,
						 enum cpufreq_value which)
{
	u32 value;
	u32 len;
	s8 linebuf[MAX_LINE_LEN];
	s8 *endp;

	if (which >= MAX_CPUFREQ_VALUE_READ_FILES)
		return 0;

	len = sysfs_cpufreq_read_file(cpu, cpufreq_value_files[which],
				linebuf, sizeof(linebuf));

	if (len == 0)
		return 0;

	value = strtoul(linebuf, &endp, 0);

	if (endp == linebuf || errno == ERANGE)
		return 0;

	return value;
}

static int sysfs_cpufreq_set_one_value(u32 cpu,
					 enum cpufreq_write which,
					 const s8 *new_value, size_t len)
{
	if (which >= MAX_CPUFREQ_WRITE_FILES)
		return 0;

	if (sysfs_cpufreq_write_file(cpu, cpufreq_write_files[which],
					new_value, len) != len)
		return -ENODEV;

	return 0;
};



static s8 *sysfs_cpufreq_get_one_string(u32 cpu,
					   enum cpufreq_string which)
{
	s8 linebuf[MAX_LINE_LEN];
	s8 *result;
	u32 len;

	if (which >= MAX_CPUFREQ_STRING_FILES)
		return NULL;

	len = sysfs_cpufreq_read_file(cpu, cpufreq_string_files[which],
				linebuf, sizeof(linebuf));
	if (len == 0)
		return NULL;

	result = strdup(linebuf);
	if (result == NULL)
		return NULL;

	if (result[strlen(result) - 1] == '\n')
		result[strlen(result) - 1] = '\0';

	return result;
}


static s32 sysfs_temperature_get_rawdata(char *them_path, char *tme_buf)
{

	FILE *file = NULL;
	s8 buf[MAX_LINE_LEN];
	u32 i, value =0;
	u32 tmep[6] = { 0 };

	if (!them_path || !tme_buf)
		return -1;
    file = fopen(them_path, "r");
    if (!file) {
		dprintf("Could not open %s.\n", them_path);
		return -1;
    }

    fseek(file,0,SEEK_SET);
    while(fgets(buf,100,file) != NULL)
    {
	if(sscanf(buf, "temperature[%d]:%d",
			&i, &value) <0) {
		        fclose(file);
			return -1;
	}
		tmep[i] = value;
    }
	memcpy(tme_buf, tmep,sizeof(tmep));
    fclose(file);

	return 0;
}

void dump_cpuinfo(char *buf, struct cpu_stat *cur_cpu)
{
	dprintf( "%s cpu info u:%u n:%u s:%u "
			 "i:%u iow:%u irq:%u sirq:%u "
			 "total-cpu:%u total-busy:%u\n",
			buf,
			cur_cpu->utime, cur_cpu->ntime,
			cur_cpu->stime, cur_cpu->itime, cur_cpu->iowtime,
			cur_cpu->irqtime, cur_cpu->sirqtime,
			cur_cpu->totalcpuTime, cur_cpu->totalbusyTime);
}



u32 get_cpufreq_by_cpunr(u32 cpu)
{
	u32 freq = 0;
	freq = sysfs_cpufreq_get_one_value(cpu, CPUINFO_CUR_FREQ);
	return freq;
}



/************************CPU Test API*****************************/

u32 find_cpu_online(u32 cpu)
{
	s8 path[SYSFS_PATH_MAX];
	s8 linebuf[MAX_LINE_LEN];
	s8 *endp;
	u32 value;
	u32 len;

	snprintf(path, sizeof(path), PATH_TO_CPU "cpu%u/online",
			 cpu);

	len = sysfs_read_file(path, linebuf, sizeof(linebuf));
	if (len == 0)
		return 0;

	value = strtoul(linebuf, &endp, 0);

	if (endp == linebuf || errno == ERANGE)
		return 0;

	return value;
}


u32 get_cpufre_params(u32 cpu, struct cpufreq_info *ins)
{
	u32 is_online;
	if (ins == NULL)
		return -1;
	is_online = find_cpu_online(cpu);
	if (is_online == CPU_IS_ONLINE) {
		ins->online_cpu = cpu;
		ins->boot_freq	= sysfs_cpufreq_get_one_value(cpu,CPUINFO_BOOT_FREQ);
		ins->burst_freq = sysfs_cpufreq_get_one_value(cpu,CPUINFO_BURST_FREQ);
		ins->cur_freq	= sysfs_cpufreq_get_one_value(cpu,CPUINFO_CUR_FREQ);
		ins->max_freq	= sysfs_cpufreq_get_one_value(cpu,CPUINFO_MAX_FREQ);
		ins->min_freq	= sysfs_cpufreq_get_one_value(cpu,CPUINFO_MIN_FREQ);
		//ins->transition_latency = sysfs_cpufreq_get_one_value(cpu,CPUINFO_LATENCY);
		ins->scaling_cur_freq	= sysfs_cpufreq_get_one_value(cpu,SCALING_CUR_FREQ);
		ins->scaling_max_freq	= sysfs_cpufreq_get_one_value(cpu,SCALING_MAX_FREQ);
		ins->scaling_min_freq	= sysfs_cpufreq_get_one_value(cpu,SCALING_MIN_FREQ);
		return 0;
	} else {
		return -1;
	}
}

#if 0

s32 read_proc_usage_stat(struct cpu_info *cur_cpu)
{
    FILE *file;
    char buf[100];
    if (!cur_cpu)
		return -1;

    file = fopen("/proc/stat", "r");
    if (!file) {
		dprintf("Could not open /proc/stat.\n");
		return -1;
    }

    fseek(file,0,SEEK_SET);
    if((fgets(buf,100,file) != NULL) && (!strncmp(buf, "cpu",3)))
    {
		if(sscanf(buf, "cpu  %d %d %d %d %d %d %d",
			&cur_cpu->utime, &cur_cpu->ntime,
			&cur_cpu->stime, &cur_cpu->itime,
			&cur_cpu->iowtime, &cur_cpu->irqtime,
			&cur_cpu->sirqtime) < 0)
				return -1;
	/*dprintf( "cpu  %lu %lu %lu %lu %lu %lu %lu\n",
			cur_cpu->utime, cur_cpu->ntime,
			cur_cpu->stime, cur_cpu->itime, cur_cpu->iowtime,
			cur_cpu->irqtime, cur_cpu->sirqtime);*/
    }

	cur_cpu->totalbusyTime = cur_cpu->utime + cur_cpu->ntime + cur_cpu->stime +
							 + cur_cpu->irqtime + cur_cpu->sirqtime;

	cur_cpu->totalidle = cur_cpu->itime + cur_cpu->iowtime;

	cur_cpu->totalcpuTime = cur_cpu->totalbusyTime + cur_cpu->totalidle;

    fclose(file);
    return 0;
}
#endif

s32 read_proc_thermal_stat(void)
{
	char *path = NULL;
	u32 value;
	u32 len;
	s8 buf[MAX_LINE_LEN];
	s8 *endp;

	path = "/sys/class/thermal/thermal_zone0/temp";
	len  = sysfs_read_file(path, buf, sizeof(buf));
	if (len == 0)
		return -1;

	value = strtoul(buf, &endp, 0);

	if (endp == buf || errno == ERANGE)
		return -1;
	return (s32)value;
}





s32 read_proc_precpu_stat(struct cpu_usage *usages)
{
    FILE *file;
    char buf[100];
    s32 i = 0;
	s32 item = 0;
	struct cpu_stat cur_cpu;
    if (!usages)
		return -1;

    file = fopen("/proc/stat", "r");
    if (!file) {
		dprintf("Could not open /proc/stat.\n");
		return -1;
    }

    fseek(file,0,SEEK_SET);
    while((fgets(buf,100,file) != NULL) && (!strncmp(buf, "cpu",3)))
    {
	item = 0;
		memset(&cur_cpu, 0 , sizeof(cur_cpu));

	if (i == 0) {
			//printf("+++++ buf %s\n",buf);
		if(sscanf(buf, "cpu  %d %d %d %d %d %d %d",
					&cur_cpu.utime, &cur_cpu.ntime,
					&cur_cpu.stime, &cur_cpu.itime,
					&cur_cpu.iowtime, &cur_cpu.irqtime,
					&cur_cpu.sirqtime) < 0) {
				        fclose(file);
						return -1;
					}
			item = 8;
			i++;
	} else {
		//printf("+++++ buf %s\n",buf);
			if (sscanf(buf, "cpu%d  %d %d %d %d %d %d %d",
					&cur_cpu.id, &cur_cpu.utime, &cur_cpu.ntime,
					&cur_cpu.stime, &cur_cpu.itime,
					&cur_cpu.iowtime, &cur_cpu.irqtime,
					&cur_cpu.sirqtime) < 0) {
				        fclose(file);
						return -1;
					}
			item = cur_cpu.id;
       }
		cur_cpu.totalbusyTime = cur_cpu.utime +
			                    cur_cpu.stime +
			                    cur_cpu.ntime +
			                    cur_cpu.irqtime +
			                    cur_cpu.sirqtime;

		cur_cpu.totalcpuTime =	cur_cpu.totalbusyTime +
								cur_cpu.itime +
								cur_cpu.iowtime;

		//dump_cpuinfo(&cur_cpu);
		usages->active |= (1UL<<item);
		memcpy(&usages->stats[item], &cur_cpu, sizeof(cur_cpu));
		//dump_cpuinfo(&cur_cpu);
    }
    fclose(file);
    return 0;
}


s32 read_sysfs_tempe_state(char *buf)
{
    int fd = -1;
	int find = -1;
	int input_id = -1;
    const char *dirname = "/sys/class/input";
    char devname[256];
    char *filename;
    DIR *root;
	DIR *sub;
    struct dirent *dir_root;
	struct dirent *dir_sub;

	if (!buf)
		return -1;
    root = opendir(dirname);
    if(root == NULL)
       return -1;

	filename = &devname[0];
	strcpy(devname, dirname);
	filename +=  strlen(devname);
    *filename++ = '/';

    while((dir_root = readdir(root))) {
		if ((dir_root->d_type & 0x0a)&&
			(strncmp(dir_root->d_name, "input",5)==0)){
			strcpy(filename, dir_root->d_name);
			filename += strlen(dir_root->d_name);
			sub = opendir(devname);
			if(sub == NULL)
			return -1;
			while((dir_sub = readdir(sub))) {
				if ((strcmp(dir_sub->d_name,"temperature")==0)) {
					*filename++ = '/';
					strcpy(filename, dir_sub->d_name);
					find = 1;
					break;
				}
			}
			if (find == 1) {
				break;
			} else {
				filename -= strlen(dir_root->d_name);
			}
			closedir(sub);
		 }
    }

    closedir(root);

	if (find != 1) {
		printf("%s Not found temp sysfs\n",__func__);
		return -1;
	}

	if (sysfs_temperature_get_rawdata(devname, buf) <0) {
		printf("%s Read Temp sysfs Err\n",__func__);
		return -1;
	}

	return 0;
}
