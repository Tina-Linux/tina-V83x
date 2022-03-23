/*************************************************
* Codec Test
**************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <alsa/asoundlib.h>

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

#define BUF_LEN      (4096)
#define REPLAY_TIME  (1)

char buf[BUF_LEN];
static char filename[64] = "test.wav";
int capturing = 1;

void sigint_handler(int sig)
{
    capturing = 0;
}

int main(void)
{
	int err;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	int dtime = 1;
	int r;
	FILE *fp = NULL;
	int loop;
	int sample_rate = 16000;
	unsigned int bits = 16;
	unsigned int channels = 2;
	unsigned int frames_read = 0;
	struct wav_header capture_header;

	fprintf(stderr, "sample_rate is %d\n", sample_rate);

	if ((err = snd_pcm_open(&capture_handle,  "plug:dsnoop", SND_PCM_STREAM_CAPTURE, 0)) < 0)
	//if((err = snd_pcm_open(&capture_handle,  "default", SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		fprintf(stderr, "cannot open audio device (%s)\n",  snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, sample_rate, 0)) < 0)
	{
		fprintf(stderr , "cannot set sample rate (%s)\n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 2)) <0)
	{
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_free(hw_params);

	fprintf(stderr, "open file : %s\n",filename);
	dtime = 60;
	/* install signal handler and begin capturing */
    signal(SIGINT, sigint_handler);

	fp = fopen(filename, "wb+");
	if (fp == NULL) {
		fprintf(stderr, "open test pcm file err\n");
		return -1;
	}
	capture_header.riff_id = ID_RIFF;
    capture_header.riff_sz = 0;
    capture_header.riff_fmt = ID_WAVE;
    capture_header.fmt_id = ID_FMT;
    capture_header.fmt_sz = 16;
    capture_header.audio_format = FORMAT_PCM;
    capture_header.num_channels = 2;
    capture_header.sample_rate = sample_rate;
    capture_header.bits_per_sample = bits;
    capture_header.byte_rate = (capture_header.bits_per_sample / 8) * channels * sample_rate;
    capture_header.block_align = channels * (capture_header.bits_per_sample / 8);
    capture_header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(fp, sizeof(struct wav_header), SEEK_SET);


	loop = dtime*sample_rate/1024;

    do {
	r = snd_pcm_readi( capture_handle, buf , 1024);
	if (r>0) {
		err = fwrite(buf, 1, r*4, fp);
			if (err < 0) {
				fprintf(stderr, "write to audio interface failed (%s)\n",snd_strerror(err));
				return err;
			}
	}
	frames_read += r;
		loop--;
		if (capturing == 0)
			break;
    } while(loop != 0);
    /* write header now all information is known */
    capture_header.data_sz = frames_read * capture_header.block_align;
    fseek(fp, 0, SEEK_SET);
    fwrite(&capture_header, sizeof(struct wav_header), 1, fp);
	fprintf(stderr, "close file\n");
	fclose(fp);
	capturing = 0;
	fprintf(stderr, "close dev\n");
	snd_pcm_close(capture_handle);
	fprintf(stderr, "ok\n");

	return 0;
}
