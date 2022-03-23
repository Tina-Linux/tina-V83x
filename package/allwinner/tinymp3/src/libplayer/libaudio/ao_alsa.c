/*
 * libalsa output driver. This file is part of Shairport.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <alsa/asoundlib.h>

#include "audio.h"

#define DEVICE_NAME	"libaudio alsa"

#if 0
#define DL(x,y...)	{fprintf(stderr, "%s debug: "x"\n", DEVICE_NAME, ##y);}
#else
#define DL(x,y...)
#endif
#define EL(x,y...)	{fprintf(stderr, "%s: "x"\n", DEVICE_NAME, ##y);}

//static snd_pcm_t *alsa_handle = NULL;
//static snd_pcm_hw_params_t *alsa_params = NULL;

//static char *alsa_out_dev = "default";
static char *alsa_out_dev = "plug:dmix";

static void stop(ao_device_t *device);

static int bits2format(int bits)
{
	switch(bits) {
	case 8: return SND_PCM_FORMAT_S8;
	case 16: return SND_PCM_FORMAT_S16;
	}

	return -1;
}

static int init(ao_device_t *device)
{
	return 0;
}

static void deinit(ao_device_t *device)
{
	stop(device);
}

static int start(ao_device_t *device)
{
	ao_format_t *fmt = &(device->fmt);
	snd_pcm_t *alsa_handle = NULL;
	snd_pcm_hw_params_t *alsa_params = NULL;

	unsigned int buffer_time = 128000; /* 128ms, Unit: us */
	unsigned int period_time = 8000; /* 8ms, Unit: us */

	int ret, dir = 0;

	fmt->format = bits2format(fmt->bytes * 8);
	if (fmt->format < 0) {
		EL("NOT supported sample byte\n");
		return -1;
	}

	ret = snd_pcm_open(&alsa_handle, alsa_out_dev, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		EL("Alsa initialization failed: unable to open pcm device: %s\n", snd_strerror(ret));
		return -1;
	}

	snd_pcm_hw_params_alloca(&alsa_params);

	/* choose all parameters */
	ret = snd_pcm_hw_params_any(alsa_handle, alsa_params);
	if (ret < 0) {
		EL("snd_pcm_hw_params_any: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}

	/* open hardware resampling */
	snd_pcm_hw_params_set_rate_resample(alsa_handle, alsa_params, 1);
	/* set the interleaved read/write format */
	snd_pcm_hw_params_set_access(alsa_handle, alsa_params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/* set the sample format */
	snd_pcm_hw_params_set_format(alsa_handle, alsa_params, fmt->format);
	if (ret < 0) {
		EL("snd_pcm_hw_params_set_format: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}

	/* set the count of channels */
	snd_pcm_hw_params_set_channels(alsa_handle, alsa_params, fmt->channels);
	if (ret < 0) {
		EL("snd_pcm_hw_params_set_channels: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}

	/* set the stream rate */
	snd_pcm_hw_params_set_rate_near(alsa_handle, alsa_params, (unsigned int *)&fmt->rate, &dir);
	if (ret < 0) {
		EL("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}

	/* set the buffer time */
	snd_pcm_hw_params_set_buffer_time_near(alsa_handle, alsa_params, &buffer_time, &dir);
	if (ret < 0) {
		EL("snd_pcm_hw_params_set_buffer_time_near: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}

	/* set the period time */
	snd_pcm_hw_params_set_period_time_near(alsa_handle, alsa_params, &period_time, &dir);
	if (ret < 0) {
		EL("snd_pcm_hw_params_set_period_time_near: %s\n", snd_strerror(ret));
		goto err_pcm_hw_params;
	}


	ret = snd_pcm_hw_params(alsa_handle, alsa_params);
	if (ret < 0) {
		EL("unable to set hw parameters: %s\n", snd_strerror(ret));
		goto err_pcm_hw;
	}
	device->handler = (void*)alsa_handle;
	return 0;

err_pcm_hw:
err_pcm_hw_params:
	snd_pcm_close(alsa_handle);
	alsa_handle = NULL;

	return -1;
}

/**
 * xrun_recovery - Underrun and suspend recovery
 * @handle:
 * @err:
 */
static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {  /* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);  /* wait until the suspend flag is released */

		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}

	return err;
}

/**
 * play-
 * @buf:
 * @samples
 */
static void play(ao_device_t *device, short buf[], int samples)
{
	snd_pcm_t *alsa_handle = (snd_pcm_t *)(device->handler);

	signed short *ptr = buf;
	int len = samples;
	int err;

	//do about sw-gain

	while (len > 0) {
		err = snd_pcm_writei(alsa_handle, ptr, len);

		if (err == -EAGAIN || err == -EINTR)
			continue;

		if (err < 0) {
			/* this might be an error, or an exception */
			err = xrun_recovery(alsa_handle, err);
			if (err < 0) {
				EL("Failed to write to PCM device: %s\n", snd_strerror(err));
				return;
			} else
				continue;
		}

		/* decrement the sample counter */
		len -= err;
		/* adjust the start pointer */
		ptr += err * 2;
	}
}

static void stop(ao_device_t *device)
{
	snd_pcm_t *alsa_handle = (snd_pcm_t *)(device->handler);

	if (alsa_handle) {
		snd_pcm_drain(alsa_handle);
		snd_pcm_close(alsa_handle);
		device->handler = NULL;
	}
}

static void help(void)
{
	EL("There are no options for oss audio.\n");
}

static void volume(ao_device_t *device, int vol)
{

}

audio_output_t audio_alsa = {
	.name	= "alsa",
	.help	= &help,
	.init	= &init,
	.deinit	= &deinit,
	.start	= &start,
	.stop	= &stop,
	.play	= &play,
	.volume	= NULL,
	.dev_try = NULL,
};
