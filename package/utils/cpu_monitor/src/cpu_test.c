/*  cpufreq-test Tool
 *
 *  Copyright (C) 2008 Christian Kornacker <ckornacker@suse.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "cpu_lib.h"

/*
static struct cpu_info old_cpu;
*/
static s32 monitor_usage_start = 0;
static s32 monitor_magic_switch = 0;
static struct option monitor_long_options[] = {
	{"output",		1,	0,	'o'},
	{"cpu",			1,	0,	'c'},
	{"tempreture",	1,	0,	't'},
	{"file",		1,	0,	'f'},
	{"show",        0,  0,  's'},
	{"help",		0,	0,	'h'},
	{"vresion",     0,  0,  'v'},
	{0,			0,	0,	 0}
};

const char* const monitor_short_options = "h:o:c:t:f:v:s:";

/*******************************************************************
  monitor_show_usage
 *******************************************************************/

void monitor_show_usage()
{
	printf("too show how to use: ./cpu_monitor\n");
	printf("Options:\n");
	printf(" -l, \t\tlist all available cpufreq \t\n");
	printf(" -c [count],\tdisplay CPU freq,utilization rate and temp [count] times\n");
	printf(" -s,\t\tdisplay CPU freq,utilization rate and temp all the time\n");
	//printf(" -t, --tempreture=<A15 A7 DRAM GPU> four tempreture sensor data\n");
	//printf(" -f, --file=<configfile>\t\tconfig file to use\n");
	printf(" -o, [dir],\toutput path. Filename will be OUTPUTPATH/cpufreq_TIMESTAMP.log\n");
	printf(" -h, \t\tPrs32 this help screen\n");
	printf(" -v, \t\tthe tool developer version\n");
	exit(1);
}

s32 monitor_show_themp(struct config *cfg)
{
	s32 themp_value = 0;
	if (!cfg)
		return -1;
	themp_value	= read_proc_thermal_stat();
	if (themp_value < 0)
		return -1;
	 cfg->themp = themp_value;
	 return 0;
}


/*
s32 monitor_proc_usage_show(struct config *cfg)
{
	struct cpu_info new_cpu;
	s32 ret =0;
	float usage = 0;
	s32 total, busy, idle = 0;

	if (!cfg)
		return -1;
	memset(&new_cpu, 0, sizeof(struct cpu_info));

	if (monitor_usage_start == 0) {
		memset(&old_cpu, 0, sizeof(struct cpu_info));
		ret = read_proc_usage_stat(&old_cpu);
		if (ret <0)
			return -1;
		monitor_usage_start = 1;
		usleep(100000);
	}
	if (read_proc_usage_stat(&new_cpu)<0)
		return -1;

	total = (new_cpu.totalcpuTime - old_cpu.totalcpuTime);
	busy = (new_cpu.totalbusyTime - old_cpu.totalbusyTime);
	idle = (new_cpu.totalidle - old_cpu.totalidle);
	//monitor_dump_cpuinfo(&old_cpu);
	//monitor_dump_cpuinfo(&new_cpu);
    //dprintf("diff_sys:%d diff_total:%d\n",diff_sys, diff_total);
    if (idle > total) {
		usage = 0;
    } else {
		usage = (float)busy/total*100;
    }

	memcpy(&old_cpu, &new_cpu,sizeof(struct cpu_info));
	cfg->usage = usage;

	return 0;
}
*/
s32 monitor_percpu_freq_show(struct config *cfg)
{
	u32 i = 0;
	u32 is_online;
	u32 pcpu_freq;
	if(!cfg)
		return -1;
	for (i=0; i<CPU_NRS_MAX; i++)
	{
		is_online = (i == 0) ? 1 : find_cpu_online(i);
		if (is_online == CPU_IS_ONLINE) {
			pcpu_freq = 0;
			pcpu_freq = get_cpufreq_by_cpunr(i);
			if (pcpu_freq != 0) {
				cfg->cpu_freq.cpu_online[i] = 1;
				cfg->cpu_freq.cpufreq_list[i] = pcpu_freq/1000;
			}
		} else {
			cfg->cpu_freq.cpu_online[i] = 0;
			cfg->cpu_freq.cpufreq_list[i] = 0;
		}
	}
	return 0;
}



s32 monitor_percpu_usage_show(struct config *cfg)
{
	struct cpu_usage cur;
	s32 ret =0;
	s32 i = 0;
	float usage = 0;
	float total, busy, idle, iowait = 0;

	if (!cfg)
		return -1;
	memset(&cur, 0, sizeof(cur));
	ret = read_proc_precpu_stat(&cur);
	if (ret <0)
		return -1;

	if (cfg->pre_usages.active == 0) {
		memcpy(&cfg->pre_usages, &cur, sizeof(cur));
		memset(&cur, 0, sizeof(cur));
		usleep(300000);
		if (read_proc_precpu_stat(&cur)<0)
			return -1;
	}
	//printf("cur.active %d \n",cur.active);

	for(i=0; i<(CPU_NRS_MAX+1); i++)
	{
		if (cur.active&(0x01<<i)) {
			if (cfg->pre_usages.active&(0x01<<i)) {
					total = cur.stats[i].totalcpuTime-
							cfg->pre_usages.stats[i].totalcpuTime;
					busy = cur.stats[i].totalbusyTime -
							cfg->pre_usages.stats[i].totalbusyTime;
					idle = cur.stats[i].itime -
							cfg->pre_usages.stats[i].itime;
					iowait =  cur.stats[i].iowtime -
							cfg->pre_usages.stats[i].iowtime;

				if ((idle > total) || (total <= 0)){
					//printf("WANRRING :CPU-%d idle %d total %d busy%d \n",
				   //		i,idle,total,busy);
					//dump_cpuinfo("cur: ", &cur.usages[i]);
					//dump_cpuinfo("pre: ", &cfg->pre_usages.usages[i]);
					cfg->cpu_load[i].iowait = iowait;
					cfg->cpu_load[i].idle = idle;
					cfg->cpu_load[i].busy = busy;
			} else {
			   // dump_cpuinfo(&cur.usages[i]);
					//dump_cpuinfo(&cfg->pre_usages.usages[i]);
					//cfg->usage[i] = (busy*1000/total/10);
				   usage = (float)busy/total*100;
				   cfg->cpu_load[i].busy = (u32)usage;
				   usage = (float)idle/total*100;
				   cfg->cpu_load[i].idle = (u32)usage;
				   usage = (float)iowait/total*100;
				   cfg->cpu_load[i].iowait = (u32)usage;
				   //printf("CPU-%d idle %d total %d busy%d \n",
				   //		i,idle,total,busy);
				}

				//printf("total%d busy%d idle%d cfg->usage[%d]: %6.2f\n",
				//		total, busy, idle,i, cfg->usage[i]);
			}
		} else {
			cfg->cpu_load[i].iowait = 0;
			cfg->cpu_load[i].idle = 0;
			cfg->cpu_load[i].busy = 0;
		}
	}

	memcpy(&cfg->pre_usages, &cur, sizeof(cur));

	return 0;
}


s32 monitor_precpu_show_style_one(struct config *config)
{
	char buf[512] = {0};
	u32 blen = 0;
	s32 ret = 0;

	if (!config)
		return -1;

	ret = monitor_percpu_usage_show(config);
	if (ret < 0)
		return -1;

	ret = monitor_show_themp(config);
	if (ret < 0)
		config->themp = -1;

	ret = monitor_percpu_freq_show(config);
	if (ret < 0)
		return -1;

	if ((monitor_magic_switch&0xf) == 0) {

		blen += snprintf(buf, 512,
				//"%11s"
				//"      "
				"--------------------------------------------"
				"CPU[x]<Freq:MHZ>-<Usage:%%>"
				"-------------------------------------------------\n"
				//"%13s"
				" %9s"
				" %12s"
				" %12s"
				" %12s"
				" %12s"
				" %12s"
				" %12s"
				" %12s"
				" %8s"
				//" %13s"
				"\n",
				//"Time",
				//"Update",
				"CPU0","CPU1","CPU2","CPU3",
				"CPU4","CPU5","CPU6","CPU7","Temp");//,"Total-Usage");
		monitor_magic_switch = 0;
	}

	blen += snprintf((buf + blen), 512,
			//"%15.3f"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  |%4d %3d%%|"
			"  %4d \n",
			//"  %8d%% \n",
			//now.tv_sec+now.tv_usec*1e-6,
			config->cpu_freq.cpufreq_list[0], config->cpu_load[0].busy,
			config->cpu_freq.cpufreq_list[1], config->cpu_load[1].busy,
			config->cpu_freq.cpufreq_list[2], config->cpu_load[2].busy,
			config->cpu_freq.cpufreq_list[3], config->cpu_load[3].busy,
			config->cpu_freq.cpufreq_list[4], config->cpu_load[4].busy,
			config->cpu_freq.cpufreq_list[5], config->cpu_load[5].busy,
			config->cpu_freq.cpufreq_list[6], config->cpu_load[6].busy,
			config->cpu_freq.cpufreq_list[7], config->cpu_load[7].busy,
			config->themp);
			//config->usage[8]);
	if(config->output){
		fprintf(config->output, "%s\n", buf);
		fflush(config->output);
	}
	printf("%s\n",buf);
	monitor_magic_switch ++;
	return 0;
}


s32 monitor_precpu_list_show(struct config *config)
{
	//struct timeval now;

	char buf[1024] = {0};
	u32 blen = 0;
	s32 ret = 0;

	if (!config)
		return -1;

	//gettimeofday(&now,NULL);
	ret = monitor_percpu_usage_show(config);
	if (ret < 0)
		return -1;

	ret = monitor_show_themp(config);
	if (ret < 0)
		return -1;

	ret = monitor_percpu_freq_show(config);
	if (ret < 0)
		return -1;

	if ((monitor_magic_switch&0x7) == 0) {

		blen += snprintf(buf, 512,
				//"%11s"
				//"      "
				"--------------------------------------------"
				"CPU[x]<Freq:MHZ> <Usage:%%>-<Idle:%%>-<Iowait:%%>-"
				"-------------------------------------------------\n"
				//"%13s"
				" %9s"
				" %14s"
				" %14s"
				" %14s"
				" %15s"
				" %15s"
				" %15s"
				" %15s"
				" %16s"
				//" %16s"
				"\n",
				//"Time",
				//"Update",
				"CPU0","CPU1","CPU2","CPU3",
				"CPU4","CPU5","CPU6","CPU7","Temp"
				//,"Total-Usage"
				);
		monitor_magic_switch = 0;
	}

	//gettimeofday(&now,NULL);
	blen += snprintf((buf + blen), 512,
			//"%15.3f"
			"%9d "
			"%15d "
			"%14d "
			"%14d "
			"%15d "
			"%15d "
			"%15d "
			"%15d "
			"%15d \n",
			//now.tv_sec+now.tv_usec*1e-6,
			config->cpu_freq.cpufreq_list[0],
			config->cpu_freq.cpufreq_list[1],
			config->cpu_freq.cpufreq_list[2],
			config->cpu_freq.cpufreq_list[3],
			config->cpu_freq.cpufreq_list[4],
			config->cpu_freq.cpufreq_list[5],
			config->cpu_freq.cpufreq_list[6],
			config->cpu_freq.cpufreq_list[7],
			config->themp);

	blen += snprintf((buf + blen), 512,
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%%\t"
			"%3d%%%3d%%%3d%% \n\n",
			config->cpu_load[0].busy, config->cpu_load[0].idle,config->cpu_load[0].iowait,
			config->cpu_load[1].busy, config->cpu_load[1].idle,config->cpu_load[1].iowait,
			config->cpu_load[2].busy, config->cpu_load[2].idle,config->cpu_load[2].iowait,
			config->cpu_load[3].busy, config->cpu_load[3].idle,config->cpu_load[3].iowait,
			config->cpu_load[4].busy, config->cpu_load[4].idle,config->cpu_load[4].iowait,
			config->cpu_load[5].busy, config->cpu_load[5].idle,config->cpu_load[5].iowait,
			config->cpu_load[6].busy, config->cpu_load[6].idle,config->cpu_load[6].iowait,
			config->cpu_load[7].busy, config->cpu_load[7].idle,config->cpu_load[7].iowait);
	if(config->output){
        fprintf(config->output, "%s", buf);
		fflush(config->output);
    }
    printf("%s",buf);
	monitor_magic_switch ++;
	return 0;
}


s32 monitor_precpu_list_shore(struct config *config)
{
	//struct timeval now;

	char buf[1024] = {0};
	u32 blen = 0;
	s32 ret = 0;

	if (!config)
		return -1;

	//gettimeofday(&now,NULL);
	ret = monitor_percpu_usage_show(config);
	if (ret < 0)
		return -1;

	ret = monitor_show_themp(config);
	if (ret < 0)
		return -1;

	ret = monitor_percpu_freq_show(config);
	if (ret < 0)
		return -1;

	//gettimeofday(&now,NULL);
	blen += snprintf((buf + blen), 512,
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d %3d %3d %3d  "
			"%4d\n",
			config->cpu_freq.cpufreq_list[0],config->cpu_load[0].busy, config->cpu_load[0].idle,config->cpu_load[0].iowait,
			config->cpu_freq.cpufreq_list[1],config->cpu_load[1].busy, config->cpu_load[1].idle,config->cpu_load[1].iowait,
			config->cpu_freq.cpufreq_list[2],config->cpu_load[2].busy, config->cpu_load[2].idle,config->cpu_load[2].iowait,
			config->cpu_freq.cpufreq_list[3],config->cpu_load[3].busy, config->cpu_load[3].idle,config->cpu_load[3].iowait,
			config->cpu_freq.cpufreq_list[4],config->cpu_load[4].busy, config->cpu_load[4].idle,config->cpu_load[4].iowait,
			config->cpu_freq.cpufreq_list[5],config->cpu_load[5].busy, config->cpu_load[5].idle,config->cpu_load[5].iowait,
			config->cpu_freq.cpufreq_list[6],config->cpu_load[6].busy, config->cpu_load[6].idle,config->cpu_load[6].iowait,
			config->cpu_freq.cpufreq_list[7],config->cpu_load[7].busy, config->cpu_load[7].idle,config->cpu_load[7].iowait,
			config->themp);

	printf("%s",buf);
	return 0;
}


struct config *monitor_prepare_config(void)
{
	struct config *config = malloc(sizeof(struct config));
	memset(config, 0, sizeof(struct config));
	return config;
}

s32 monitor_release_config(struct config *cfg)
{
	if (cfg)
		free(cfg);
	return 0;
}

FILE *monitor_prepare_output(const char *dirname)
{
	FILE *output = NULL;
	s32 len;
	char *filename;
	struct utsname sysdata;
	DIR *dir;

	dir = opendir(dirname);
	if (dir == NULL) {
		if (mkdir(dirname, 0755)) {
			perror("mkdir");
			fprintf(stderr, "error: Cannot create dir %s\n",
					dirname);
			return NULL;
		}
	}
	closedir(dir);

	len = strlen(dirname) + 30;
	filename = malloc(sizeof(char) * len);

	if (uname(&sysdata) == 0) {
		char *filename_tmp = filename;
		len += strlen(sysdata.nodename) + strlen(sysdata.release);
		filename = realloc(filename, sizeof(char) * len);

		if (filename == NULL) {
			free(filename_tmp);
			perror("realloc");
			return NULL;
		}

		snprintf(filename, len - 1, "%s/CPU-TEST_%s_%s_%li.log",
				dirname, sysdata.nodename, sysdata.release, time(NULL));
	} else {
		snprintf(filename, len - 1, "%s/CPU-TEST_%li.log",
				dirname, time(NULL));
	}

	dprintf("logilename: %s\n", filename);

	output = fopen(filename, "w+");
	if (output == NULL) {
		perror("fopen");
		fprintf(stderr, "error: unable to open logfile\n");
	}

	fprintf(stdout, "Logfile: %s\n", filename);

	free(filename);
	fprintf(output, "#round load sleep performance powersave percentage\n");
	return output;
}

s32 monitor_cpufreq_recore(struct cpufreq_info *cfs)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	if (monitor_magic_switch == 0) {
		cfs->blen += snprintf(cfs->buf, 256,
				"%11s %7s %8s %14s %16s %20s\n",
				"Time","CPU","Cur-F","<Min-Max-F>",
				"Scaling-Cur-F","<Scaling-Min-Max-F>");
		monitor_magic_switch = 1;
	}
	cfs->blen += snprintf((cfs->buf + cfs->blen), 256,
			"%7.3f %3d  %8d %7d-%4d %12d %15d-%4d \n",
			now.tv_sec+now.tv_usec*1e-6,
			cfs->online_cpu,
			cfs->cur_freq/1000,
			cfs->min_freq/1000,
			cfs->max_freq/1000,
			cfs->scaling_cur_freq/1000,
			cfs->scaling_min_freq/1000,
			cfs->scaling_max_freq/1000);
	dprintf("%s", cfs->buf);

	return 0;
}



/**
 * cpu test
 * generates a specific sleep an load time with the performance
 * governor and compares the used time for same calculations done
 * with the configured powersave governor
 *
 * @param config config values for the benchmark
 *
 **/

s32 monitor_cpufreq_start(struct config *config)
{
	u32 i = 0;
	u32 ret = 0;
	struct cpufreq_info pcinfo[CPU_NRS_MAX];
	memset(pcinfo,0,sizeof(struct cpufreq_info)*CPU_NRS_MAX);
	if (!config)
		return -1;
	for (i=0; i<CPU_NRS_MAX; i++)
	{
		ret = get_cpufre_params(i, &pcinfo[i]);
		if (ret == 0) {
			monitor_cpufreq_recore(&pcinfo[i]);
		}
	}
	return 0;
}

s32 monitor_presensor_tempe(struct config *config)
{
	s32 ret = 0;
	char strout[512] = {0};
	u32 strlen = 0;
	struct timeval now;

	if (!config)
		return -1;

	ret = read_sysfs_tempe_state((char *)config->thempre);
	if (ret <0)
		return -1;
	if ((monitor_magic_switch&0x1f) == 0) {
		strlen += snprintf(strout, 512,
			" %13s"
			" %12s"
			" %12s"
			" %12s"
			" %12s"
			" %12s"
			" %12s"
			"\n",
			"Time", "A15 Temp", "A7 Temp",
			"Dram Temp", "GPU Temp",
			"avg Temp", "max Temp");
		monitor_magic_switch = 0;
	}

	gettimeofday(&now,NULL);
	strlen += snprintf((strout + strlen), 512,
			" %15.2f"
			" %7d"
			" %12d"
			" %9d"
			" %13d"
			" %13d"
			" %12d\n",
			now.tv_sec+now.tv_usec*1e-6,
			config->thempre[0],
			config->thempre[3],
			config->thempre[1],
			config->thempre[2],
			config->thempre[5],
			config->thempre[4]);

	dprintf("%s", strout);
	monitor_magic_switch ++;

    //printf("tmep A15 [%d]  A7[%d] Dram[%d] GPU[%d] avg[%d] max[%d]\n",
	//	tmep[0], tmep[3], tmep[1], tmep[2], tmep[4], tmep[5]);
	return 0;
}


/*******************************************************************
  main
 *******************************************************************/

s32 main(s32 argc, char **argv)
{
	s32 c;
	s32 ret;
	s32 option_index = 0;
	struct config *config = NULL;
	u32 i = ~0;
	config = monitor_prepare_config();

	while(1) {
		c = getopt_long (argc, argv, "h::o:c:t:f:v::s::l::",
				monitor_long_options, &option_index);
		if(c == -1)
			break;
		switch (c) {
			case 'l':
				system("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies");
				break;
			case 'o':
				if (config->output != NULL)
					fclose(config->output);
				config->output = monitor_prepare_output(optarg);
				if (config->output == NULL)
					return TEST_EXIT_FAILURE;
				dprintf("user output path -> %s\n", optarg);

				break;
			case 't':
				sscanf(optarg, "%d", &i);
				dprintf("user load time -> %s\n", optarg);
				do {
					ret = monitor_presensor_tempe(config);
					usleep(500000);
				}while((i--) && (ret==0));

				break;
			case 'c':
				sscanf(optarg, "%u", &i);
				dprintf("user cpu test time -> %s\n", optarg);
				do {
					ret = monitor_precpu_list_show(config);
					sleep(1);
				}while((--i) && (ret==0));

				break;
			case 's':
				//strncpy(config->governor, optarg, 14);
				//dprintf("user governor -> %s\n", optarg);
				do {
					ret = monitor_precpu_show_style_one(config);
					sleep(1);
				}while(ret==0);
				break;

			case 'f':
				//if (prepare_config(optarg, config))
				//	return EXIT_FAILURE;
				sscanf(optarg, "%u", &i);
				dprintf("store user cpu test time -> %s\n", optarg);
				do {
					ret = monitor_precpu_list_shore(config);
					sleep(1);
				}while((i--) && (ret==0));

				break;

			case 'v':
				printf("Cpu monitor version Debug-v1.1.2\n");
				break;

			case 'h':
			case '?':
			default:
				if (config != NULL) {
					if (config->output != NULL)
						fclose(config->output);
					free(config);
				}
				monitor_show_usage();
		}
	}
	monitor_release_config(config);
	exit(0);
}
