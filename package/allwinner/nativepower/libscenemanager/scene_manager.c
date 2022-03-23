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
#include <stdio.h>
#include <string.h>

#include "scene_manager.h"
#include "np_scene_utils.h"

#define PROP_SECTION		"properties"
#define PROP_OPTION_CUR_SCENE	"current_scene"

int np_scene_get(char *scene_name, size_t len)
{
	np_get_property(PROP_SECTION, PROP_OPTION_CUR_SCENE, scene_name, len);
	return 0;
}

int np_scene_change(const char *scene_name)
{
	NP_SCENE scene;

	memset(&scene, 0, sizeof(NP_SCENE));

	if (np_get_scene_config(&scene, scene_name) < 0)
		return -1;

	np_set_property(PROP_SECTION, PROP_OPTION_CUR_SCENE, scene_name);
	np_set_scene(&scene);
	return 0;
}

int np_scene_property_get(const char *conf_name, char *conf_buf, size_t len)
{
	return np_get_property(PROP_SECTION, conf_name, conf_buf, len);
}

int np_scene_property_set(char *conf_name, char *conf_buf)
{
	return np_set_property(PROP_SECTION, conf_name, conf_buf);
}

int np_cpu_freq_get(char *freq, size_t len)
{
	CPU_SCENE_OPS *ops;

	ops = np_get_ops(0);
	if (!ops || !ops->GetCpuFreq)
		return -1;
	return ops->GetCpuFreq(freq, len);
}

int np_cpu_freq_set(const char *freq)
{
	CPU_SCENE_OPS *ops;

	ops = np_get_ops(0);
	if (!ops || !ops->SetCpuFreq)
		return -1;
	return ops->SetCpuFreq(freq);
}

int np_cpu_gov_get(char *gov, size_t len)
{
	CPU_SCENE_OPS *ops;

	ops = np_get_ops(0);
	if (!ops || !ops->GetCpuGov)
		return -1;
	return ops->GetCpuGov(gov, len);
}

int np_cpu_online_get(char *online, size_t len)
{
	CPU_SCENE_OPS *ops;

	ops = np_get_ops(0);
	if (!ops || !ops->GetCpuOnline)
		return -1;
	return ops->GetCpuOnline(online, len);
}

int np_dram_freq_get(char *freq, size_t len)
{
	DRAM_SCENE_OPS *ops;

	ops = np_get_ops(1);
	if (!ops || !ops->GetDramFreq)
		return -1;
	return ops->GetDramFreq(freq, len);
}

int np_dram_freq_set(const char *freq)
{
	DRAM_SCENE_OPS *ops;

	ops = np_get_ops(1);
	if (!ops || !ops->SetDramFreq)
		return -1;
	return ops->SetDramFreq(freq);
}

int np_gpu_freq_get(char *freq, size_t len)
{
	GPU_SCENE_OPS *ops;

	ops = np_get_ops(2);
	if (!ops || !ops->GetGpuFreq)
		return -1;
	return ops->GetGpuFreq(freq, len);
}

int np_gpu_freq_set(const char *freq)
{
	GPU_SCENE_OPS *ops;

	ops = np_get_ops(2);
	if (!ops || !ops->SetGpuFreq)
		return -1;
	return ops->SetGpuFreq(freq);
}
