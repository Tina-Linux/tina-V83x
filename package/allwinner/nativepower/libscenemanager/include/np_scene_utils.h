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

#ifndef __NP_SCENE_UTILS_H__
#define __NP_SCENE_UTILS_H__

typedef struct {
	int (*SetBootLock) (const char *boot_lock);
	int (*SetRoomAge) (const char *room_age);
	int (*SetCpuFreq) (const char *cpu_freq);
	int (*SetCpuFreqMax) (const char *cpu_freq_max);
	int (*SetCpuFreqMin) (const char *cpu_freq_min);
	int (*SetCpuGov) (const char *cpu_gov);
	int (*SetCpuHot) (const char *cpu_hot);
	int (*SetCpuOnline) (const char *cpu_online);

	int (*GetCpuFreq) (char *cpu_freq, size_t len);
	int (*GetCpuGov) (char *gov, size_t len);
	int (*GetCpuOnline) (char *cpu_online, size_t len);
} CPU_SCENE_OPS;

typedef struct {
	int (*SetGpuFreq) (const char *gpu_freq);

	int (*GetGpuFreq) (char *gpu_freq, int len);
} GPU_SCENE_OPS;

typedef struct {
	int (*SetDramFreqAdaptive) (const char *pause);
	int (*SetDramFreq) (const char *dram_freq);
	int (*SetDramFreqMax) (const char *dram_freq_max);
	int (*SetDramFreqMin) (const char *dram_freq_min);

	int (*GetDramFreq) (char *dram_freq, size_t len);
} DRAM_SCENE_OPS;

typedef struct {
	char bootlock[4];
	char cpu_freq[16];
	char cpu_freq_max[16];
	char cpu_freq_min[16];
	char roomage[64];
	char cpu_gov[16];
	char cpu_hot[16];
	char cpu_online[16];
} CPU_SCENE;

typedef struct {
	char gpu_freq[16];
} GPU_SCENE;

typedef struct {
	char dram_adaptive[4];
	char dram_freq[16];
	char dram_freq_max[16];
	char dram_freq_min[16];
} DRAM_SCENE;

typedef struct {
	CPU_SCENE cpu;
	GPU_SCENE gpu;
	DRAM_SCENE dram;
} NP_SCENE;

void *np_get_ops(int sel);
int np_get_scene_config(NP_SCENE * scene, const char *scene_name);
int np_set_scene(NP_SCENE * scene);
int np_get_property(const char *conf_section, const char *conf_name, char *scene, size_t len);
int np_set_property(const char *conf_section, const char *conf_name, const char *scene);
#endif
