#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>

#include <scene_manager.h>

#ifdef USE_DBUS
#include "power_manager_client.h"

#define SEL_SUSPEND	"suspend"
#define SEL_SHUTDOWN	"shutdown"
#define SEL_REBOOT		"reboot"
#define SEL_ACQUIRE	"acquire_wakelock"
#define SEL_RELEASE	"release_wakelock"
#define SEL_USRACTIVITY "useractivity"
#define SEL_SETWAKETIME "setwaketime"
#define SEL_SETSCENE	"setscene"

static void usage(char *name)
{
	printf("%s usage:\n", name);
	printf("%s sel [arg]\n", name);
	printf("sel:\n%-20s%-20s%-20s%-20s\n", SEL_SUSPEND, SEL_SHUTDOWN, SEL_REBOOT, SEL_ACQUIRE);
	printf("%-20s%-20s%-20s%-20s\n", SEL_RELEASE, SEL_USRACTIVITY, SEL_SETWAKETIME, SEL_SETSCENE);
	exit(1);
}
#endif

static void utils_usage(void)
{
	printf("Usage:\n");
	printf("nativepower_utils [OPTION [ARG]]\n");
	printf(" -h, --help        show this help statement\n");
	printf(" -m, --module      contains scene, cpu, dram, gpu\n");
	printf(" -g, --get         get the specify module info,default\n");
	printf(" -s, --set <para>  set module para\n");
	printf("		   para:scene name - detail info in /etc/config/nativepower\n");
	printf("		        cpu,dram   - keep freq in the specfy para\n");
	printf(" -l, --wakelock    acquire wakelock\n");
	printf(" -u, --wakeunlock  release wakelock\n");
	printf("\n");
	printf("Example: nativepower_utils -m scene -s scene boot_complete\n");
	printf("\n");
	exit(1);
}

static int nativepower_utils(int argc, char *argv[])
{
	int ret = 0;
	enum {
		MODULE_SCENE = 0,
		MODULE_CPU,
		MODULE_DRAM,
		MODULE_GPU,
	} module = MODULE_SCENE;
	enum {
		METHOD_GET = 0,
		METHOD_SET,
		METHOD_LOCK,
		METHOD_UNLOCK,
	} method = METHOD_GET;
	union {
		char module_para[32];
		char set_para[32];
		char wakelock_para[32];
	} para;

	memset(&para, 0, sizeof(para));
	while (1) {
		const struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"module", required_argument, NULL, 'm'},
			{"set", required_argument, NULL, 's'},
			{"get", no_argument, NULL, 'g'},
			{"wakelock", required_argument, NULL, 'l'},
			{"wakeunlock", required_argument, NULL, 'u'},
		};
		int option_index = 0;
		int c = 0;
		c = getopt_long(argc, argv, "hm:s:gl:u:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			utils_usage();
			break;
		case 'm':
			if (!strcmp(optarg, "scene"))
				module = MODULE_SCENE;
			else if (!strcmp(optarg, "cpu"))
				module = MODULE_CPU;
			else if (!strcmp(optarg, "dram"))
				module = MODULE_DRAM;
			else if (!strcmp(optarg, "gpu"))
				module = MODULE_GPU;
			else
				utils_usage();
			break;
		case 's':
			method = METHOD_SET;
			strcpy(para.set_para, optarg);
			break;
		case 'g':
			method = METHOD_GET;
			break;
		case 'l':
			method = METHOD_LOCK;
			strcpy(para.wakelock_para, optarg);
			break;
		case 'u':
			method = METHOD_UNLOCK;
			strcpy(para.wakelock_para, optarg);
			break;
		default:
			utils_usage();
			break;
		}
	}

	if (optind < 2)
		utils_usage();

	if (method == METHOD_GET) {
		if (module == MODULE_SCENE) {
			char scene_name[32];
			memset(scene_name, 0, sizeof(scene_name));
			ret = np_scene_get(scene_name, sizeof(scene_name));
			printf("current scene:\"%s\"\n", strlen(scene_name) ? scene_name : "unknown");
		} else if (module == MODULE_CPU) {
			char cpu_freq[32];
			ret = np_cpu_freq_get(cpu_freq, sizeof(cpu_freq));
			if (ret != 0)
				printf("get cpufreq failed\n");
			else
				printf("get cpufreq successful, Freq:%s\n", cpu_freq);
		} else if (module == MODULE_DRAM) {
			char dram_freq[32];
			ret = np_dram_freq_get(dram_freq, sizeof(dram_freq));
			if (ret != 0)
				printf("get dramfreq failed\n");
			else
				printf("get dramfreq successful, Freq:%s\n", dram_freq);
		} else if (module == MODULE_GPU) {
			char gpu_freq[32];
			ret = np_gpu_freq_get(gpu_freq, sizeof(gpu_freq));
			if (ret != 0)
				printf("get gpufreq failed\n");
			else
				printf("get gpufreq successful, Freq:%s\n", gpu_freq);
		}
	} else if (method == METHOD_SET) {
		if (module == MODULE_SCENE) {
			ret = np_scene_change(para.set_para);
			if (ret != 0)
				printf("set scene:\"%s\" failed\n", para.set_para);
			else
				printf("set scene:\"%s\" successful\n", para.set_para);
		} else if (module == MODULE_CPU) {
			ret = np_cpu_freq_set(para.set_para);
			if (ret != 0)
				printf("set cpufreq:%s failed\n", para.set_para);
			else {
				char cpu_freq[32];
				np_cpu_freq_get(cpu_freq, sizeof(cpu_freq));
				printf("set cpufreq:%s complete\n", para.set_para);
				printf("now cpufreq is %s\n", cpu_freq);
			}
		} else if (module == MODULE_DRAM) {
			ret = np_dram_freq_set(para.set_para);
			if (ret != 0)
				printf("set dramfreq:%s failed\n", para.set_para);
			else {
				char dram_freq[32];
				np_dram_freq_get(dram_freq, sizeof(dram_freq));
				printf("set dramfreq:%s complete\n", para.set_para);
				printf("now dramfreq is %s\n", dram_freq);
			}
		} else if (module == MODULE_GPU) {
			ret = np_gpu_freq_set(para.set_para);
			if (ret != 0)
				printf("set gpufreq:%s failed\n", para.set_para);
			else {
				char gpu_freq[32];
				np_gpu_freq_get(gpu_freq, sizeof(gpu_freq));
				printf("set gpufreq:%s complete\n", para.set_para);
				printf("now gpufreq is %s\n", gpu_freq);
			}
		}
	} else if (method == METHOD_LOCK) {
		/* ignore module */
		ret = acquire_wake_lock(para.wakelock_para);
	} else if (method == METHOD_UNLOCK) {
		/* ignore module */
		ret = release_wake_lock(para.wakelock_para);
	}

	return ret;
}

#ifndef USE_DBUS
int main(int argc, char *argv[])
{
	if (!strcmp(basename(argv[0]), "nativepower_utils"))
		return nativepower_utils(argc, argv);
	else
		printf("%s failed\n", argv[0]);
}
#else
int main(int argc, char **argv)
{
	int ret;
	unsigned int sel = 0;

	if (!strcmp(basename(argv[0]), "nativepower_utils"))
		return nativepower_utils(argc, argv);

	if (argc == 1)
		usage(argv[0]);
	else if (argc > 1) {
		if (!strcmp(SEL_SUSPEND, argv[1]))
			sel = 0;
		else if (!strcmp(SEL_SHUTDOWN, argv[1]))
			sel = 1;
		else if (!strcmp(SEL_REBOOT, argv[1]))
			sel = 2;
		else if (!strcmp(SEL_ACQUIRE, argv[1]) && argc == 3)
			sel = 3;
		else if (!strcmp(SEL_RELEASE, argv[1]) && argc == 3)
			sel = 4;
		else if (!strcmp(SEL_USRACTIVITY, argv[1]))
			sel = 5;
		else if (!strcmp(SEL_SETWAKETIME, argv[1]) && argc == 3)
			sel = 6;
		else if (!strcmp(SEL_SETSCENE, argv[1]) && argc <= 3)
			sel = 7;
		else
			usage(argv[0]);
	}

	switch (sel) {
	case 0:
		ret = PowerManagerSuspend(0);
		break;
	case 1:
		ret = PowerManagerShutDown();
		break;
	case 2:
		ret = PowerManagerReboot();
		break;
	case 3:
		ret = PowerManagerAcquireWakeLock(argv[2]);
		break;
	case 4:
		ret = PowerManagerReleaseWakeLock(argv[2]);
		break;
	case 5:
		ret = PowerManagerUserActivity();
		break;
	case 6:
		ret = PowerManagerSetAwakeTimeout(atoi(argv[2]));
		break;
	case 7:
		if (argc == 3)
			ret = PowerManagerSetScene(argv[2]);
		else
			ret = PowerManagerSetScene("boot_complete");
		break;
	}
	printf("return[%d] by sending cmd: %s\n", ret, argv[1]);
	return ret;
}
#endif
