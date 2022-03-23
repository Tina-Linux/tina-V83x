#ifndef __UVOICE_CONFIG_H__
#define __UVOICE_CONFIG_H__

#include<stdio.h>
#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

struct queue {
	char *data;
        int     size;
        int     readp;
        int     writep;
        pthread_mutex_t mutex;
        pthread_cond_t empty;
        pthread_cond_t full;
};
struct play_queue {
	int data[2];
        pthread_mutex_t play_mutex;
        pthread_cond_t player;
};
struct data_config
{
	int8_t destory_pthr;
        int8_t destory_record;
	int8_t refresh_timer;//0:no Refresh 1:Refresh
        int8_t ns_mode;//0:no ns 1:ns
        int8_t run_mode;//0:nomal  1:factory mode
        unsigned int card; //= 0;
        unsigned int device; //= 0;
        unsigned int channels; //= 2;
        unsigned int rate; //= 16000;
        unsigned int bits; //= 16;
        unsigned int frames;
        unsigned int period_size; //= 1024;
        unsigned int period_count; //= 4;
        unsigned int capture_time; //= UINT_MAX;
	unsigned int play_flag;
        unsigned int get_data_len;
        enum pcm_format format;
	long long end_timer;
	struct play_queue *player_queue;
	struct queue *data_queue;
	//struct queue *online_queue;
	FILE *file;
};
int uvoice_init(struct data_config *init_config,int32_t all_buffers_size);
int creat_record(struct data_config *record_config);
int creat_uvoice_handle(struct data_config *handle_config);
int destory_all(struct data_config *get_config);
int8_t *get_data(struct data_config *get_config);
void refresh_wakeup_timer(struct data_config *get_config,int8_t timer);
void ns_flag_set(struct data_config *get_config,int8_t flag);
void change_run_mode(struct data_config *get_config,int8_t mode);
//int get_asr_data(struct data_config *get_config,void* asr_data);
#ifdef __cplusplus
};
#endif

#endif // END __UVOICE_CONFIG_H__
