#include <alsa/asoundlib.h>
#include <alloca.h>
#include <stdio.h>
#include <signal.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1
struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

typedef struct pcm_recorder pcm_recorder_t;
struct pcm_recorder
{
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int sample_rate;
    snd_pcm_uframes_t frames;
    char *frame_buffer;
    int chanel;
};

char *recorder_module_name = "recorder.module.name";

static pcm_recorder_t* _new_pcm_recorder(int sample_rate, snd_pcm_uframes_t frames, int chanel);
static void _delete_pcm_recorder(pcm_recorder_t *pcm_recorder);
static int _process_pcm_recorder(pcm_recorder_t *pcm_recorder);
int capturing = 1;

void sigint_handler(int sig)
{
    capturing = 0;
}

static pcm_recorder_t* _new_pcm_recorder(int sample_rate, snd_pcm_uframes_t frames, int chanel)
{
    pcm_recorder_t *recorder=NULL;
    int rc,dir;
    recorder=(pcm_recorder_t *)malloc(sizeof(pcm_recorder_t));

    recorder->chanel=chanel;
    recorder->sample_rate=sample_rate;
    //rc = snd_pcm_open(&(recorder->handle), "default", SND_PCM_STREAM_CAPTURE, 0);
    rc = snd_pcm_open(&(recorder->handle), "plug:dsnoop", SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        goto end;
    }
    snd_pcm_hw_params_alloca(&(recorder->params));
    snd_pcm_hw_params_any(recorder->handle, recorder->params);
    snd_pcm_hw_params_set_access(recorder->handle, recorder->params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(recorder->handle, recorder->params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(recorder->handle, recorder->params, chanel);
#if 0
    snd_pcm_hw_params_set_rate_near(recorder->handle, recorder->params, (unsigned int *)&sample_rate, &dir);
#else
	snd_pcm_hw_params_set_rate(recorder->handle, recorder->params, sample_rate, 0);
#endif
    snd_pcm_hw_params_set_period_size_near(recorder->handle, recorder->params, &frames, &dir);
    rc = snd_pcm_hw_params(recorder->handle, recorder->params);/* Write the parameters to the driver */
    if (rc < 0) {
        fprintf(stderr,
                "unable to set hw parameters: %s\n",
                snd_strerror(rc));
        goto end;
    }
    snd_pcm_hw_params_get_period_size(recorder->params, &frames, &dir);
    recorder->frames=frames;
    recorder->frame_buffer = (char *) malloc(sizeof(char)*(frames*chanel*sizeof(short)));/* 2 bytes/sample */
    printf("buf len: %ld\n", sizeof(char)*(frames*chanel*sizeof(short)));
end:
    return recorder;
}

static void _delete_pcm_recorder(pcm_recorder_t *pcm_recorder)
{
    snd_pcm_drain(pcm_recorder->handle);
    snd_pcm_close(pcm_recorder->handle);
    free(pcm_recorder->frame_buffer);
    free(pcm_recorder);
}

static int _process_pcm_recorder(pcm_recorder_t *pcm_recorder)
{
    int ret=-1;
    ret=snd_pcm_readi(pcm_recorder->handle, pcm_recorder->frame_buffer, pcm_recorder->frames);
    if (ret == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(pcm_recorder->handle);
    } else if (ret < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(ret));
    } else if (ret != (int)(pcm_recorder->frames)) {
      fprintf(stderr, "short read, read %d frames\n", ret);
    }
    return ret;
}


int main()
{
    int ret=-1;
    pcm_recorder_t *recorder=NULL;
    char buf[4096 * 10] = { 0 };
    FILE *fp = NULL;
    int sample_rate = 16000;
	unsigned int bits = 16;
	unsigned int channels = 2;
	unsigned int frames_read = 0;
	struct wav_header capture_header;
    fp = fopen("rec.wav", "wb");
	/* install signal handler and begin capturing */
    signal(SIGINT, sigint_handler);

	capture_header.riff_id = ID_RIFF;
    capture_header.riff_sz = 0;
    capture_header.riff_fmt = ID_WAVE;
    capture_header.fmt_id = ID_FMT;
    capture_header.fmt_sz = 16;
    capture_header.audio_format = FORMAT_PCM;
    capture_header.num_channels = 2;
    capture_header.sample_rate = 16000;
    capture_header.bits_per_sample = bits;
    capture_header.byte_rate = (capture_header.bits_per_sample / 8) * channels * sample_rate;
    capture_header.block_align = channels * (capture_header.bits_per_sample / 8);
    capture_header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(fp, sizeof(struct wav_header), SEEK_SET);

    recorder=_new_pcm_recorder(16000, 100, 2);

    while(1) {
        ret=_process_pcm_recorder(recorder);

        memset(buf, 0, 4096 * 10);
        memcpy(buf,recorder->frame_buffer,(recorder->chanel)*(recorder->frames)*sizeof(short));
        if (fp) {
            fwrite(buf, 1, (recorder->chanel)*(recorder->frames)*sizeof(short), fp);
        }
        frames_read += (recorder->chanel)*(recorder->frames)*sizeof(short)/4;
        if (capturing == 0)
			break;
    }

/* write header now all information is known */
    capture_header.data_sz = frames_read * capture_header.block_align;

    _delete_pcm_recorder(recorder);
    capturing = 0;
    fseek(fp, 0, SEEK_SET);
    fwrite(&capture_header, sizeof(struct wav_header), 1, fp);
    if (fp) {
        fclose(fp);
    }

    ret=0;
    return ret;
}
