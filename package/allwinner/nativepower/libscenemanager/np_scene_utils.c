/*
 * Copyright (C) 2016 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "np_uci_config.h"
#include "np_scene_utils.h"
#include "SceneConfig.h"

int np_get_property(const char *conf_section, const char *conf_name, char *property, size_t len)
{
	NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);
	if (uci == NULL)
		return -1;
	np_uci_read_config(uci, conf_section, conf_name, property, len);
	np_uci_close(uci);
	return 0;
}

int np_set_property(const char *conf_section, const char *conf_name, const char *property)
{
	NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);
	if (uci == NULL)
		return -1;
	np_uci_write_config(uci, conf_section, conf_name, property);
	np_uci_close(uci);
	return 0;
}

int np_get_scene_config(NP_SCENE * scene, const char *scene_name)
{
	int ret = 0;
	NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);
	if (uci == NULL)
		return -1;

	ret += np_uci_read_config(uci, scene_name, "bootlock", scene->cpu.bootlock, sizeof(scene->cpu.bootlock));
	ret += np_uci_read_config(uci, scene_name, "roomage", scene->cpu.roomage, sizeof(scene->cpu.roomage));
	ret += np_uci_read_config(uci, scene_name, "cpu_freq", scene->cpu.cpu_freq, sizeof(scene->cpu.cpu_freq));
	ret += np_uci_read_config(uci, scene_name, "cpu_freq_max", scene->cpu.cpu_freq_max, sizeof(scene->cpu.cpu_freq_max));
	ret += np_uci_read_config(uci, scene_name, "cpu_freq_min", scene->cpu.cpu_freq_min, sizeof(scene->cpu.cpu_freq_min));
	ret += np_uci_read_config(uci, scene_name, "cpu_gov", scene->cpu.cpu_gov, sizeof(scene->cpu.cpu_gov));
	ret += np_uci_read_config(uci, scene_name, "cpu_hot", scene->cpu.cpu_hot, sizeof(scene->cpu.cpu_hot));
	ret += np_uci_read_config(uci, scene_name, "cpu_online", scene->cpu.cpu_online, sizeof(scene->cpu.cpu_online));

	ret += np_uci_read_config(uci, scene_name, "gpu_freq", scene->gpu.gpu_freq, sizeof(scene->gpu.gpu_freq));

	ret += np_uci_read_config(uci, scene_name, "dram_adaptive", scene->dram.dram_adaptive, sizeof(scene->dram.dram_adaptive));
	ret += np_uci_read_config(uci, scene_name, "dram_freq", scene->dram.dram_freq, sizeof(scene->dram.dram_freq));
	ret += np_uci_read_config(uci, scene_name, "dram_freq_max", scene->dram.dram_freq_max, sizeof(scene->dram.dram_freq_max));
	ret += np_uci_read_config(uci, scene_name, "dram_freq_min", scene->dram.dram_freq_min, sizeof(scene->dram.dram_freq_min));

	TLOGI("get uci config:%s\n", scene_name);
	TLOGI("bootlock:%s", scene->cpu.bootlock);
	TLOGI("roomage:%s", scene->cpu.roomage);
	TLOGI("cpu_freq:%s", scene->cpu.cpu_freq);
	TLOGI("cpu_freq_max:%s", scene->cpu.cpu_freq_max);
	TLOGI("cpu_freq_min:%s", scene->cpu.cpu_freq_min);
	TLOGI("cpu_gov:%s", scene->cpu.cpu_gov);
	TLOGI("cpu_hot:%s", scene->cpu.cpu_hot);
	TLOGI("cpu_online:%s", scene->cpu.cpu_online);

	TLOGI("gpu_freq:%s", scene->gpu.gpu_freq);

	TLOGI("dram_adaptive:%s", scene->dram.dram_adaptive);
	TLOGI("dram_freq:%s", scene->dram.dram_freq);
	TLOGI("dram_freq_max:%s", scene->dram.dram_freq_max);
	TLOGI("dram_freq_min:%s", scene->dram.dram_freq_min);

	np_uci_close(uci);
	return ret;
}

static int getConfig(const char *path, char *value, size_t size)
{
	int fd = -1;

	fd = open(path, O_RDONLY);
	TLOGI("open %s return %d\n", path, fd);

	if (fd < 0)
		return -1;

	read(fd, value, size);

	close(fd);
	return 0;
}

static int setConfig(const char *path, const char *value)
{
	int fd = -1;
	TLOGI("path:%s set value:%s", path, value);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;

	write(fd, value, strlen(value));

	close(fd);
	return 0;
}

static int SetBootLock(const char *bootlock)
{
	return setConfig(CPU0LOCK, bootlock);
}

static int SetRoomage(const char *roomage)
{
	return setConfig(ROOMAGE, roomage);
}

static const char *GetActualFreq(char *freq, const char *avail_freq_path)
{
	int ret = 0;

	if (!strcmp(freq, "max"))
		ret = 1;
	else if (!strcmp(freq, "min"))
		ret = 2;

	if (ret != 0) {
		char value[256];
		char *token = NULL;
		size_t len;

		memset(value, 0, sizeof(value));
		if (getConfig(avail_freq_path, value, sizeof(value)) != 0) {
			fprintf(stderr, "avail_freq_path:%s unknown\n", avail_freq_path);
			return NULL;
		}
		TLOGI("avail_freq: %s", value);
		len = strlen(value);
		token = strtok(value, " ");
		if (token == NULL)
			return NULL;
		if (ret == 2)
			strcpy(freq, token);
		else {
			char *token_pre;
			do {
				TLOGI("freq: %s", token);
				token_pre = token;
				token = strtok(NULL, " ");
			} while (token != NULL && (len - (token - value) > 2));
			strcpy(freq, token_pre);
		}
	}
	return freq;
}

static int SetCpuFreq(const char *cpu_freq)
{
	char actualfreq[32];

	strcpy(actualfreq, cpu_freq);
	if(!GetActualFreq(actualfreq, CPUFREQ_AVAIL))
		return -1;
	setConfig(CPUFREQ_MAX, actualfreq);
	setConfig(CPUFREQ_MIN, actualfreq);

	return 0;
}

static int SetCpuFreqMax(const char *cpu_freq_max)
{
	return setConfig(CPUFREQ_MAX, cpu_freq_max);
}

static int SetCpuFreqMin(const char *cpu_freq_min)
{
	return setConfig(CPUFREQ_MIN, cpu_freq_min);
}

static int GetCpuFreq(char *buf, size_t len)
{
	return getConfig(CPUFREQ, buf, len);
}

static int GetCpuOnline(char *buf, size_t len)
{
	return getConfig(CPUONLINE, buf, len);
}

static int GetCpuGov(char *buf, size_t len)
{
	return getConfig(CPU0GOV, buf, len);
}

static int SetCpuGov(const char *cpu_gov)
{
	return setConfig(CPU0GOV, cpu_gov);
}

static int SetCpuHot(const char *cpu_hot)
{
	return setConfig(CPUHOT, cpu_hot);
}

static int SetCpuOnline(const char *cpu_online)
{
	char online[CPU_NUM_MAX];
	int num;

	if (!strcmp(cpu_online, "all")) {
		memset(online, 1, sizeof(online));
	} else {
		char cpu_online_tmp[sizeof(((CPU_SCENE *) 0)->cpu_online)];
		char *token = NULL;
		memset(online, 0, sizeof(online));
		memcpy(cpu_online_tmp, cpu_online, sizeof(cpu_online_tmp));
		token = strtok(cpu_online_tmp, "-,");
		while (token != NULL) {
			num = atoi(token);
			if (num < 0 || num > (CPU_NUM_MAX - 1))
				return -1;
			online[num] = 1;
			if (cpu_online[token - cpu_online_tmp + 1] == '-') {
				int start = num;
				int end, i;
				if (token - cpu_online_tmp + 2 < sizeof(cpu_online_tmp))
					end = cpu_online[token - cpu_online_tmp + 2] - '0';
				else
					return -1;
				if (end < start)
					return -1;
				for (i = start; i <= end; i++)
					online[i] = 1;
			}
			token = strtok(NULL, "-,");
		}
	}
	/* ignore CPU0 */
	for (num = 1; num < CPU_NUM_MAX; num++) {
		char buf[128];
		snprintf(buf, sizeof(buf), "/sys/devices/system/cpu/cpu%d/online", num);
		setConfig(buf, online[num] != 0 ? "1" : "0");
	}
	return 0;
}

static int SetDramFreqAdaptive(const char *pause)
{
	return setConfig(DRAMMODE, pause);
}

static int SetDramFreq(const char *dram_freq)
{
	char actualfreq[32];

	strcpy(actualfreq, dram_freq);
	if(!GetActualFreq(actualfreq, DRAMFREQ_AVAIL))
		return -1;
	if (atoi(actualfreq)/1000000U > 0)
		snprintf(actualfreq, sizeof(actualfreq), "%d", atoi(actualfreq) / 1000);
	setConfig(DRAMFREQ_MAX, actualfreq);
	setConfig(DRAMFREQ_MIN, actualfreq);

	return 0;
}

static int SetDramFreqMax(const char *dram_freq_max)
{
	return setConfig(DRAMFREQ_MAX, dram_freq_max);
}

static int SetDramFreqMin(const char *dram_freq_min)
{
	return setConfig(DRAMFREQ_MIN, dram_freq_min);
}

static int GetDramFreq(char *buf, size_t len)
{
	return getConfig(DRAMFREQ, buf, len);
}

static int SetGpuFreq(const char *gpu_freq)
{
	return -1;
}

static int GetGpuFreq(char *buf, size_t len)
{
	return -1;
}

static CPU_SCENE_OPS cpu_ops = {
	.SetBootLock = SetBootLock,
	.SetRoomAge = SetRoomage,
	.SetCpuFreq = SetCpuFreq,
	.SetCpuFreqMax = SetCpuFreqMax,
	.SetCpuFreqMin = SetCpuFreqMin,
	.SetCpuGov = SetCpuGov,
	.SetCpuHot = SetCpuHot,
	.SetCpuOnline = SetCpuOnline,

	.GetCpuFreq = GetCpuFreq,
	.GetCpuGov = GetCpuGov,
	.GetCpuOnline = GetCpuOnline,
};

GPU_SCENE_OPS gpu_ops = {
	.SetGpuFreq = SetGpuFreq,
	.GetGpuFreq = GetGpuFreq,
};

DRAM_SCENE_OPS dram_ops = {
	.SetDramFreqAdaptive = SetDramFreqAdaptive,
	.SetDramFreq = SetDramFreq,
	.SetDramFreqMax = SetDramFreqMax,
	.SetDramFreqMin = SetDramFreqMin,

	.GetDramFreq = GetDramFreq,
};

void *np_get_ops(int sel)
{
	void *ops = NULL;
	switch (sel) {
	case 0:
		ops = &cpu_ops;
		break;
	case 1:
		ops = &dram_ops;
		break;
	case 2:
		ops = &gpu_ops;
		break;
	}
	return ops;
}

int np_set_scene(NP_SCENE * scene)
{
	if (strlen(scene->cpu.bootlock) != 0)
		cpu_ops.SetBootLock(scene->cpu.bootlock);

	if (strlen(scene->cpu.roomage) != 0)
		cpu_ops.SetRoomAge(scene->cpu.roomage);

	if (strlen(scene->cpu.cpu_freq) != 0)
		cpu_ops.SetCpuFreq(scene->cpu.cpu_freq);

	if (strlen(scene->cpu.cpu_freq_max) != 0)
		cpu_ops.SetCpuFreqMax(scene->cpu.cpu_freq_max);

	if (strlen(scene->cpu.cpu_freq_min) != 0)
		cpu_ops.SetCpuFreqMin(scene->cpu.cpu_freq_min);

	if (strlen(scene->cpu.cpu_gov) != 0)
		cpu_ops.SetCpuGov(scene->cpu.cpu_gov);

	if (strlen(scene->cpu.cpu_hot) != 0)
		cpu_ops.SetCpuHot(scene->cpu.cpu_hot);

	if (strlen(scene->cpu.cpu_online) != 0)
		cpu_ops.SetCpuOnline(scene->cpu.cpu_online);

	if (strlen(scene->gpu.gpu_freq) != 0)
		gpu_ops.SetGpuFreq(scene->gpu.gpu_freq);

	if (strlen(scene->dram.dram_adaptive) != 0)
		dram_ops.SetDramFreqAdaptive(scene->dram.dram_adaptive);

	if (strlen(scene->dram.dram_freq) != 0)
		dram_ops.SetDramFreq(scene->dram.dram_freq);

	if (strlen(scene->dram.dram_freq_max) != 0)
		dram_ops.SetDramFreqMax(scene->dram.dram_freq_max);

	if (strlen(scene->dram.dram_freq_min) != 0)
		dram_ops.SetDramFreqMin(scene->dram.dram_freq_min);

	return 0;
}
