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

struct queue {
	char *data;
	int size;
	int readp;
	int writep;
	pthread_mutex_t mutex;
	pthread_cond_t empty;
	pthread_cond_t full;
};

struct play_queue {
	char data[2];
	pthread_mutex_t play_mutex;
	pthread_cond_t player;
};

struct data_config {
	unsigned int card;
	unsigned int device;
	unsigned int channels;
	unsigned int rate;
	unsigned int bits;
	unsigned int frames;
	unsigned int period_size;
	unsigned int period_count;
	unsigned int capture_time;
	int play_flag;
	unsigned int get_data_len;
	enum pcm_format format;
	struct play_queue *player_queue;
	struct queue *data_queue;
	FILE *file;
};

int uvoice_init(struct data_config *init_config, int32_t all_buffers_size);
int creat_record(struct data_config *record_config);
int creat_uvoice_handle(struct data_config *handle_config);
int8_t *get_data(struct data_config *get_config);
int destory_all();
