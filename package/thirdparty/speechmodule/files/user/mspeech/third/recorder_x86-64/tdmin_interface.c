#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "tdmin_interface.h"
#include "debug.h"

#define ALSA_OK             0
#define ALSA_FAIL          -1

#define PB_ALSA_ERR(x)  printf x
#define PB_ALSA_INFO(x) printf x

#define PCM_HANDLE_CHECK(handle) do{  \
									if(NULL == handle) \
									{  PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"%s -- Invail pcm handle fail\n",__FUNCTION__)); \
										return ALSA_FAIL;}  \
									}while(0)

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_ReadPCM
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -ok:write pcm size  fail or pause:-1
 *---------------------------------------------------------------------------*/

static int _alsa_read_pcm(PCMContainer_t *sndpcm, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;
	uint8_t *data = sndpcm->data_buf;

	if (count != sndpcm->chunk_size) {
		count = sndpcm->chunk_size;
	}

	while (count > 0) {
		r = snd_pcm_readi(sndpcm->handle, data, count);

		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(sndpcm->handle, 1000);
		} else if (r == -EPIPE) {
			snd_pcm_prepare(sndpcm->handle);
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n"));
		} else if (r == -ESTRPIPE) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n"));
		} else if (r < 0) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_readi: [%s]\n", snd_strerror(r)));
			return -1;
		}

		if (r > 0) {
			result += r;
			count -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return rcount;
}

//add by 926 @20190605
/**
 * \brief show configs
 * \param pcm_params
 * \param func/line
 * \param enable:0,disable ; 1,enable show
 * \return null
 */

void show_alsa_configs(PCMContainer_t *pcm_params, char *func, uint32_t line, uint8_t enable)
{
	if (enable == 0x01) {
		printf("/--------------------------------/\n");
		printf(" * func:(%s, %d)\n", func, line);
		//printf(" * handle = %#x\n", pcm_params->handle);
		printf(" * chunk_size = %d\n", pcm_params->chunk_size);
		printf(" * buffer_size = %d\n", pcm_params->buffer_size);
		printf(" * format = %d (S16_LE:2)\n", pcm_params->format);
		printf(" * channels = %d\n", pcm_params->channels);
		printf(" * chunk_bytes = %d\n", pcm_params->chunk_bytes);
		printf(" * bits_per_sample = %d\n", pcm_params->bits_per_sample);
		printf(" * bits_per_frame = %d\n", pcm_params->bits_per_frame);
		printf(" * sample_rate = %d\n", pcm_params->sample_rate);
		//printf(" * data_buf = %#x\n", pcm_params->data_buf);
		printf("/--------------------------------/\n");
	}
	return;
}
//end

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_SetHWParams
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          - OK(0)  PARAMS ERR(-1)
 *---------------------------------------------------------------------------*/

static int _alsa_set_hw_params(PCMContainer_t *pcm_params)
{
	snd_pcm_hw_params_t *hwparams;
	uint32_t exact_rate;
	uint32_t buffer_time, period_time;
	int	 err;

	PCM_HANDLE_CHECK(pcm_params->handle);

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* Fill it with default values */
	err = snd_pcm_hw_params_any(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_any : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

    /* Interleaved mode */
	err = snd_pcm_hw_params_set_access(pcm_params->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_access : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample format */
	err = snd_pcm_hw_params_set_format(pcm_params->handle, hwparams,pcm_params->format);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_format : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm_params->handle, hwparams, LE_SHORT(pcm_params->channels));
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_channels : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	exact_rate = LE_INT(pcm_params->sample_rate);
	err = snd_pcm_hw_params_set_rate_near(pcm_params->handle, hwparams, &exact_rate, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_rate_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}
	if (LE_INT(pcm_params->sample_rate) != exact_rate) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n",
			LE_INT(pcm_params->sample_rate), exact_rate));
	}

#if 0/*Rk3308 doesn't support follow configs, xjh @180905*/
	err = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_get_buffer_time_max : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}
	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_hw_params_get_buffer_time_max : %ul (us)\n",buffer_time));

	if (buffer_time > 200000)
		buffer_time = 200000;/*200000us = 200ms*/

	if (buffer_time > 0)
		period_time = buffer_time / 4;

	err = snd_pcm_hw_params_set_buffer_time_near(pcm_params->handle, hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_buffer_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	err = snd_pcm_hw_params_set_period_time_near(pcm_params->handle, hwparams, &period_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_period_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}
#else
    uint32_t period_frames = 1024;
    err = snd_pcm_hw_params_set_period_size_near(pcm_params->handle, hwparams, &period_frames, 0);
    if (err < 0) {
            PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_period_size_near: %s at line->%d\n",snd_strerror(err),__LINE__));
            goto ERR_SET_PARAMS;
    }
#endif
	/* Set hw params */
	err = snd_pcm_hw_params(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params: %s at line->%d\n",snd_strerror(err),__LINE__));
		goto ERR_SET_PARAMS;
	}

	snd_pcm_hw_params_get_period_size(hwparams, &pcm_params->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(hwparams, &pcm_params->buffer_size);
	if (pcm_params->chunk_size == pcm_params->buffer_size) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Can't use period equal to buffer size (%lu == %lu)\n", pcm_params->chunk_size, pcm_params->buffer_size));
		goto ERR_SET_PARAMS;
	}

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"chunk_size is %lu, buffer size is %lu\n", pcm_params->chunk_size, pcm_params->buffer_size));

	/*bits per sample = bits depth*/
	pcm_params->bits_per_sample = snd_pcm_format_physical_width(pcm_params->format);

	/*bits per frame = bits depth * channels*/
	pcm_params->bits_per_frame = pcm_params->bits_per_sample * LE_SHORT(pcm_params->channels);

	/*chunk byte is a better size for each write or read for alsa*/
	pcm_params->chunk_bytes = pcm_params->chunk_size * pcm_params->bits_per_frame / 8;
	pcm_params->chunk_bytes_16 = pcm_params->chunk_bytes/2;

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"chunk_bytes is %lu\n", pcm_params->chunk_bytes));

	/* Allocate audio data buffer */
	pcm_params->data_buf = (uint8_t *)malloc(pcm_params->chunk_bytes);
	if (!pcm_params->data_buf) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error malloc: [data_buf] at line-> %d\n",__LINE__));
		goto ERR_SET_PARAMS;
	}
	pcm_params->data_buf_16 = (uint8_t *)malloc(pcm_params->chunk_bytes_16);
	if (!pcm_params->data_buf_16) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error malloc: [data_buf] at line-> %d\n",__LINE__));
		goto ERR_SET_PARAMS;
	}

	return 0;

ERR_SET_PARAMS:
	if(NULL != pcm_params->data_buf) {
		free(pcm_params->data_buf);
		pcm_params->data_buf = NULL;
	}
	if(NULL != pcm_params->data_buf_16) {
		free(pcm_params->data_buf_16);
		pcm_params->data_buf_16 = NULL;
	}
	snd_pcm_close(pcm_params->handle);
	pcm_params->handle = NULL;
	return -1;
}

int _pcm_32_to_16(char *dst_buf, char *ori_buf, int ori_len, int fseek_bit)
{
    int i, j, k;

    while (ori_len >= 32) {
        for (i = j = k = 0; i < 8; i++) {
            dst_buf[0 + j] = ori_buf[2 + k];
            dst_buf[1 + j] = ori_buf[3 + k];
            j += 2;
            k += 4;
        }
        dst_buf[0 + j - 4] = dst_buf[0 + j - 2];
        dst_buf[1 + j - 4] = dst_buf[1 + j - 2];

        dst_buf += 16;
        ori_buf += 32;

        ori_len -= 32;
    }
}


int alsa_tdmin_init(PCMContainer_t *pcm_params)
{

    int i4_ret = 0;

	if(pcm_params == NULL)
	{
	//	debug_f(LOG_DEBUG, "PCMContainer handle == NULL!\n");
		return TDMIN_INV_ARG;
	}

	//pcm_params->format = SND_PCM_FORMAT_S32_LE;
	pcm_params->format = SND_PCM_FORMAT_S16_LE;
	pcm_params->sample_rate = AISPEECH_TDMIC_SAMPLERATE;
	pcm_params->channels = AISPEECH_TDMIC_CHANNLE;

	i4_ret = snd_pcm_open(&pcm_params->handle, AISPEECH_TDMIC_PCM_RECORD_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
	if (0 != i4_ret)
	{
		printf("snd_pcm_open failed!\n");
		return TDMIN_FAIL;
	}
	i4_ret = _alsa_set_hw_params(pcm_params);
	if(i4_ret != 0)
	{
	    printf("u_alsa_set_hw_params failed!\n");
		return TDMIN_FAIL;
	}
	printf("Set alsa param OK, start read pcm!!!\n");

	return TDMIN_OK;
}

void alsa_tdmin_uninit(PCMContainer_t *pcm_params)
{
    if(pcm_params == NULL)
	{
		printf("PCMContainer handle == NULL!\n");
		return TDMIN_INV_ARG;
	}
	if(NULL != pcm_params->data_buf) {
			free(pcm_params->data_buf);
			pcm_params->data_buf = NULL;
		}
	if(NULL != pcm_params->data_buf_16) {
			free(pcm_params->data_buf_16);
			pcm_params->data_buf_16 = NULL;
		}
	snd_pcm_close(pcm_params->handle);
	pcm_params->handle = NULL;
}

int alsa_read_tdmin_pcm(PCMContainer_t *pcm_params)
{
    int i4_ret = 0;

	if(pcm_params == NULL)
	{
		printf("PCMContainer handle == NULL!\n");
		return TDMIN_INV_ARG;
	}

    i4_ret = _alsa_read_pcm(pcm_params,pcm_params->chunk_bytes);

	if(i4_ret == pcm_params->chunk_bytes)
	{
	    //i4_ret = _pcm_32_to_16(pcm_params->data_buf_16,pcm_params->data_buf,pcm_params->chunk_bytes,PCM_SEEK_BIT);

	    return TDMIN_OK;
	}
	else
	{
	    printf("TDMIN pcm read fail\n");
		return i4_ret;
	}

}
