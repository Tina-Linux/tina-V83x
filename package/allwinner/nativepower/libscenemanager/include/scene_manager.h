#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__
#include <unistd.h>

#if __cplusplus
extern "C" {
#endif

int np_scene_get(char *scene_name, size_t len);
int np_scene_change(const char *scene_name);

int np_scene_property_get(const char *conf_name, char *conf_buf, size_t len);
int np_scene_property_set(char *conf_name, char *conf_buf);

int acquire_wake_lock(const char *id);
int release_wake_lock(const char *id);
int get_wake_lock_count(void);

int np_cpu_freq_get(char *freq, size_t len);
int np_cpu_gov_get(char *gov, size_t len);
int np_cpu_online_get(char *online, size_t len);
int np_dram_freq_get(char *freq, size_t len);
int np_gpu_freq_get(char *freq, size_t len);

int np_cpu_freq_set(const char *freq);
int np_dram_freq_set(const char *freq);
int np_gpu_freq_set(const char *freq);

#if __cplusplus
}
#endif
#endif
