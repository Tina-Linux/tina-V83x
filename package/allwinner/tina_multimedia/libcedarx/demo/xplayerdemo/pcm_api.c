/* pcm.c
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#include <linux/ioctl.h>
#define __force
#define __bitwise
#define __user
#include <sound/asound.h>

#include <asoundlib.h>

#define PARAM_MAX SNDRV_PCM_HW_PARAM_LAST_INTERVAL
#define SNDRV_PCM_HW_PARAMS_NO_PERIOD_WAKEUP (1<<2)
#define LG_MAX      0x7fffffff


#define CLICK_VOICE_KEY_PATH	"./resource/key.wav"
#define CLICK_VOICE_CAP_PATH	"./resource/capture.wav"
#define CLICK_VOICE_BUF_SIZE	(4*1024)
#define CLICK_VOICE_FILE_NUM	(2)

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct wav_header {
    unsigned int   riff_id;
    unsigned int   riff_sz;
    unsigned int   riff_fmt;
    unsigned int   fmt_id;
    unsigned int   fmt_sz;
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int   sample_rate;
    unsigned int   byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
    unsigned int   data_id;
    unsigned int   data_sz;
};

#define FORMAT_PCM 1


struct click_voice_st {
	int             click_voice_fds[CLICK_VOICE_FILE_NUM];
	char           *click_voice_buf;
	int             click_voice_init_flag;
	sem_t           click_voice_sem;
	pthread_mutex_t click_voice_lock;
	pthread_t       click_voice_tid;
};

static struct click_voice_st click_voice_context;

//static pthread_mutex_t pcm_write_lock = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t click_lock = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t  pcm_write_cond = PTHREAD_COND_INITIALIZER;


struct pcm *pcm_p = NULL;
struct pcm *pcm_c = NULL;
//static int key_voice_file = -1;
//static unsigned int key_clicks = 0;

static sem_t  sem_pcm_p;
extern int pcm_resume(struct pcm *pcm);

static void tinymix_set_value(struct mixer *mixer, unsigned int id,
                              char *string)
{
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values;
    unsigned int i;

    ctl = mixer_get_ctl(mixer, id);
    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);

    if (isdigit(string[0])) {
        int value = atoi(string);

        for (i = 0; i < num_values; i++) {
            if (mixer_ctl_set_value(ctl, i, value)) {
                fprintf(stderr, "Error: invalid value\n");
                return;
            }
        }
    } else {
        if (type == MIXER_CTL_TYPE_ENUM) {
            if (mixer_ctl_set_enum_by_string(ctl, string))
                fprintf(stderr, "Error: invalid enum value\n");
        } else {
            fprintf(stderr, "Error: only enum types can be set with strings\n");
        }
    }
}

static void tinymix_get_value(struct mixer *mixer, unsigned int id, int *value)
{
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;

	if(!value) {
		printf("invalid value pointer\n");
		return ;
	}

	if(id >= mixer_get_num_ctls(mixer)) {
        fprintf(stderr, "Invalid mixer id\n");
        return;
    }

	ctl = mixer_get_ctl(mixer, id);
	type = mixer_ctl_get_type(ctl);

	if(type == MIXER_CTL_TYPE_INT) {
		*value = mixer_ctl_get_value(ctl, 0);
	}
	if(type == MIXER_CTL_TYPE_BOOL) {
		*value = mixer_ctl_get_value(ctl, 0);
	}

}



#if 0

static void open_key_voice(void)
{
	key_voice_file = open(VOICE_FILE, O_RDWR);
	if(key_voice_file == -1) {
		fprintf(stderr, "open key voice file fail\n");
		return ;
	}

	lseek(key_voice_file, 44, SEEK_SET);
}


static void close_key_voice(void)
{
	if(key_voice_file >= 0) close(key_voice_file);
	key_voice_file = -1;
}

static void click_key_voice(void)
{
//	pthread_mutex_lock(&click_lock);
	key_clicks++;
//	pthread_mutex_unlock(&click_lock);
//	pthread_cond_signal(&pcm_write_cond);
}

#define ARCH_ADD(p,a) ((p) += (a))
static void alsa_mix_16(short data1,short data2,short *date_mix)
{
	int sample, old_sample,sum;
	sample = data1;
	old_sample = data2;
	sum = data2;

	if(*date_mix == 0)
		sample -= old_sample;
	ARCH_ADD(sum,sample);
	do{
		old_sample = sum;
		if(old_sample > 0x7fff)
			sample = 0x7fff;
		else if(old_sample < -0x8000)
			sample = -0x8000;
		else
			sample = old_sample;
		*date_mix = sample;
	}while(0);
}

static void convert_out(char *src1, char *src2, char *dst, int size)
{
	int i = 0, cnt = size/2;

	if(!cnt) {
		fprintf(stderr, "invalid size\n");
		return ;
	}

	for(i = 0; i < cnt; i++){
		alsa_mix_16(((short *)src1)[i],((short *)src2)[i], ((short *)dst)+i);
	}

}

static void add_key_voice(void *data, unsigned int count)
{
	char *buf;
	int cnt;

	buf = malloc(count);
	if(!buf) {
		fprintf(stderr, "malloc err\n");
		return ;
	}

	cnt = read(key_voice_file, buf, count);
	if(cnt < 0) {
		fprintf(stderr, "read voice file error\n");
		return ;
	}

	if(cnt < count) {
		pthread_mutex_lock(&click_lock);
		key_clicks--;
		pthread_mutex_unlock(&click_lock);
		lseek(key_voice_file, 44, SEEK_SET);
	}
	convert_out(data, buf, data, cnt);
	free(buf);
}

void play_key_voice(void)
{
	char *buf;
	int cnt, ret;

	buf = malloc(1024);
	if(!buf) {
		fprintf(stderr, "malloc err\n");
		return ;
	}
	cnt = read(key_voice_file, buf, 1024);
	if(cnt < 0) {
		fprintf(stderr, "read voice file error\n");
		return ;
	}

	if(cnt < 1024) {
		pthread_mutex_lock(&click_lock);
		key_clicks--;
		pthread_mutex_unlock(&click_lock);
		lseek(key_voice_file, 44, SEEK_SET);
	}
	ret = pcm_write(pcm_p, buf, cnt);
	if(ret != 0) {
		fprintf(stderr, "paly voice err\n");
	}
	free(buf);
}


static int mixer_play(struct play_queue *queue)
{
	int ret;

	if(!queue || !queue->size) {
		fprintf(stderr, "mixer play error queue\n");
		return -1;
	}

	add_key_voice(queue->buf, queue->size);
	ret = pcm_write(pcm_p, queue->buf, queue->size);

	return ret;
}


static void *key_voice_thread_function(void *pvoid)
{
#if 0
	struct play_queue *queue;
	int ret;

	while(1) {
		pthread_mutex_lock(&pcm_write_lock);
		while(!is_buf_ready())
			pthread_cond_wait(&pcm_write_cond, &pcm_write_lock);
		pthread_mutex_unlock(&pcm_write_lock);

		if(!is_empty_queue() && key_clicks > 0) { /* mixer play */
			queue = get_play_queue();
			if(queue) {
				mixer_play(queue);
				destroy_queue(queue);
				queue = NULL;
			}
		}else if(!is_empty_queue()) {/* normal play */
			queue = get_play_queue();
			if(queue) {
				ret = pcm_write(pcm_p, queue->buf, queue->size);
				destroy_queue(queue);
				queue = NULL;
			}
		}else if(key_clicks > 0) { /* key void only */
			play_key_voice();
		}else {
			printf("paly nothing\n");
		}
	}
#endif
	return NULL;
}

#endif

#if 0
static void key_voice_init(void)
{
	int err = 0;

	open_key_voice();

	/* create pthread */
	err = pthread_create(&key_voice_tid, NULL, key_voice_thread_function, NULL);
	if(err != 0) {
		fprintf(stderr, "create pthread error!\n");
		goto fail;
	}

	return ;

fail:
	close_key_voice();
}
static void key_voice_exit(void)
{

	pthread_cancel(key_voice_tid);
	close_key_voice();
}

#endif

static int _pcm_open(unsigned int flags, struct pcm_config *cfg)
{
	struct pcm *pcm = NULL;
	struct mixer *mixer;

	if(flags & PCM_OUT) {
		if(click_voice_context.click_voice_init_flag) {
			if(flags&PCM_OUT_WAIT) {
				sem_wait(&sem_pcm_p);
			}else if(flags&PCM_OUT_TRY) {
				if(sem_trywait(&sem_pcm_p) != 0) {
					return -1;
				}
			}
		}
		if(pcm_p) {
			printf("The play device has already creat!\n");

			if(click_voice_context.click_voice_init_flag) sem_post(&sem_pcm_p);
			return -1;
		}
		pcm = pcm_open(0, 0, flags & (~PCM_IN), cfg);

		if (!pcm || !pcm_is_ready(pcm)) {
			fprintf(stderr, "Unable to open PCM device %d (%s)\n",
					0, pcm_get_error(pcm));
			if(click_voice_context.click_voice_init_flag) sem_post(&sem_pcm_p);
			return -1;
		} else {
			pcm_p = pcm;
		//	printf("create pcm out suceessfully\n");
		}
	} else if(flags & PCM_IN) {
		if(pcm_c){
			printf("The catpure device has already creat!\n");
			return -1;
		}
		pcm = pcm_open(0, 0, flags & (~PCM_OUT), cfg);
		if (!pcm || !pcm_is_ready(pcm)) {
			fprintf(stderr, "Unable to open PCM device (%s)\n",
					pcm_get_error(pcm));
			return -1;
		} else {
			pcm_c = pcm;
		}

		mixer = mixer_open(0);
		if (!mixer) {
			fprintf(stderr, "Failed to open mixer\n");
			return 0;
		}

		tinymix_set_value(mixer, 35, "1");
	//	tinymix_set_value(mixer, 27, "1");
		mixer_close(mixer);
	} else {
		fprintf(stderr, "Invalid param\n");
		return -1;
	}

	return 0;

}

static int click_voice_play_file(char *buf, int fd)
{
	struct wav_header play_header;
	struct pcm_config cfg;
	ssize_t size = 0;

	if(!buf) return -1;

	lseek(fd, 0, SEEK_SET);
	size = read(fd, &play_header, sizeof(struct wav_header));
	if(size != sizeof(struct wav_header)) {
		printf("%s:read voice file error\n", __func__);
		return -1;
	}

	/* click key voice */
	cfg.channels		  = play_header.num_channels;
	cfg.rate			  = play_header.sample_rate;
	cfg.period_size		  = 512;
	cfg.period_count	  = 2;
	cfg.format			  = play_header.bits_per_sample;
	cfg.start_threshold   = 0;
	cfg.stop_threshold	  = 0;
	cfg.silence_threshold = 0;

	if(init_pcm(PCM_OUT|PCM_OUT_TRY, &cfg)) {
		printf("%s:open pcm error\n", __func__);
		return -1;
	}

	lseek(fd, 44, SEEK_SET);
	while((size = read(fd, buf, CLICK_VOICE_BUF_SIZE)) > 0) {
		pcm_write_api(buf, (unsigned int)size);
	}

	uninit_pcm(PCM_OUT);


	return 0;
}
static void *voice_thread_function(void *pvoid)
{
	struct click_voice_st *pclick_voice_ctx = (struct click_voice_st *)pvoid;
	int i;

	while(1) {
		sem_wait(&pclick_voice_ctx->click_voice_sem);

		if(!pclick_voice_ctx->click_voice_init_flag) {
			break;
		}

		for(i = 0; i < CLICK_VOICE_FILE_NUM; i++) {
			if(pclick_voice_ctx->click_voice_fds[i] >= 0) {
				click_voice_play_file(pclick_voice_ctx->click_voice_buf, pclick_voice_ctx->click_voice_fds[i]);
				close(pclick_voice_ctx->click_voice_fds[i]);
				pthread_mutex_lock(&click_voice_context.click_voice_lock);
				pclick_voice_ctx->click_voice_fds[i] = -1;
				pthread_mutex_unlock(&click_voice_context.click_voice_lock);
			}
		}
	}

	return NULL;
}

int audio_module_init(void)
{
	int ret = 0;
	int i;

	sem_init(&click_voice_context.click_voice_sem, 0, 0);
	pthread_mutex_init(&click_voice_context.click_voice_lock, NULL);
	click_voice_context.click_voice_buf = (char *)malloc(CLICK_VOICE_BUF_SIZE);
	if(!click_voice_context.click_voice_buf) {
		printf("%s:malloc error\n", __func__);
		return -1;
	}

	sem_init(&sem_pcm_p, 0, 1);
	sem_init(&click_voice_context.click_voice_sem, 0, 1);
	ret = pthread_create(&click_voice_context.click_voice_tid, NULL, voice_thread_function, &click_voice_context);
	if(ret != 0) {
		pthread_mutex_destroy(&click_voice_context.click_voice_lock);
		sem_destroy(&click_voice_context.click_voice_sem);
		free(click_voice_context.click_voice_buf);
		printf("%s:create pthread error\n", __func__);
		return -1;
	}

	for(i = 0; i < CLICK_VOICE_FILE_NUM; i++) {
		click_voice_context.click_voice_fds[i] = -1;
	}

	click_voice_context.click_voice_init_flag = 1;

	return 0;
}

int audio_module_uninit(void)
{
	if(!click_voice_context.click_voice_init_flag) return 1;

	click_voice_context.click_voice_init_flag = 0;
	sem_post(&click_voice_context.click_voice_sem);
	pthread_join(click_voice_context.click_voice_tid, NULL);

	pthread_mutex_destroy(&click_voice_context.click_voice_lock);

	sem_destroy(&click_voice_context.click_voice_sem);
	sem_destroy(&sem_pcm_p);
	free(click_voice_context.click_voice_buf);
	memset(&click_voice_context, 0, sizeof(click_voice_context));

	return 0;
}


int play_sound(const char *file_path)
{
	int i;

	if(!click_voice_context.click_voice_init_flag) {
		return -1;
	}
	if(!file_path) {
		printf("%s:invalid file path\n", __func__);
		return -1;
	}

	for(i = 0; i < CLICK_VOICE_FILE_NUM; i++) {
		if(click_voice_context.click_voice_fds[i] == -1) {
			break;
		}
	}

	if(i >= CLICK_VOICE_FILE_NUM) {
		return -1;
	}

	pthread_mutex_lock(&click_voice_context.click_voice_lock);
	click_voice_context.click_voice_fds[i] =  open(file_path, O_RDONLY);
	if(click_voice_context.click_voice_fds[i] == -1) {
		pthread_mutex_unlock(&click_voice_context.click_voice_lock);
		printf("%s:open voice file error\n", __func__);
		return -1;
	}
	pthread_mutex_unlock(&click_voice_context.click_voice_lock);

	sem_post(&click_voice_context.click_voice_sem);

	return 0;
}

int init_pcm(unsigned int flags, struct pcm_config *cfg)
{
	struct pcm_config *config = NULL;
	int ret = 0;

	if(!cfg) {
		config = calloc(1, sizeof(struct pcm));
		if(!config) {
			fprintf(stderr, "fail to allocate config memory\n");
			return -1;
		}
		config->channels = 2;
		config->rate = 8000;
		config->period_size = 512;
		config->period_count = 3;
		config->format = PCM_FORMAT_S16_LE;
		config->start_threshold = 0;
		config->stop_threshold = 0;
		config->silence_threshold = 0;
		cfg = config;
	}

	if(flags & PCM_IN)
		ret = _pcm_open(PCM_IN, cfg);

	if(flags & PCM_OUT) {
		ret = _pcm_open(flags, cfg);
	//	printf("open out pcm ok\n");
	//	key_voice_init();
	}

//	printf("create pcm ok:%p %p\n", pcm_p, pcm_c);
	if(config) free(config);

	return ret;
}


int uninit_pcm(unsigned int flags)
{
	struct pcm *pcm = NULL;
	struct mixer *mixer;
	int ret = 0;

	if((flags & PCM_OUT) && (flags & PCM_IN)) {
		printf("You must close only one device!\n");
		return -1;
	}

	if(flags & PCM_OUT){
		if(!pcm_p) {
			printf("The play device has already close!\n");
			return -1;
		}
		pcm = pcm_p;
		pcm_p = NULL;
	//	key_voice_exit();
	}else if(flags & PCM_IN) {
		if(!pcm_c) {
			printf("The catpure device has already close!\n");
			return -1;
		}
		pcm = pcm_c;
		pcm_c = NULL;

	    mixer = mixer_open(0);
	    if (!mixer) {
	        fprintf(stderr, "Failed to open mixer\n");
	        return -1;
	    }

		tinymix_set_value(mixer, 35, "0");
		tinymix_set_value(mixer, 27, "0");
		mixer_close(mixer);
	}


	ret = pcm_close(pcm);
	if(!ret && (flags&PCM_OUT)) {
		if(click_voice_context.click_voice_init_flag) sem_post(&sem_pcm_p);
	}
//	printf("close pcm\n");
	return ret;
}

int pcm_write_api(void *data, unsigned int count)
{
#if 1
	int res =0;

	if(!pcm_p) {
		fprintf(stderr, "You must creat play pcm firstly\n");
		return -1;
	}
	if(!data) {
		fprintf(stderr, "Invalid data pointer\n");
		return -1;
	}

	do {
		res = pcm_write(pcm_p, data, count);

        if (res != 0)
        {
            printf("pcm write fail:%s res:%d ESTRPIPE:%d\n", strerror(errno),res,ESTRPIPE);
        }
		if (res == -EINTR)
        {
			/* nothing to do */
			res = 0;
		}
        else if (res == -EPIPE)
		{ /* suspend */
            printf("MSGTR_AO_ALSA_PcmInSuspendModeTryingResume\n");
			while ((res = pcm_resume(pcm_p)) == -EAGAIN)
				sleep(1);
		}
		if (res < 0)
        {
            printf("MSGTR_AO_ALSA_WriteError\n");
			if ((res = pcm_prepare(pcm_p)) < 0)
            {
                printf("MSGTR_AO_ALSA_PcmPrepareError\n");
				return -1;
				//break;
			}
		}
	} while (res != 0);



	return 0;

#else
	int ret = 0;
	struct play_queue *queue;

	if(!pcm_p) {
		fprintf(stderr, "You must creat play pcm firstly\n");
		return -1;
	}
	if(!data) {
		fprintf(stderr, "Invalid data pointer\n");
		return -1;
	}

	queue = make_a_queue(data, count);
	if(!queue) {
		return -1;
	}
	put_play_queue(queue);

	return ret;
#endif
}

int pcm_read_api(void *data, unsigned int count)
{
	if(!pcm_c) {
		fprintf(stderr, "You must creat capture pcm firstly\n");
		return -1;
	}
	return pcm_read(pcm_c, data, count);
}


int audio_set_play_volume(int volume)
{
	struct mixer *mixer;
	char buf[10];

	if(volume > 63) volume = 63;
	if(volume < 0) volume = 0;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	snprintf(buf, 10, "%d", volume);
	tinymix_set_value(mixer, 22, buf);
	mixer_close(mixer);

	return 0;
}

int audio_get_play_volume(int *value)
{
	struct mixer *mixer;

	if(!value) {
		printf("%s:invalid value pointer\n", __func__);
		return -1;
	}
	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 22, value);

	mixer_close(mixer);

	return 0;
}

int audio_set_rec_volume(int volume)
{
	struct mixer *mixer;
	char buf[10];

	if(volume > 7) volume = 7;
	if(volume < 0) volume = 0;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	snprintf(buf, 10, "%d", volume);
	tinymix_set_value(mixer, 36, buf);
	mixer_close(mixer);

	return 0;
}

int audio_get_rec_volume(int *value)
{
	struct mixer *mixer;

	if(!value) {
		printf("%s:invalid value pointer\n", __func__);
		return -1;
	}
	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 36, value);

	mixer_close(mixer);

	return 0;
}


int audio_set_rec_mute(int is_mute)
{
	struct mixer *mixer;
	int value = -1;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 27, &value);

	if(is_mute) {
		if(!value) {
			mixer_close(mixer);
			return 0;
		}
		tinymix_set_value(mixer, 27, "0"); /* mute */
	}
	else {
		if(value) {
			mixer_close(mixer);
			return 0;
		}
		tinymix_set_value(mixer, 27, "1"); /* unmute */
	}

	mixer_close(mixer);

	return 0;
}

int audio_get_rec_mute(void)
{
	struct mixer *mixer;
	int is_mute = -1;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 27, &is_mute);

	mixer_close(mixer);

	if(is_mute == 1) {
		return 0;
	}else if(is_mute == 0) {
		return 1;
	}

	return -1;
}

int audio_set_play_mute(int is_mute)
{
	struct mixer *mixer;
	int is_mute_r = -1, is_mute_l = -1;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 5, &is_mute_r);
	tinymix_get_value(mixer, 6, &is_mute_l);

	if(is_mute) {
		if(is_mute_r != 0) tinymix_set_value(mixer, 5, "0");
		if(is_mute_l != 0) tinymix_set_value(mixer, 6, "0");
	}
	else {
		if(is_mute_r != 1) tinymix_set_value(mixer, 5, "1");
		if(is_mute_l != 1) tinymix_set_value(mixer, 6, "1");
	}

	mixer_close(mixer);

	return 0;
}

int audio_get_play_mute(void)
{
	struct mixer *mixer;
	int is_mute_r = -1, is_mute_l = -1;

	mixer = mixer_open(0);
	if (!mixer) {
		fprintf(stderr, "Failed to open mixer\n");
		return -1;
	}

	tinymix_get_value(mixer, 5, &is_mute_r);
	tinymix_get_value(mixer, 6, &is_mute_l);

	mixer_close(mixer);

	if(is_mute_r == 0 && is_mute_l == 0) {
		return 1;
	}

	if(is_mute_r == 1 || is_mute_l == 1) {
		return 0;
	}

	return -1;
}
