/*****************************************************************************
 **
 **  Name:           app_avk.c
 **
 **  Description:    Bluetooth Manager application
 **
 **  Copyright (c) 2009-2015, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "buildcfg.h"

#include "bsa_api.h"
#include "bsa_avk_api.h"

#include "gki.h"
#include "uipc.h"

#include "app_xml_utils.h"
#include "app_disc.h"
#include "app_utils.h"
#include "app_wav.h"

#ifdef PCM_ALSA
#include "alsa/asoundlib.h"
//#define APP_AVK_ASLA_DEV "hw:1,0"
#include "aumixcom.h"
#include "doResample.h"
#define APP_AVK_ASLA_DEV "default"
#endif

#include "app_avk.h"
/*
 * Defines
 */
#define BT_RESAMPLE 1

#define APP_XML_REM_DEVICES_FILE_PATH "./bt_devices.xml"

#define APP_AVK_SOUND_FILE "test_avk"

//#define USE_MEMQ_PCM
#ifndef BSA_AVK_SECURITY
#define BSA_AVK_SECURITY    BSA_SEC_AUTHORIZATION
#endif
#ifndef BSA_AVK_FEATURES
#define BSA_AVK_FEATURES    (BSA_AVK_FEAT_RCCT|BSA_AVK_FEAT_RCTG|\
                             BSA_AVK_FEAT_VENDOR|BSA_AVK_FEAT_METADATA|\
                             BSA_AVK_FEAT_BROWSE|BSA_AVK_FEAT_DELAY_RPT)
#endif

#ifndef BSA_AVK_FEATURES_NO_AVRCP
#define BSA_AVK_FEATURES_NO_AVRCP    (BSA_AVK_FEAT_VENDOR|BSA_AVK_FEAT_METADATA | BSA_AVK_FEAT_BROWSE)
#endif

#ifndef BSA_AVK_DUMP_RX_DATA
#define BSA_AVK_DUMP_RX_DATA FALSE
#endif

#ifndef BSA_AVK_AAC_SUPPORTED
#define BSA_AVK_AAC_SUPPORTED FALSE
#endif

/* bitmask of events that BSA client is interested in registering for notifications */
tBSA_AVK_REG_NOTIFICATIONS reg_notifications =
        (1 << (BSA_AVK_RC_EVT_PLAY_STATUS_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_TRACK_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_TRACK_REACHED_END - 1)) |
        (1 << (BSA_AVK_RC_EVT_TRACK_REACHED_START - 1)) |
        (1 << (BSA_AVK_RC_EVT_PLAY_POS_CHANGED - 1)) |
        (1 << (BSA_AVK_RC_EVT_BATTERY_STATUS_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_SYSTEM_STATUS_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_APP_SETTING_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_NOW_PLAYING_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_AVAL_PLAYERS_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_ADDR_PLAYER_CHANGE - 1)) |
        (1 << (BSA_AVK_RC_EVT_UIDS_CHANGE - 1));


/*
 * global variable
 */

tAPP_AVK_CB app_avk_cb;
extern int connect_link_status;

#ifdef PCM_ALSA
static char *alsa_device = "default";     /* ALSA playback device */
//static char *alsa_device = "plug:dmix"; /* ALSA playback device */

static pcm_params pcm_alsa_params;
static int pcm_alsa_close_cmd = 0;
static int pcm_thread_running = 0;
#endif

#ifdef BT_RESAMPLE
#include <stdint.h>
/* PCM sub formats */
typedef enum {
    AUDIO_FORMAT_PCM_SUB_16_BIT          = 0x1, /* DO NOT CHANGE - PCM signed 16 bits */
    AUDIO_FORMAT_PCM_SUB_8_BIT           = 0x2, /* DO NOT CHANGE - PCM unsigned 8 bits */
    AUDIO_FORMAT_PCM_SUB_32_BIT          = 0x3, /* PCM signed .31 fixed point */
    AUDIO_FORMAT_PCM_SUB_8_24_BIT        = 0x4, /* PCM signed 7.24 fixed point */
} audio_format_pcm_sub_fmt_t;

typedef enum {
    AUDIO_FORMAT_INVALID             = 0xFFFFFFFFUL,
    AUDIO_FORMAT_DEFAULT             = 0,
    AUDIO_FORMAT_PCM                 = 0x00000000UL, /* DO NOT CHANGE */

    /* Aliases */
    AUDIO_FORMAT_PCM_16_BIT          = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_16_BIT),
    AUDIO_FORMAT_PCM_8_BIT           = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_8_BIT),
    AUDIO_FORMAT_PCM_32_BIT          = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_32_BIT),
    AUDIO_FORMAT_PCM_8_24_BIT        = (AUDIO_FORMAT_PCM |
                                        AUDIO_FORMAT_PCM_SUB_8_24_BIT),
} audio_format_t;

struct sunxi_stream_out {
	Resampler *resampler;
	ResCfg cfg;
	//struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
};
static struct sunxi_stream_out out ;

#define SAMPLING_RATE_8K	8000
#define SAMPLING_RATE_11K	11025
#define SAMPLING_RATE_16K	16000
#define SAMPLING_RATE_44K	44100
#define SAMPLING_RATE_48K	48000
#define DEFAULT_OUT_SAMPLING_RATE SAMPLING_RATE_48K

#define SHORT_PERIOD_SIZE 2048
#define RESAMPLER_BUFFER_FRAMES (SHORT_PERIOD_SIZE * 2)
#define RESAMPLER_BUFFER_SIZE (4 * RESAMPLER_BUFFER_FRAMES)

int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer);
void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer);
#endif

/*
 * Local functions
 */
static void app_avk_close_wave_file(tAPP_AVK_CONNECTION *connection);
static void app_avk_create_wave_file(void);
static void app_avk_uipc_cback(BT_HDR *p_msg);

static tAvkCallback *s_pCallback = NULL;
static tAPP_AVK_CONNECTION* pStreamingConn = NULL;

static pthread_mutex_t   alsa_opt_mutex;
static UINT8 a_data[1024*4];
static int   next = 0;
/*******************************************************************************
 **
 ** Function         app_avk_get_label
 **
 ** Description      get transaction label (used to distinguish transactions)
 **
 ** Returns          UINT8
 **
 *******************************************************************************/
UINT8 app_avk_get_label()
{
    if(app_avk_cb.label >= 15)
        app_avk_cb.label = 0;
    return app_avk_cb.label++;
}

/*******************************************************************************
 **
 ** Function         app_avk_create_wave_file
 **
 ** Description     create a new wave file
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_create_wave_file(void)
{
    int file_index = 0;
    int fd;
    char filename[200];

#ifdef PCM_ALSA
    return;
#endif

    /* If a wav file is currently open => close it */
    if (app_avk_cb.fd != -1)
    {
        tAPP_AVK_CONNECTION dummy;
        memset (&dummy, 0, sizeof(tAPP_AVK_CONNECTION));
        app_avk_close_wave_file(&dummy);
    }

    do
    {
        snprintf(filename, sizeof(filename), "%s-%d.wav", APP_AVK_SOUND_FILE, file_index);
        filename[sizeof(filename)-1] = '\0';
        fd = app_wav_create_file(filename, O_EXCL);
        file_index++;
    } while (fd < 0);

    app_avk_cb.fd = fd;
}
/*******************************************************************************
 **
 ** Function         app_avk_create_aac_file
 **
 ** Description     create a new wave file
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_create_aac_file(void)
{
    int file_index = 0;
    int fd;
    char filename[200];

    /* If a wav file is currently open => close it */
    if (app_avk_cb.fd != -1)
    {
//        close(app_avk_cb.fd);
//        app_avk_cb.fd = -1;
        return;
    }

    do
    {
        int flags = O_EXCL | O_RDWR | O_CREAT | O_TRUNC;

        snprintf(filename, sizeof(filename), "%s-%d.aac", APP_AVK_SOUND_FILE, file_index);
        filename[sizeof(filename)-1] = '\0';

        /* Create file in read/write mode, reset the length field */
        fd = open(filename, flags, 0666);
        if (fd < 0)
        {
                APP_ERROR1("open(%s) failed: %d", filename, errno);
            }

        file_index++;
    } while ((fd < 0) && (file_index<100));

    app_avk_cb.fd_codec_type = BSA_AVK_CODEC_M24;
    app_avk_cb.fd = fd;
}

/*******************************************************************************
 **
 ** Function         app_avk_close_wave_file
 **
 ** Description     proper header and close the wave file
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_close_wave_file(tAPP_AVK_CONNECTION *connection)
{
    tAPP_WAV_FILE_FORMAT format;

#ifdef PCM_ALSA
    return;
#endif

    if (app_avk_cb.fd == -1)
    {
        printf("app_avk_close_wave_file: no file to close\n");
        return;
    }

    format.bits_per_sample = connection->bit_per_sample;
    format.nb_channels = connection->num_channel;
    format.sample_rate = connection->sample_rate;

    app_wav_close_file(app_avk_cb.fd, &format);

    app_avk_cb.fd = -1;
}

/*******************************************************************************
**
** Function         app_avk_end
**
** Description      This function is used to close AV
**
** Returns          void
**
*******************************************************************************/
void app_avk_end(void)
{
    tBSA_AVK_DISABLE disable_param;
    tBSA_AVK_DEREGISTER deregister_param;

    /* deregister avk */
    BSA_AvkDeregisterInit(&deregister_param);
    BSA_AvkDeregister(&deregister_param);

    /* disable avk */
    BSA_AvkDisableInit(&disable_param);
    BSA_AvkDisable(&disable_param);

    pthread_mutex_destroy(&alsa_opt_mutex);
}
#ifdef BT_RESAMPLE
static inline size_t audio_bytes_per_sample(audio_format_t format)
{
    size_t size = 0;

    switch (format) {
    case AUDIO_FORMAT_PCM_32_BIT:
    case AUDIO_FORMAT_PCM_8_24_BIT:
        size = sizeof(int32_t);
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
        size = sizeof(int16_t);
        break;
    case AUDIO_FORMAT_PCM_8_BIT:
        size = sizeof(uint8_t);
        break;
    default:
        break;
    }
    return size;
}

static inline size_t audio_stream_frame_size()
{
    size_t chan_samp_sz;
    audio_format_t format = AUDIO_FORMAT_PCM_16_BIT;

    chan_samp_sz = audio_bytes_per_sample(format);
    return 2 * chan_samp_sz;
}
#endif

#if 0
static int alsa_set_pcm_params(snd_pcm_t *playback_handle,
                           snd_pcm_format_t format,
                           unsigned int channels,
                           unsigned int sample_rate)
{
    int err;
    int chunk_size, buffer_size;
	  snd_pcm_hw_params_t *hw_params;

	  if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
	  fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		    return -1;
    }

    if((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	if((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		return -1;
	}

	if((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, format)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
		return -1;
	}

	if((err = snd_pcm_hw_params_set_rate(playback_handle, hw_params, sample_rate, 0)) < 0)
	{
		fprintf(stderr , "cannot set sample rate (%s)\n", snd_strerror(err));
		return -1;
	}

	if((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels)) <0)
	{
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_get_period_size(hw_params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
	printf("chunk_size = %d, buffer_size = %d\n", chunk_size, buffer_size);

	if((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0)
	{
		fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_free(hw_params);
	return 0;
}
#endif

static void avk_pcm_set_output(void)
{
  char cmd[128] = {0};
  int alsa_vol = 0;

  alsa_vol = (63 * app_avk_cb.volume)/127;
  snprintf(cmd, sizeof(cmd), "amixer cset name='headphone volume' '%d'", alsa_vol);
	system(cmd);

	system("amixer cset name='DACL Mixer AIF1DA0L Switch' 1");
	system("amixer cset name='DACR Mixer AIF1DA0R Switch' 1");
	system("amixer cset name='Headphone Switch' 1");
}

//close when hs
static void avk_pcm_close_output(void)
{
	system("amixer cset name='DACL Mixer AIF1DA0L Switch' 0");
	system("amixer cset name='DACR Mixer AIF1DA0R Switch' 0");
}

//close when music
static void avk_close_hs_output(void)
{
	system("amixer cset name='DACR Mixer AIF2DACR Switch' 0");
	system("amixer cset name='DACL Mixer AIF2DACL Switch' 0");
	//system("amixer cset name='AIF2INL Mux VIR switch aif2inl aif3' 0");
}

static void avk_close_hs_input(void)
{
	system("amixer cset name='AIF3OUT Mux' 0");
	system("amixer cset name='AIF2 ADR Mixer ADCR Switch' 0");
	system("amixer cset name='AIF2 ADL Mixer ADCL Switch' 0");
	system("amixer cset name='LEFT ADC input Mixer MIC1 boost Switch' 0");
	system("amixer cset name='RIGHT ADC input Mixer MIC1 boost Switch' 0");

}
/*******************************************************************************
**
** Function         app_avk_handle_start
**
** Description      This function is the AVK start handler function
**
** Returns          void
**
*******************************************************************************/
static void app_avk_handle_start(tBSA_AVK_MSG *p_data, tAPP_AVK_CONNECTION *connection)
{
	struct sched_param sched;
    char *avk_format_display[12] = {"SBC", "M12", "M24", "??", "ATRAC", "PCM", "APT-X", "SEC"};
#ifdef PCM_ALSA
#ifdef BT_RESAMPLE
	int ret = 0;
#endif
    int status;
    snd_pcm_format_t format;
#endif

    if (p_data->start.media_receiving.format < (sizeof(avk_format_display)/sizeof(avk_format_display[0])))
    {
        APP_INFO1("AVK start: format %s", avk_format_display[p_data->start.media_receiving.format]);
    }
    else
    {
        APP_INFO1("AVK start: format code (%u) unknown", p_data->start.media_receiving.format);
    }

    connection->format = p_data->start.media_receiving.format;

    if (connection->format == BSA_AVK_CODEC_PCM)
    {
        app_avk_create_wave_file();/* create and open a wave file */
        connection->sample_rate = p_data->start.media_receiving.cfg.pcm.sampling_freq;
        connection->num_channel = p_data->start.media_receiving.cfg.pcm.num_channel;
        connection->bit_per_sample = p_data->start.media_receiving.cfg.pcm.bit_per_sample;
        printf("Sampling rate:%d, number of channel:%d bit per sample:%d\n",
            p_data->start.media_receiving.cfg.pcm.sampling_freq,
            p_data->start.media_receiving.cfg.pcm.num_channel,
            p_data->start.media_receiving.cfg.pcm.bit_per_sample);
#ifdef PCM_ALSA

        pthread_mutex_lock(&alsa_opt_mutex);

        /* If ALSA PCM driver was already open => close it */
        if (app_avk_cb.alsa_handle != NULL)
        {

			if (connection->bit_per_sample == 8)
                format = SND_PCM_FORMAT_U8;
            else
                format = SND_PCM_FORMAT_S16_LE;
			if ((pcm_alsa_params.format == format)
				&& (pcm_alsa_params.num_channel == connection->num_channel)
				&& (pcm_alsa_params.sample_rate == connection->sample_rate)) {
				pthread_mutex_unlock(&alsa_opt_mutex);
				return;
			} else {
				printf("Already open: snd_pcm_close\n");
				snd_pcm_close(app_avk_cb.alsa_handle);
				app_avk_cb.alsa_handle = NULL;
			}
			#ifdef USE_MEMQ_PCM
			if(!pcm_thread_running)
		            app_avk_write_buffer();
			#endif
		}

		avk_close_hs_output();
    app_hs_close_alsa_duplex();

		/* set output */
		avk_pcm_set_output();
#if 0
        /* Open ALSA driver */
	sched.sched_priority = 99;

	if(sched_setscheduler(getpid(), SCHED_FIFO, &sched) < 0)
		fprintf(stderr, "SETSCHEDULER failed -err = %s \n", strerror(errno));
	else
		printf("Priority set to \"%d\"\n", sched.sched_priority);
#endif
        status = snd_pcm_open(&(app_avk_cb.alsa_handle), alsa_device,
            SND_PCM_STREAM_PLAYBACK, 0);
        if (status < 0)
        {
            APP_ERROR1("snd_pcm_open failed: %s", snd_strerror(status));

        }
        else
        {
            APP_DEBUG0("ALSA driver opened");
            #ifdef BT_RESAMPLE
			if (DEFAULT_OUT_SAMPLING_RATE != connection->sample_rate) {
				if (out.resampler) {
					Destroy_Resampler(out.resampler);
					out.resampler = NULL;
				}

        if (out.buffer != NULL) {
          free(out.buffer);
          out.buffer = NULL;
        }

				out.buffer = malloc(RESAMPLER_BUFFER_SIZE*4); /* todo: allow for reallocing */
				printf("%s,l:%d, out.buffer:%p, resample_out_buf_bytes:%d\n", __func__, __LINE__, out.buffer, RESAMPLER_BUFFER_SIZE*4);
				/*params:inSampleRate, outSampleRate, channel, quality, buf_provider/NULL, resample*/
				out.resampler = Creat_Resampler();
				if (ret != 0) {
					printf("create out resampler failed, %d -> %d", connection->sample_rate, DEFAULT_OUT_SAMPLING_RATE);
                    pthread_mutex_unlock(&alsa_opt_mutex);
					return ;
				}
				out.cfg.inch = 2;
				out.cfg.insrt = connection->sample_rate;
				out.cfg.outsrt = DEFAULT_OUT_SAMPLING_RATE;
				printf("create out resampler OK, %d -> %d", connection->sample_rate, DEFAULT_OUT_SAMPLING_RATE);
			} else {
				printf("do not use out resampler");
			}
			#endif
            if (connection->bit_per_sample == 8)
                format = SND_PCM_FORMAT_U8;
            else
                format = SND_PCM_FORMAT_S16_LE;
            /* Configure ALSA driver with PCM parameters */
            #ifdef BT_RESAMPLE
            status = snd_pcm_set_params(app_avk_cb.alsa_handle, format,
                SND_PCM_ACCESS_RW_INTERLEAVED, connection->num_channel,
                DEFAULT_OUT_SAMPLING_RATE, 1, 400000);/* 0.4sec */
            #else
			status = snd_pcm_set_params(app_avk_cb.alsa_handle, format,
                SND_PCM_ACCESS_RW_INTERLEAVED, connection->num_channel,
                connection->sample_rate, 1, 400000);/* 0.4sec */
            #endif
            if (status < 0)
            {
                APP_ERROR1("snd_pcm_set_params failed: %s", snd_strerror(status));
				snd_pcm_close(app_avk_cb.alsa_handle);
				app_avk_cb.alsa_handle = NULL;
                pthread_mutex_unlock(&alsa_opt_mutex);
                return ;
            }

            /* store pcm params */
            pcm_alsa_params.format = format;
            pcm_alsa_params.num_channel = connection->num_channel;
            pcm_alsa_params.sample_rate = connection->sample_rate;
		#ifdef USE_MEMQ_PCM
            app_avk_write_buffer();
		#endif
            /* set output */
            //mixer_set("Speaker Function",2);
        }

        pthread_mutex_unlock(&alsa_opt_mutex);
#endif
    }
    else if (connection->format == BSA_AVK_CODEC_M24)
    {
        /* create and open an aac file to dump data to */
        app_avk_create_aac_file();

        /* Initialize the decoder with the format information here */
    }

}

/*******************************************************************************
 **
 ** Function         app_avk_cback
 **
 ** Description      This function is the AVK callback function
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_cback(tBSA_AVK_EVT event, tBSA_AVK_MSG *p_data)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    tBSA_AVK_REM_RSP RemRsp;

    switch (event)
    {
    case BSA_AVK_OPEN_EVT:
        APP_DEBUG1("BSA_AVK_OPEN_EVT status 0x%x", p_data->open.status);

        if (p_data->open.status == BSA_SUCCESS)
        {
            connection = app_avk_add_connection(p_data->open.bd_addr);

            if(connection == NULL)
            {
                APP_DEBUG0("BSA_AVK_OPEN_EVT cannot allocate connection cb");
                break;
            }

            connection->ccb_handle = p_data->open.ccb_handle;
            connection->is_open = TRUE;
            connection->is_streaming_open = FALSE;
        }

        app_avk_cb.open_pending = FALSE;

        printf("AVK connected to device %02X:%02X:%02X:%02X:%02X:%02X\n",
            p_data->open.bd_addr[0], p_data->open.bd_addr[1],p_data->open.bd_addr[2],
            p_data->open.bd_addr[3], p_data->open.bd_addr[4],p_data->open.bd_addr[5]);
        break;

    case BSA_AVK_CLOSE_EVT:
        /* Close event, reason BTA_AVK_CLOSE_STR_CLOSED or BTA_AVK_CLOSE_CONN_LOSS*/
        APP_DEBUG1("BSA_AVK_CLOSE_EVT status 0x%x, handle %d", p_data->close.status, p_data->close.ccb_handle);
        connection = app_avk_find_connection_by_av_handle(p_data->close.ccb_handle);

        if(connection == NULL)
        {
            APP_DEBUG1("BSA_AVK_CLOSE_EVT unknown handle %d", p_data->open.ccb_handle);
            break;
        }

        connection->is_open = FALSE;
        app_avk_cb.open_pending = FALSE;
        if (connection->is_started == TRUE)
        {
            connection->is_started = FALSE;

            if (connection->format == BSA_AVK_CODEC_M24)
            {
                close(app_avk_cb.fd);
                app_avk_cb.fd  = -1;
            }
            else
                app_avk_close_wave_file(connection);

            /* close alsa dev */
            if (connection->format == BSA_AVK_CODEC_PCM){
#ifdef PCM_ALSA
                pthread_mutex_lock(&alsa_opt_mutex);

                /* If ALSA PCM driver was already open => close it */
                if (app_avk_cb.alsa_handle != NULL)
                {
                    printf("BSA_AVK_CLOSE_EVT snd_pcm_close\n");
                    snd_pcm_close(app_avk_cb.alsa_handle);
                    app_avk_cb.alsa_handle = NULL;
                }
	#ifdef BT_RESAMPLE
                if (out.resampler) {
					Destroy_Resampler(out.resampler);
					out.resampler = NULL;
				}

				if (out.buffer != NULL) {
          free(out.buffer);
          out.buffer = NULL;
        }
	#endif

                pthread_mutex_unlock(&alsa_opt_mutex);
#endif /* PCM_ALSA */
            }
        }
        app_avk_reset_connection(connection->bda_connected);
        break;

    case BSA_AVK_START_EVT:
        APP_DEBUG1("BSA_AVK_START_EVT status 0x%x, streaming ? %d", p_data->start.status, p_data->start.streaming);

        if(p_data->start.streaming == FALSE)
        {
            connection = app_avk_find_connection_by_av_handle(p_data->start.ccb_handle);
            if(connection == NULL)
            {
                break;
            }
            connection->is_streaming_open = TRUE;
        }
        else
        {
            /* We got START_EVT for a new device that is streaming but server discards the data
                because another stream is already active */
            if(p_data->start.discarded)
            {
                connection = app_avk_find_connection_by_bd_addr(p_data->start.bd_addr);
                if(connection)
                {
                    connection->is_started = TRUE;
                    connection->is_streaming_open = TRUE;
                }
                break;
            }

            pStreamingConn = NULL;

            connection = app_avk_find_connection_by_bd_addr(p_data->start.bd_addr);

            if(connection == NULL)
            {
                break;
            }

            if(connection->is_started)
            {
                connection->is_streaming_open = TRUE;
                pStreamingConn = connection;
				APP_DEBUG0("BT is playing music!");
                break;
            }
            else
            {
				APP_DEBUG0("BT is start!");
                connection->is_started = TRUE;
                app_avk_handle_start(p_data, connection);
                connection->is_streaming_open = TRUE;
                pStreamingConn = connection;
            }
        }
        break;

    case BSA_AVK_STOP_EVT:
        APP_DEBUG1("BSA_AVK_STOP_EVT streaming: %d  Suspended: %d", p_data->stop.streaming, p_data->stop.suspended);

				/* close alsa dev */
#ifdef PCM_ALSA
				pthread_mutex_lock(&alsa_opt_mutex);

				/* If ALSA PCM driver was already open => close it */
				if (app_avk_cb.alsa_handle != NULL)
				{
						printf("BSA_AVK_STOP_EVT snd_pcm_close\n");
						snd_pcm_close(app_avk_cb.alsa_handle);
						app_avk_cb.alsa_handle = NULL;
				}
#ifdef BT_RESAMPLE
				if (out.resampler) {
						Destroy_Resampler(out.resampler);
						out.resampler = NULL;
				}

				if (out.buffer != NULL) {
          free(out.buffer);
          out.buffer = NULL;
        }
#endif

				avk_pcm_close_output();
				pthread_mutex_unlock(&alsa_opt_mutex);
#endif /* PCM_ALSA */

        if(p_data->stop.suspended)
        {
            connection = app_avk_find_connection_by_bd_addr(p_data->stop.bd_addr);
            if(connection == NULL)
            {
                break;
            }

            connection->is_streaming_open = FALSE;
            connection->is_started = FALSE;
        }
        else if(p_data->stop.streaming == FALSE)
        {
            connection = app_avk_find_connection_by_av_handle(p_data->stop.ccb_handle);
            if(connection == NULL)
            {
                break;
            }

            connection->is_streaming_open = FALSE;
            connection->is_started = FALSE;
        }
        else
        {
            connection = app_avk_find_streaming_connection();

            if(connection == NULL)
            {
                break;
            }

            connection->is_started = FALSE;

            if (connection->format == BSA_AVK_CODEC_M24)
            {
                close(app_avk_cb.fd);
                app_avk_cb.fd  = -1;
            }
            else
                app_avk_close_wave_file(connection);

        }

        break;

    case BSA_AVK_RC_OPEN_EVT:

        if(p_data->rc_open.status == BSA_SUCCESS)
        {
            connection = app_avk_add_connection(p_data->rc_open.bd_addr);
            if(connection == NULL)
            {
                APP_DEBUG0("BSA_AVK_RC_OPEN_EVT could not allocate connection");
                break;
            }

            APP_DEBUG1("BSA_AVK_RC_OPEN_EVT peer_feature=0x%x rc_handle=%d",
                p_data->rc_open.peer_features, p_data->rc_open.rc_handle);
            connection->rc_handle = p_data->rc_open.rc_handle;
            connection->peer_features =  p_data->rc_open.peer_features;
            connection->peer_version = p_data->rc_open.peer_version;
            connection->is_rc_open = TRUE;
        }
        break;

    case BSA_AVK_RC_PEER_OPEN_EVT:

        connection = app_avk_find_connection_by_rc_handle(p_data->rc_open.rc_handle);
        if(connection == NULL)
        {
            APP_DEBUG1("BSA_AVK_RC_PEER_OPEN_EVT could not find connection handle %d", p_data->rc_open.rc_handle);
            break;
        }

        APP_DEBUG1("BSA_AVK_RC_PEER_OPEN_EVT peer_feature=0x%x rc_handle=%d",
            p_data->rc_open.peer_features, p_data->rc_open.rc_handle);

        connection->peer_features =  p_data->rc_open.peer_features;
        connection->peer_version = p_data->rc_open.peer_version;
        connection->is_rc_open = TRUE;
        break;

    case BSA_AVK_RC_CLOSE_EVT:
        APP_DEBUG0("BSA_AVK_RC_CLOSE_EVT");
        connection = app_avk_find_connection_by_rc_handle(p_data->rc_close.rc_handle);
        if(connection == NULL)
        {
            break;
        }

        connection->is_rc_open = FALSE;
        break;

    case BSA_AVK_REMOTE_RSP_EVT:
        APP_DEBUG0("BSA_AVK_REMOTE_RSP_EVT");
        APP_DEBUG1(" BSA_AVK_REMOTE_RSP_EVT:\n rc_handle = 0x%x  rc_id = 0x%x key_state = 0x%x", p_data->remote_rsp.rc_handle,p_data->remote_rsp.rc_id,p_data->remote_rsp.key_state);
        break;

    case BSA_AVK_REMOTE_CMD_EVT:
        APP_DEBUG0("BSA_AVK_REMOTE_CMD_EVT");
        APP_DEBUG1(" label:0x%x", p_data->remote_cmd.label);
        APP_DEBUG1(" op_id:0x%x", p_data->remote_cmd.op_id);
        APP_DEBUG1(" label:0x%x", p_data->remote_cmd.label);
        APP_DEBUG0(" avrc header");
        APP_DEBUG1("   ctype:0x%x", p_data->remote_cmd.hdr.ctype);
        APP_DEBUG1("   subunit_type:0x%x", p_data->remote_cmd.hdr.subunit_type);
        APP_DEBUG1("   subunit_id:0x%x", p_data->remote_cmd.hdr.subunit_id);
        APP_DEBUG1("   opcode:0x%x", p_data->remote_cmd.hdr.opcode);
        APP_DEBUG1(" len:0x%x", p_data->remote_cmd.len);
        APP_DUMP("data", p_data->remote_cmd.data, p_data->remote_cmd.len);

        BSA_AvkRemoteRspInit(&RemRsp);
        RemRsp.avrc_rsp = BSA_AVK_RSP_ACCEPT;
        RemRsp.label = p_data->remote_cmd.label;
        RemRsp.op_id = p_data->remote_cmd.op_id;
        RemRsp.key_state = p_data->remote_cmd.key_state;
        RemRsp.handle = p_data->remote_cmd.rc_handle;
        RemRsp.length = 0;
        BSA_AvkRemoteRsp(&RemRsp);
        break;

    case BSA_AVK_VENDOR_CMD_EVT:
        APP_DEBUG0("BSA_AVK_VENDOR_CMD_EVT");
        APP_DEBUG1(" code:0x%x", p_data->vendor_cmd.code);
        APP_DEBUG1(" company_id:0x%x", p_data->vendor_cmd.company_id);
        APP_DEBUG1(" label:0x%x", p_data->vendor_cmd.label);
        APP_DEBUG1(" len:0x%x", p_data->vendor_cmd.len);
#if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
        APP_DUMP("A2DP Data", p_data->vendor_cmd.data, p_data->vendor_cmd.len);
#endif
        break;

    case BSA_AVK_VENDOR_RSP_EVT:
        APP_DEBUG0("BSA_AVK_VENDOR_RSP_EVT");
        APP_DEBUG1(" code:0x%x", p_data->vendor_rsp.code);
        APP_DEBUG1(" company_id:0x%x", p_data->vendor_rsp.company_id);
        APP_DEBUG1(" label:0x%x", p_data->vendor_rsp.label);
        APP_DEBUG1(" len:0x%x", p_data->vendor_rsp.len);
#if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
        APP_DUMP("A2DP Data", p_data->vendor_rsp.data, p_data->vendor_rsp.len);
#endif
        break;

    case BSA_AVK_CP_INFO_EVT:
        APP_DEBUG0("BSA_AVK_CP_INFO_EVT");
        if(p_data->cp_info.id == BSA_AVK_CP_SCMS_T_ID)
        {
            switch(p_data->cp_info.info.scmst_flag)
            {
                case BSA_AVK_CP_SCMS_COPY_NEVER:
                    APP_INFO1(" content protection:0x%x - COPY NEVER", p_data->cp_info.info.scmst_flag);
                    break;
                case BSA_AVK_CP_SCMS_COPY_ONCE:
                    APP_INFO1(" content protection:0x%x - COPY ONCE", p_data->cp_info.info.scmst_flag);
                    break;
                case BSA_AVK_CP_SCMS_COPY_FREE:
                case (BSA_AVK_CP_SCMS_COPY_FREE+1):
                    APP_INFO1(" content protection:0x%x - COPY FREE", p_data->cp_info.info.scmst_flag);
                    break;
                default:
                    APP_ERROR1(" content protection:0x%x - UNKNOWN VALUE", p_data->cp_info.info.scmst_flag);
                    break;
            }
        }
        else
        {
            APP_INFO0("No content protection");
        }
        break;

    case BSA_AVK_REGISTER_NOTIFICATION_EVT:
    case BSA_AVK_LIST_PLAYER_APP_ATTR_EVT:
    case BSA_AVK_LIST_PLAYER_APP_VALUES_EVT:
    case BSA_AVK_SET_PLAYER_APP_VALUE_EVT:
    case BSA_AVK_GET_PLAYER_APP_VALUE_EVT:
    case BSA_AVK_GET_ELEM_ATTR_EVT:
    case BSA_AVK_GET_PLAY_STATUS_EVT:
    case BSA_AVK_SET_ADDRESSED_PLAYER_EVT:
    case BSA_AVK_SET_BROWSED_PLAYER_EVT:
    case BSA_AVK_GET_FOLDER_ITEMS_EVT:
    case BSA_AVK_CHANGE_PATH_EVT:
    case BSA_AVK_GET_ITEM_ATTR_EVT:
    case BSA_AVK_PLAY_ITEM_EVT:
    case BSA_AVK_ADD_TO_NOW_PLAYING_EVT:
        break;

    case BSA_AVK_SET_ABS_VOL_CMD_EVT:
        {
			APP_DEBUG0("BSA_AVK_SET_ABS_VOL_CMD_EVT");
            connection = app_avk_find_connection_by_rc_handle(p_data->abs_volume.handle);
            if((connection != NULL) && connection->m_bAbsVolumeSupported)
            {
                /* Peer requested change in volume. Make the change and send response with new system volume. BSA is TG role in AVK */
                if (p_data->abs_volume.abs_volume_cmd.volume <= BSA_MAX_ABS_VOLUME)
                {
                    app_avk_cb.volume = p_data->abs_volume.abs_volume_cmd.volume;
                }

                app_avk_set_abs_vol_rsp(p_data->abs_volume.abs_volume_cmd.volume,
                    connection->rc_handle, p_data->abs_volume.label);

                /* Change the code below based on which interface audio is going out to. */
                char buffer[100];
                int  alsa_vol = 0;
                alsa_vol = (63 * app_avk_cb.volume)/(BSA_MAX_ABS_VOLUME - BSA_MIN_ABS_VOLUME);
                APP_INFO1("abs volume %d, alsa volume %d", app_avk_cb.volume, alsa_vol);
                sprintf(buffer, "amixer cset name='headphone volume' '%d'", alsa_vol);
                system(buffer);
            }
            else
            {
                APP_ERROR1("not changing volume connection 0x%x m_bAbsVolumeSupported %d", connection, connection->m_bAbsVolumeSupported);
            }
        }
        break;

    case BSA_AVK_REG_NOTIFICATION_CMD_EVT:
        {
			APP_DEBUG0("BSA_AVK_REG_NOTIFICATION_CMD_EVT");
            if (p_data->reg_notif_cmd.reg_notif_cmd.event_id == AVRC_EVT_VOLUME_CHANGE)
            {
                connection = app_avk_find_connection_by_rc_handle(p_data->reg_notif_cmd.handle);
                if(connection != NULL)
                {
                    connection->m_bAbsVolumeSupported = TRUE;
                    connection->volChangeLabel = p_data->reg_notif_cmd.label;

                    /* Peer requested registration for vol change event. Send response with current system volume. BSA is TG role in AVK */
                    app_avk_reg_notfn_rsp(app_avk_cb.volume,
                                          p_data->reg_notif_cmd.handle,
                                          p_data->reg_notif_cmd.label,
                                          p_data->reg_notif_cmd.reg_notif_cmd.event_id,
                                          BTA_AV_RSP_INTERIM);
                }

            }
        }
        break;

    default:
        APP_ERROR1("Unsupported event %d", event);
        break;
    }

    /* forward the callback to registered applications */
    if(s_pCallback)
        s_pCallback(event, p_data);
}

int app_avk_close_pcm_alsa()
{
    /* close alsa dev */
#ifdef PCM_ALSA

    pthread_mutex_lock(&alsa_opt_mutex);
    /* If ALSA PCM driver was already open => close it */
    if (app_avk_cb.alsa_handle != NULL)
    {
        printf("app_avk_close_pcm_alsa snd_pcm_close\n");
        snd_pcm_close(app_avk_cb.alsa_handle);
        app_avk_cb.alsa_handle = NULL;
    }
    pcm_alsa_close_cmd = 1;
    pthread_mutex_unlock(&alsa_opt_mutex);

#endif /* PCM_ALSA */

    return 0;
}

int app_avk_resume_pcm_alsa()
{
    int status = 0;

    /* resume alsa dev  */
#ifdef PCM_ALSA
    pthread_mutex_lock(&alsa_opt_mutex);
    if (pcm_alsa_close_cmd == 1 && app_avk_cb.alsa_handle == NULL){
         /* Open ALSA driver */
        status = snd_pcm_open(&(app_avk_cb.alsa_handle), alsa_device,
            SND_PCM_STREAM_PLAYBACK, 0);
        if (status < 0)
        {
            APP_ERROR1("snd_pcm_open failed: %s", snd_strerror(status));
            pthread_mutex_unlock(&alsa_opt_mutex);
            return -1;
        }
        else
        {
            APP_DEBUG0("ALSA driver opened");
            /* Configure ALSA driver with PCM parameters */
            status = snd_pcm_set_params(app_avk_cb.alsa_handle, pcm_alsa_params.format,
                SND_PCM_ACCESS_RW_INTERLEAVED, pcm_alsa_params.num_channel,
                pcm_alsa_params.sample_rate, 1, 400000);/* 0.4sec */
            if (status < 0)
            {
                APP_ERROR1("snd_pcm_set_params failed: %s", snd_strerror(status));
				snd_pcm_close(app_avk_cb.alsa_handle);
				app_avk_cb.alsa_handle = NULL;
				pthread_mutex_unlock(&alsa_opt_mutex);
                return -1;
            }

            /* set output */
            //mixer_set("Speaker Function",2);
        }
        pcm_alsa_close_cmd = 0;
    }

    pthread_mutex_unlock(&alsa_opt_mutex);
#endif /* PCM_ALSA */

    return 0;
}


int app_avk_auto_connect(BD_ADDR bt_auto_addr)
{
    tBSA_STATUS status;
    int choice;
    BOOLEAN connect = FALSE;
    BD_ADDR bd_addr;
    UINT8 *p_name;
    tBSA_AVK_OPEN open_param;
    tAPP_AVK_CONNECTION *connection = NULL;

    if (app_avk_cb.open_pending)
    {
        APP_ERROR0("already trying to connect");
        return -1;
    }


        /* Read the XML file which contains all the bonded devices */
        app_read_xml_remote_devices();
        app_xml_display_devices(app_xml_remote_devices_db, APP_NUM_ELEMENTS(app_xml_remote_devices_db));

        choice = 0;
        while(choice < APP_NUM_ELEMENTS(app_xml_remote_devices_db))
        {
		  printf("bt %s, use %d\n", app_xml_remote_devices_db[choice].name, app_xml_remote_devices_db[choice].in_use);

            if (app_xml_remote_devices_db[choice].in_use != FALSE)
            {
		  bdcpy(bd_addr, app_xml_remote_devices_db[choice].bd_addr);
                if(bt_auto_addr[0] == bd_addr[0] && bt_auto_addr[1] == bd_addr[1]
                    && bt_auto_addr[2] == bd_addr[2] && bt_auto_addr[3] == bd_addr[3]
                    && bt_auto_addr[4] == bd_addr[4] && bt_auto_addr[5] == bd_addr[5]){
                    p_name = app_xml_remote_devices_db[choice].name;
                    connect = TRUE;
                    break;
                }
            }
            else
            {
                APP_ERROR0("Device entry not in use");
            }

            choice++;
        }


    if (connect != FALSE)
    {
        /* Open AVK stream */
        printf("Connecting to AV device:%s \n", p_name);

        app_avk_cb.open_pending = TRUE;

        BSA_AvkOpenInit(&open_param);
        memcpy((char *) (open_param.bd_addr), bd_addr, sizeof(BD_ADDR));

        open_param.sec_mask = BSA_SEC_NONE;
        status = BSA_AvkOpen(&open_param);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("Unable to connect to device %02X:%02X:%02X:%02X:%02X:%02X with status %d",
                    open_param.bd_addr[0], open_param.bd_addr[1], open_param.bd_addr[2],
                    open_param.bd_addr[3], open_param.bd_addr[4], open_param.bd_addr[5], status);

            app_avk_cb.open_pending = FALSE;
            return -1;
        }
        else
        {
            /* this is an active wait for demo purpose */
            printf("waiting for AV connection to open\n");

            while (app_avk_cb.open_pending == TRUE){
				if (connect_link_status == 0){
					printf("Link down when connect auto\n");
					return -1;
				}

				GKI_delay(10);
			}

            connection = app_avk_find_connection_by_bd_addr(open_param.bd_addr);
            if(connection == NULL || connection->is_open == FALSE)
            {
                printf("failure opening AV connection  \n");
                return -1;
            }
            else
            {
                /* XML Database update should be done upon reception of AV OPEN event */
                APP_DEBUG1("Connected to AV device:%s", p_name);
                /* Read the Remote device xml file to have a fresh view */
                app_read_xml_remote_devices();

                /* Add AV service for this devices in XML database */
                app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr,
                    BSA_A2DP_SERVICE_MASK | BSA_AVRCP_SERVICE_MASK);

                app_xml_update_name_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr, p_name);

                /* Update database => write to disk */
                if (app_write_xml_remote_devices() < 0)
                {
                    APP_ERROR0("Failed to store remote devices database");
                }
            }
        }
    }
    return 0;
}

int app_avk_connect_by_addr(BD_ADDR bd_addr)
{
    tBSA_STATUS status;
	char *bt_name = "bt test";
	UINT8 *p_name = (UINT8 *)bt_name;
	tBSA_AVK_OPEN open_param;
    tAPP_AVK_CONNECTION *connection = NULL;

    if (app_avk_cb.open_pending)
    {
        APP_ERROR0("already trying to connect");
        return -1;
    }

    /* Open AVK stream */
    printf("Connecting to device %02X:%02X:%02X:%02X:%02X:%02X\n",
                bd_addr[0], bd_addr[1], bd_addr[2],
                bd_addr[3], bd_addr[4], bd_addr[5]);

    app_avk_cb.open_pending = TRUE;

    BSA_AvkOpenInit(&open_param);
    memcpy((char *) (open_param.bd_addr), bd_addr, sizeof(BD_ADDR));

    open_param.sec_mask = BSA_SEC_NONE;
    status = BSA_AvkOpen(&open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to connect to device %02X:%02X:%02X:%02X:%02X:%02X with status %d",
                open_param.bd_addr[0], open_param.bd_addr[1], open_param.bd_addr[2],
                open_param.bd_addr[3], open_param.bd_addr[4], open_param.bd_addr[5], status);

        app_avk_cb.open_pending = FALSE;
        return -1;
    }
    else
    {
        /* this is an active wait for demo purpose */
        printf("waiting for AV connection to open\n");

		while (app_avk_cb.open_pending == TRUE){
			if (connect_link_status == 0){
				printf("Link down when connect by addr\n");
				return -1;
			}

			GKI_delay(10);
		}

        connection = app_avk_find_connection_by_bd_addr(open_param.bd_addr);
        if(connection == NULL || connection->is_open == FALSE)
        {
            printf("failure opening AV connection\n");
            return -1;
        }
        else
        {
            /* Read the Remote device xml file to have a fresh view */
            app_read_xml_remote_devices();

            /* Add AV service for this devices in XML database */
            app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr,
                BSA_A2DP_SERVICE_MASK | BSA_AVRCP_SERVICE_MASK);

            app_xml_update_name_db(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr, p_name);

            /* Update database => write to disk */
            if (app_write_xml_remote_devices() < 0)
            {
                APP_ERROR0("Failed to store remote devices database");
            }
        }
    }

    return 0;
}



/*******************************************************************************
 **
 ** Function         app_avk_open
 **
 ** Description      Example of function to open AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_open(void)
{
    tBSA_STATUS status;
    int choice;
    BOOLEAN connect = FALSE;
    BD_ADDR bd_addr;
    UINT8 *p_name;
    tBSA_AVK_OPEN open_param;
    tAPP_AVK_CONNECTION *connection = NULL;

    if (app_avk_cb.open_pending)
    {
        APP_ERROR0("already trying to connect");
        return;
    }

    printf("Bluetooth AVK Open menu:\n");
    printf("    0 Device from XML database (already paired)\n");
    printf("    1 Device found in last discovery\n");
    choice = app_get_choice("Select source");

    switch(choice)
    {
    case 0:
        /* Read the XML file which contains all the bonded devices */
        app_read_xml_remote_devices();

        app_xml_display_devices(app_xml_remote_devices_db, APP_NUM_ELEMENTS(app_xml_remote_devices_db));
        choice = app_get_choice("Select device");
        if ((choice >= 0) && (choice < APP_NUM_ELEMENTS(app_xml_remote_devices_db)))
        {
            if (app_xml_remote_devices_db[choice].in_use != FALSE)
            {
                bdcpy(bd_addr, app_xml_remote_devices_db[choice].bd_addr);
                p_name = app_xml_remote_devices_db[choice].name;
                connect = TRUE;
            }
            else
            {
                APP_ERROR0("Device entry not in use");
            }
        }
        else
        {
            APP_ERROR0("Unsupported device number");
        }
        break;
    case 1:
        app_disc_display_devices();
        choice = app_get_choice("Select device");
        if ((choice >= 0) && (choice < APP_NUM_ELEMENTS(app_discovery_cb.devs)))
        {
            if (app_discovery_cb.devs[choice].in_use != FALSE)
            {
                bdcpy(bd_addr, app_discovery_cb.devs[choice].device.bd_addr);
                p_name = app_discovery_cb.devs[choice].device.name;
                connect = TRUE;
            }
            else
            {
                APP_ERROR0("Device entry not in use");
            }
        }
        else
        {
            APP_ERROR0("Unsupported device number");
        }
        break;
    default:
        APP_ERROR0("Unsupported choice");
        break;
    }

    if (connect != FALSE)
    {
        /* Open AVK stream */
        printf("Connecting to AV device:%s \n", p_name);

        app_avk_cb.open_pending = TRUE;

        BSA_AvkOpenInit(&open_param);
        memcpy((char *) (open_param.bd_addr), bd_addr, sizeof(BD_ADDR));

        open_param.sec_mask = BSA_SEC_NONE;
        status = BSA_AvkOpen(&open_param);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("Unable to connect to device %02X:%02X:%02X:%02X:%02X:%02X with status %d",
                    open_param.bd_addr[0], open_param.bd_addr[1], open_param.bd_addr[2],
                    open_param.bd_addr[3], open_param.bd_addr[4], open_param.bd_addr[5], status);

            app_avk_cb.open_pending = FALSE;
        }
        else
        {
            /* this is an active wait for demo purpose */
            printf("waiting for AV connection to open\n");

            while (app_avk_cb.open_pending == TRUE);

            connection = app_avk_find_connection_by_bd_addr(open_param.bd_addr);
            if(connection == NULL || connection->is_open == FALSE)
            {
                printf("failure opening AV connection  \n");
            }
            else
            {
                /* XML Database update should be done upon reception of AV OPEN event */
                APP_DEBUG1("Connected to AV device:%s", p_name);
                /* Read the Remote device xml file to have a fresh view */
                app_read_xml_remote_devices();

                /* Add AV service for this devices in XML database */
                app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr,
                    BSA_A2DP_SERVICE_MASK | BSA_AVRCP_SERVICE_MASK);

                app_xml_update_name_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr, p_name);

                /* Update database => write to disk */
                if (app_write_xml_remote_devices() < 0)
                {
                    APP_ERROR0("Failed to store remote devices database");
                }
            }
        }
    }

}

/*******************************************************************************
 **
 ** Function         app_avk_close
 **
 ** Description      Function to close AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_close(BD_ADDR bda)
{
    tBSA_STATUS status;
    tBSA_AVK_CLOSE bsa_avk_close_param;
    tAPP_AVK_CONNECTION *connection = NULL;

    /* Close AVK connection */
    BSA_AvkCloseInit(&bsa_avk_close_param);

    connection = app_avk_find_connection_by_bd_addr(bda);

    if(connection == NULL)
    {
        APP_ERROR0("Unable to Close AVK connection , invalid BDA");
        return;
    }

    bsa_avk_close_param.ccb_handle = connection->ccb_handle;
    bsa_avk_close_param.rc_handle = connection->rc_handle;

    status = BSA_AvkClose(&bsa_avk_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Close AVK connection with status %d", status);
    }
    else
    {
        app_avk_reset_connection(bda);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_close_all
 **
 ** Description      Function to close all AVK connections
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_close_all()
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (app_avk_cb.connections[index].in_use == TRUE)
        {
            app_avk_close(app_avk_cb.connections[index].bda_connected);
        }
    }
}

/*******************************************************************************
**
** Function         app_avk_open_rc
**
** Description      Function to opens avrc controller connection. AVK should be open before opening rc.
**
** Returns          void
**
*******************************************************************************/
void app_avk_open_rc(BD_ADDR bd_addr)
{
    tBSA_STATUS status;
    tBSA_AVK_OPEN bsa_avk_open_param;

    /* open avrc connection */
    BSA_AvkOpenRcInit(&bsa_avk_open_param);
    bdcpy(bsa_avk_open_param.bd_addr, bd_addr);
    status = BSA_AvkOpenRc(&bsa_avk_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to open arvc connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function         app_avk_close_rc
**
** Description      Function to closes avrc controller connection.
**
** Returns          void
**
*******************************************************************************/
void app_avk_close_rc(UINT8 rc_handle)
{
    tBSA_STATUS status;
    tBSA_AVK_CLOSE bsa_avk_close_param;

    /* close avrc connection */
    BSA_AvkCloseRcInit(&bsa_avk_close_param);

    bsa_avk_close_param.rc_handle = rc_handle;

    status = BSA_AvkCloseRc(&bsa_avk_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to close arvc connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function         app_avk_close_str
**
** Description      Function to close an A2DP Steam connection
**
** Returns          void
**
*******************************************************************************/
void app_avk_close_str(UINT8 ccb_handle)
{
    tBSA_STATUS status;
    tBSA_AVK_CLOSE_STR bsa_avk_close_str_param;

    /* close avrc connection */
    BSA_AvkCloseStrInit(&bsa_avk_close_str_param);

    bsa_avk_close_str_param.ccb_handle = ccb_handle;

    status = BSA_AvkCloseStr(&bsa_avk_close_str_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to close arvc connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function        app_avk_register
**
** Description     Example of function to register an avk endpoint
**
** Returns         void
**
*******************************************************************************/
int app_avk_register(void)
{
    tBSA_STATUS status;
    tBSA_AVK_REGISTER bsa_avk_register_param;

    /* register an audio AVK end point */
    BSA_AvkRegisterInit(&bsa_avk_register_param);

    bsa_avk_register_param.media_sup_format.audio.pcm_supported = TRUE;
    bsa_avk_register_param.media_sup_format.audio.sec_supported = TRUE;
#if (defined(BSA_AVK_AAC_SUPPORTED) && (BSA_AVK_AAC_SUPPORTED==TRUE))
    /* Enable AAC support in the app */
    bsa_avk_register_param.media_sup_format.audio.aac_supported = TRUE;
#endif
    strncpy(bsa_avk_register_param.service_name, "BSA Audio Service",
        BSA_AVK_SERVICE_NAME_LEN_MAX);

    bsa_avk_register_param.reg_notifications = reg_notifications;

    status = BSA_AvkRegister(&bsa_avk_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to register an AV sink with status %d", status);
        return -1;
    }
    /* Save UIPC channel */
    app_avk_cb.uipc_audio_channel = bsa_avk_register_param.uipc_channel;

    /* open the UIPC channel to receive the pcm */
    if (UIPC_Open(bsa_avk_register_param.uipc_channel, app_avk_uipc_cback) == FALSE)
    {
        APP_ERROR0("Unable to open UIPC channel");
        return -1;
    }

    app_avk_cb.fd = -1;

    return 0;

}

/*******************************************************************************
 **
 ** Function        app_avk_deregister
 **
 ** Description     Example of function to deregister an avk endpoint
 **
 ** Returns         void
 **
 *******************************************************************************/
void app_avk_deregister(void)
{
    tBSA_STATUS status;
    tBSA_AVK_DEREGISTER req;

    /* register an audio AVK end point */
    BSA_AvkDeregisterInit(&req);
    status = BSA_AvkDeregister(&req);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to deregister an AV sink with status %d", status);
    }

#ifdef TODO
    if (app_avk_cb.format == BSA_AVK_CODEC_M24)
    {
        close(app_avk_cb.fd);
        app_avk_cb.fd  = -1;
    }
    else
        app_avk_close_wave_file();
#endif

    /* close the UIPC channel receiving the pcm */
    UIPC_Close(app_avk_cb.uipc_audio_channel);
    app_avk_cb.uipc_audio_channel = UIPC_CH_ID_BAD;

}
#define SAVE_AVK_FILE_LENGTH     44100*240
static int avk_length = 0;

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MEM_BUFFER_SIZE (1280*512)
#define ITEM_SIZE 1280
#define MAX_ITEM_NUMBER (512*4)

static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
static int item_number;

struct tailq_entry {
	char mem[ITEM_SIZE];

	TAILQ_ENTRY(tailq_entry) entries;
};

TAILQ_HEAD(, tailq_entry) my_tailq_head = TAILQ_HEAD_INITIALIZER(my_tailq_head);

static void write_to_file(char *data, int len)
{

}

int app_avk_memqueue_push(char mem[ITEM_SIZE])
{
	struct tailq_entry *newitem;

	if(item_number>MAX_ITEM_NUMBER){
		printf("memqueue overflow\n");
		app_avk_memqueue_clear();
		return -1;
	}

	newitem = malloc(sizeof(*newitem));
	if(!newitem)
		return -1;

	memcpy(newitem->mem, mem, ITEM_SIZE);
	pthread_mutex_lock(&write_lock);
	TAILQ_INSERT_TAIL(&my_tailq_head, newitem, entries);
	++item_number;
	pthread_mutex_unlock(&write_lock);

	return 0;
}

int app_avk_memqueue_pop(char mem[ITEM_SIZE])
{
	struct tailq_entry *item;

	pthread_mutex_lock(&write_lock);
	item = TAILQ_FIRST(&my_tailq_head);
	if(item){
		TAILQ_REMOVE(&my_tailq_head, item, entries);
		--item_number;
	}
	pthread_mutex_unlock(&write_lock);

	if(item){
		memcpy(mem, item->mem, ITEM_SIZE);
		free(item);
		return 0;
	}else{
		return -1;
	}
}

void app_avk_memqueue_clear(void)
{
	struct tailq_entry *item;

	pthread_mutex_lock(&write_lock);
	while ((item = TAILQ_FIRST(&my_tailq_head))) {
		TAILQ_REMOVE(&my_tailq_head, item, entries);
		free(item);
	}
	item_number = 0;
	pthread_mutex_unlock(&write_lock);
}

int app_avk_memqueue_isempty(void)
{
	int isempty;

	pthread_mutex_lock(&write_lock);
	isempty = TAILQ_EMPTY(&my_tailq_head);
	pthread_mutex_unlock(&write_lock);

	return isempty;
}
# if 0
static void *app_avk_write_buffer_thread_main(void *arg)
{
	int count=0;
	int len;
	int16_t * buf;
	snd_pcm_sframes_t alsa_frames_to_send;
	int frame;
	tAPP_AVK_CONNECTION *connection = NULL;
	char mem[ITEM_SIZE];
	size_t frame_size = audio_stream_frame_size();
	size_t out_frames = RESAMPLER_BUFFER_SIZE / frame_size;
	pthread_detach(pthread_self());
	printf("app_avk_write_buffer_thread_main\n");
	while(pcm_thread_running){
		if(app_avk_memqueue_isempty()){
			usleep(5000);
			continue;
		}else{
			    count++;
			    if (app_avk_cb.alsa_handle!= NULL)
			    {
					app_avk_memqueue_pop(mem);
					len = *((int* )mem);
					alsa_frames_to_send = len;
					if (out.resampler) {
					out.resampler->resample_from_input(out.resampler,
	                                            (int16_t *)(mem+4),
	                                            &alsa_frames_to_send,
	                                            (int16_t *)out.buffer,
	                                            &out_frames);
					buf = out.buffer;
					printf("alsa_frame_to_send %d,out_frames %d \n",alsa_frames_to_send, out_frames);
				} else {
					out_frames = alsa_frames_to_send;
					buf = (void *)(mem+4);
				}
					frame = snd_pcm_writei(app_avk_cb.alsa_handle, buf, out_frames);
					if (frame < 0)
					{
					    frame = snd_pcm_recover(app_avk_cb.alsa_handle, frame, 0);
					    printf("snd_pcm_writei error,recover it \n");

					 }
			    } else{
					app_avk_memqueue_clear();
			    }
			if(count==200){
				printf("app_avk_write_buffer_thread_main item_number = %d, app_avk_cb.alsa_handle = %d \n",item_number,app_avk_cb.alsa_handle);
				count=0;
			}
		}
	}
	app_avk_memqueue_clear();
	return NULL;
}

void app_avk_write_buffer(void)
{
	pthread_t thread;
	pcm_thread_running = 1;
	if(pthread_create(&thread, NULL, app_avk_write_buffer_thread_main,NULL)){
		printf("app_avk_write_buffer_thread_main pthread_create failed!!!\n");
	}
}
#endif
/*******************************************************************************
**
** Function        app_avk_uipc_cback
**
** Description     uipc audio call back function.
**
** Parameters      pointer on a buffer containing PCM sample with a BT_HDR header
**
** Returns          void
**
*******************************************************************************/
unsigned int alsa_total_frame = 0;
unsigned int out_total_frame = 0;
unsigned int count  = 0;
snd_pcm_sframes_t alsa_total_frames = 0;
static void app_avk_uipc_cback(BT_HDR *p_msg)
{
	struct timeval tv;
#ifdef PCM_ALSA
    snd_pcm_sframes_t alsa_frames;
    snd_pcm_sframes_t avail_frames;
    snd_pcm_sframes_t alsa_frames_to_send;
#endif
	UINT8 *p_buffer;
#ifdef BT_RESAMPLE
	size_t frame_size = 0;
    size_t out_frames;
    void *buf;
#endif
    int dummy;
    tAPP_AVK_CONNECTION *connection = NULL;

    if (p_msg == NULL)
    {
        APP_ERROR0("app_avk_uipc_cback p_msg is NULL ?!");
        return;
    }

    p_buffer = ((UINT8 *)(p_msg + 1)) + p_msg->offset;

    connection = app_avk_find_streaming_connection();

    if(!connection)
    {
        APP_ERROR0("Unable to find connection!!!!!!");
        GKI_freebuf(p_msg);
        return;
    }

    if (connection->format == BSA_AVK_CODEC_M24)
    {
        /* Invoke AAC decoder here for the current buffer.
            decode the AAC file that is created */
        if (app_avk_cb.fd != -1)
        {
            dummy = write(app_avk_cb.fd, p_buffer, p_msg->len);
            (void)dummy;
        }

        GKI_freebuf(p_msg);
        return;
    }

    if (connection->format == BSA_AVK_CODEC_PCM &&
            app_avk_cb.fd_codec_type != BSA_AVK_CODEC_M24)
    {
        /* Invoke AAC decoder here for the current buffer.
            decode the AAC file that is created */
        if (app_avk_cb.fd != -1)
        {
            APP_DEBUG1("app_avk_uipc_cback Writing Data 0x%x, len = %d", p_buffer, p_msg->len);
            dummy = write(app_avk_cb.fd, p_buffer, p_msg->len);
            (void)dummy;
        }

        #if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
            APP_DUMP("A2DP Data", p_buffer, p_msg->len);
        #endif

    }


    if(connection->num_channel == 0)
    {
        APP_ERROR0("connection->num_channel == 0");
        GKI_freebuf(p_msg);
        return;
    }

#ifdef USE_MEMQ_PCM
    if ( app_avk_cb.alsa_handle != NULL && p_buffer )
    {
        /* Compute number of PCM samples (contained in p_msg->len bytes) */
        alsa_frames_to_send = p_msg->len / connection->num_channel;
        if (connection->bit_per_sample == 16)
            alsa_frames_to_send /= 2;
		memcpy(a_data,(char*)&alsa_frames_to_send,4);
		memcpy(a_data+4, p_buffer, p_msg->len);
		app_avk_memqueue_push(a_data);
    }
#else
#ifdef PCM_ALSA
    pthread_mutex_lock(&alsa_opt_mutex);

    if (app_avk_cb.alsa_handle != NULL && p_buffer)
    {
        /* Compute number of PCM samples (contained in p_msg->len bytes) */
        alsa_frames_to_send = p_msg->len / connection->num_channel;
        if (connection->bit_per_sample == 16)
            alsa_frames_to_send /= 2;

        //if(next + alsa_frames_to_send < 1024)
        //{
        //	memcpy(a_data+next*4, p_buffer, p_msg->len);
        //	next += alsa_frames_to_send;
        //	GKI_freebuf(p_msg);
        //	return ;
        //}
		//memcpy(a_data+next*4, p_buffer, p_msg->len);
#ifdef BT_RESAMPLE
		frame_size = audio_stream_frame_size();
		out_frames = RESAMPLER_BUFFER_SIZE / frame_size;
		out.cfg.samples = p_msg->len /4;
		out.cfg.inbuf = p_buffer;
		/* only use resampler if required */
	    if (out.resampler) {
#if 0
	        out.resampler->resample_from_input(out.resampler,
	                                            (int16_t *)p_buffer,
	                                            &alsa_frames_to_send,
	                                            (int16_t *)out.buffer,
	                                            &out_frames);
#endif
		out.resampler->prepare(out.resampler, &out.cfg);
		out_frames = out.resampler->process(out.resampler);
		out.resampler->getData(out.resampler, out.buffer, out_frames*2*out.cfg.inch);
		buf = out.buffer;
	    } else {
	        out_frames = alsa_frames_to_send;
	        buf = (void *)p_buffer;
	    }
        alsa_frames = snd_pcm_writei(app_avk_cb.alsa_handle, buf, out_frames);
	alsa_total_frames += alsa_frames;
	count ++;
	if(count == 1) {
		gettimeofday(&tv,NULL);
		printf("second : %d ,ussec : %d\n",tv.tv_sec, tv.tv_usec);
	}
#else
		alsa_frames = snd_pcm_writei(app_avk_cb.alsa_handle, p_buffer, alsa_frames_to_send);

		//printf("alsa_frames_to_send %d , out_frames %d\n",alsa_frames_to_send,alsa_frames);
#endif
        //alsa_frames = snd_pcm_writei(app_avk_cb.alsa_handle, a_data, 1024);
        if (alsa_frames < 0)
        {
		gettimeofday(&tv, NULL);
	    avail_frames = snd_pcm_avail_update(app_avk_cb.alsa_handle);
	    printf("avail_frames = %d, alsa_total_frames = %d , sec = %d, ussec = %d \n",avail_frames,alsa_total_frames, tv.tv_sec, tv.tv_usec);
            alsa_frames = snd_pcm_recover(app_avk_cb.alsa_handle, alsa_frames, 0);
        }
	struct timeval tv;
	gettimeofday(&tv, 0);
#ifdef BT_RESAMPLE
        if (alsa_frames > 0 && alsa_frames < out_frames)
            printf("app_avk_uipc_cback Short write (expected %li, wrote %li)\n",
            (long) out_frames, alsa_frames);
#else
		if (alsa_frames > 0 && alsa_frames < alsa_frames_to_send)
            printf("app_avk_uipc_cback Short write (expected %li, wrote %li)\n",
            (long) alsa_frames_to_send, alsa_frames);
#endif
        //next = 0;
    }

    pthread_mutex_unlock(&alsa_opt_mutex);
#endif /* PCM_ALSA */
#endif /* USE_MEMQ_PCM */
    GKI_freebuf(p_msg);
}

/*******************************************************************************
**
** Function         app_avk_init
**
** Description      Init Manager application
**
** Parameters       Application callback (if null, default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_avk_init(tAvkCallback pcb /* = NULL */)
{
    tBSA_AVK_ENABLE bsa_avk_enable_param;
    tBSA_STATUS status;

    /* register callback */
    s_pCallback = pcb;

    /* Initialize the control structure */
    memset(&app_avk_cb, 0, sizeof(app_avk_cb));
    app_avk_cb.uipc_audio_channel = UIPC_CH_ID_BAD;

    app_avk_cb.fd = -1;

    /* set sytem vol at 50/63 of max */
    app_avk_cb.volume = (UINT8)((50 *(BSA_MAX_ABS_VOLUME))/63);

    /* get hold on the AVK resource, synchronous mode */
    BSA_AvkEnableInit(&bsa_avk_enable_param);

    bsa_avk_enable_param.sec_mask = BSA_AVK_SECURITY;
    bsa_avk_enable_param.features = BSA_AVK_FEATURES;
    bsa_avk_enable_param.p_cback = app_avk_cback;

    status = BSA_AvkEnable(&bsa_avk_enable_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR0("Unable to enable AVK service");
        return -1;
    }

    pthread_mutex_init(&alsa_opt_mutex, NULL);

    return BSA_SUCCESS;

}

/*******************************************************************************
**
** Function         app_avk_init_no_avrcp
**
** Description      Init Manager application
**
** Parameters       Application callback (if null, default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_avk_init_no_avrcp(tAvkCallback pcb /* = NULL */)
{
    tBSA_AVK_ENABLE bsa_avk_enable_param;
    tBSA_STATUS status;

    /* register callback */
    s_pCallback = pcb;

    /* Initialize the control structure */
    memset(&app_avk_cb, 0, sizeof(app_avk_cb));
    app_avk_cb.uipc_audio_channel = UIPC_CH_ID_BAD;

    app_avk_cb.fd = -1;

    /* set sytem vol at 50% */
    app_avk_cb.volume = (UINT8)((BSA_MAX_ABS_VOLUME - BSA_MIN_ABS_VOLUME)>>1);

    /* get hold on the AVK resource, synchronous mode */
    BSA_AvkEnableInit(&bsa_avk_enable_param);

    bsa_avk_enable_param.sec_mask = BSA_AVK_SECURITY;
    bsa_avk_enable_param.features = BSA_AVK_FEATURES_NO_AVRCP;
    bsa_avk_enable_param.p_cback = app_avk_cback;

    status = BSA_AvkEnable(&bsa_avk_enable_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR0("Unable to enable AVK service");
        return -1;
    }

    pthread_mutex_init(&alsa_opt_mutex, NULL);

    return BSA_SUCCESS;

}

/*******************************************************************************
**
** Function         app_avk_rc_send_command
**
** Description      Example of Send a RemoteControl command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_command(UINT8 key_state, UINT8 command, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_REM_CMD bsa_avk_rc_cmd;

    /* Send remote control command */
    status = BSA_AvkRemoteCmdInit(&bsa_avk_rc_cmd);
    bsa_avk_rc_cmd.rc_handle = rc_handle;
    bsa_avk_rc_cmd.key_state = (tBSA_AVK_STATE)key_state;
    bsa_avk_rc_cmd.rc_id = (tBSA_AVK_RC)command;
    bsa_avk_rc_cmd.label = app_avk_get_label();

    status = BSA_AvkRemoteCmd(&bsa_avk_rc_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV RC Command %d", status);
    }
}

/*******************************************************************************
**
** Function         app_avk_rc_send_click
**
** Description      Send press and release states of a command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_click(UINT8 command, UINT8 rc_handle)
{
    app_avk_rc_send_command(BSA_AV_STATE_PRESS, command, rc_handle);
    GKI_delay(300);
    app_avk_rc_send_command(BSA_AV_STATE_RELEASE, command, rc_handle);
}


/*******************************************************************************
**
** Function         app_avk_play_start
**
** Description      Example of start steam play
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_start(UINT8 rc_handle)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    connection = app_avk_find_connection_by_rc_handle(rc_handle);
    if(connection == NULL)
    {
        APP_DEBUG1("app_avk_play_start could not find connection handle %d", rc_handle);
        return;
    }

    if ((connection->peer_features & BTA_RC_FEAT_CONTROL) && connection->is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_PLAY, rc_handle);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
            (connection->peer_features & BTA_RC_FEAT_CONTROL), connection->is_rc_open);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_play_stop
 **
 ** Description      Example of stop steam play
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_stop(UINT8 rc_handle)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    connection = app_avk_find_connection_by_rc_handle(rc_handle);
    if(connection == NULL)
    {
        APP_DEBUG1("app_avk_play_stop could not find connection handle %d", rc_handle);
        return;
    }

    if ((connection->peer_features & BTA_RC_FEAT_CONTROL) && connection->is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_STOP, rc_handle);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
            (connection->peer_features & BTA_RC_FEAT_CONTROL), connection->is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_pause
 **
 ** Description      Example of pause steam pause
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_pause(UINT8 rc_handle)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    connection = app_avk_find_connection_by_rc_handle(rc_handle);
    if(connection == NULL)
    {
        APP_DEBUG1("app_avk_play_pause could not find connection handle %d", rc_handle);
        return;
    }

    if ((connection->peer_features & BTA_RC_FEAT_CONTROL) && connection->is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_PAUSE, rc_handle);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
                    (connection->peer_features & BTA_RC_FEAT_CONTROL), connection->is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_next_track
 **
 ** Description      Example of play next track
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_next_track(UINT8 rc_handle)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    connection = app_avk_find_connection_by_rc_handle(rc_handle);
    if(connection == NULL)
    {
        APP_DEBUG1("app_avk_play_pause could not find connection handle %d", rc_handle);
        return;
    }

    if ((connection->peer_features & BTA_RC_FEAT_CONTROL) && connection->is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_FORWARD, rc_handle);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
                    (connection->peer_features & BTA_RC_FEAT_CONTROL), connection->is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_previous_track
 **
 ** Description      Example of play previous track
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_previous_track(UINT8 rc_handle)
{
    tAPP_AVK_CONNECTION *connection = NULL;
    connection = app_avk_find_connection_by_rc_handle(rc_handle);
    if(connection == NULL)
    {
        APP_DEBUG1("app_avk_play_pause could not find connection handle %d", rc_handle);
        return;
    }

    if ((connection->peer_features & BTA_RC_FEAT_CONTROL) && connection->is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_BACKWARD, rc_handle);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
                    (connection->peer_features & BTA_RC_FEAT_CONTROL), connection->is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_cmd
 **
 ** Description      Example of function to open AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_cmd(UINT8 rc_handle)
{
    int choice;

    do
    {
        printf("Bluetooth AVK AVRC CMD menu:\n");
        printf("    0 play\n");
        printf("    1 stop\n");
        printf("    2 pause\n");
        printf("    3 forward\n");
        printf("    4 backward\n");
        printf("    5 rewind key_press\n");
        printf("    6 rewind key_release\n");
        printf("    7 fast forward key_press\n");
        printf("    8 fast forward key_release\n");

        printf("    99 exit\n");

        choice = app_get_choice("Select source");

        switch(choice)
        {
            case 0:
                app_avk_rc_send_click(BSA_AVK_RC_PLAY, rc_handle);
                break;

            case 1:
                app_avk_rc_send_click(BSA_AVK_RC_STOP, rc_handle);
                break;

            case 2:
                app_avk_rc_send_click(BSA_AVK_RC_PAUSE, rc_handle);
                break;

            case 3:
                app_avk_rc_send_click(BSA_AVK_RC_FORWARD, rc_handle);
                break;

            case 4:
                app_avk_rc_send_click(BSA_AVK_RC_BACKWARD, rc_handle);
                break;

            case 5:
                app_avk_rc_send_command(BSA_AV_STATE_PRESS, BSA_AVK_RC_REWIND, rc_handle);
                break;

            case 6:
                app_avk_rc_send_command(BSA_AV_STATE_RELEASE, BSA_AVK_RC_REWIND, rc_handle);
                break;

            case 7:
                app_avk_rc_send_command(BSA_AV_STATE_PRESS, BSA_AVK_RC_FAST_FOR, rc_handle);
                break;

            case 8:
                app_avk_rc_send_command(BSA_AV_STATE_RELEASE, BSA_AVK_RC_FAST_FOR, rc_handle);
                break;

            default:
                APP_ERROR0("Unsupported choice");
            break;
        }
    } while (choice != 99);
}


/*******************************************************************************
 **
 ** Function         app_avk_send_get_element_attributes_cmd
 **
 ** Description      Example of function to send Vendor Dependent Command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_send_get_element_attributes_cmd(UINT8 rc_handle)
{
    int status;
    tBSA_AVK_VEN_CMD bsa_avk_vendor_cmd;

    /* Send remote control command */
    status = BSA_AvkVendorCmdInit(&bsa_avk_vendor_cmd);

    bsa_avk_vendor_cmd.rc_handle = rc_handle;
    bsa_avk_vendor_cmd.ctype = BSA_AVK_CMD_STATUS;
    bsa_avk_vendor_cmd.data[0] = BSA_AVK_RC_VD_GET_ELEMENT_ATTR;
    bsa_avk_vendor_cmd.data[1] = 0; /* reserved & packet type */
    bsa_avk_vendor_cmd.data[2] = 0; /* length high*/
    bsa_avk_vendor_cmd.data[3] = 0x11; /* length low*/

    /* data 4 ~ 11 are 0, which means "identifier 0x0 PLAYING" */

    bsa_avk_vendor_cmd.data[12] = 2; /* num of attributes */

    /* data 13 ~ 16 are 0x1, which means "attribute ID 1 : Title of media" */
    bsa_avk_vendor_cmd.data[16] = 0x1;

    /* data 17 ~ 20 are 0x7, which means "attribute ID 2 : Playing Time" */
    bsa_avk_vendor_cmd.data[20] = 0x7;

    bsa_avk_vendor_cmd.length = 21;
    bsa_avk_vendor_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkVendorCmd(&bsa_avk_vendor_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV Vendor Command %d", status);
    }
}

/*******************************************************************************
**
** Function         avk_is_open_pending
**
** Description      Check if AVK Open is pending
**
** Parameters
**
** Returns          TRUE if open is pending, FALSE otherwise
**
*******************************************************************************/
BOOLEAN avk_is_open_pending()
{
    return app_avk_cb.open_pending;
}

/*******************************************************************************
**
** Function         avk_set_open_pending
**
** Description      Set AVK open pending
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void avk_set_open_pending(BOOLEAN bopenpend)
{
    app_avk_cb.open_pending = bopenpend;
}

/*******************************************************************************
**
** Function         avk_is_open
**
** Description      Check if AVK is open
**
** Parameters
**
** Returns          TRUE if AVK is open, FALSE otherwise. Return BDA of the connected device
**
*******************************************************************************/
BOOLEAN avk_is_open(BD_ADDR bda)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if ((app_avk_cb.connections[index].in_use == TRUE) &&
                (app_avk_cb.connections[index].is_open == TRUE))
        {
            bdcpy(bda, app_avk_cb.connections[index].bda_connected);
            return TRUE;
        }
    }

    return FALSE;
}

/*******************************************************************************
**
** Function         avk_is_rc_open
**
** Description      Check if AVRC is open
**
** Parameters
**
** Returns          TRUE if AVRC is open, FALSE otherwise.
**
*******************************************************************************/
BOOLEAN avk_is_rc_open(BD_ADDR bda)
{
    tAPP_AVK_CONNECTION *connection = app_avk_find_connection_by_bd_addr(bda);

    if(connection == NULL)
        return FALSE;

    return connection->is_rc_open;
}

/*******************************************************************************
**
** Function         app_avk_cancel
**
** Description      Example of cancel connection command
**
** Returns          void
**
*******************************************************************************/
void app_avk_cancel()
{
    int status;
    tBSA_AVK_CANCEL_CMD bsa_avk_cancel_cmd;

    /* Send remote control command */
    status = BSA_AvkCancelCmdInit(&bsa_avk_cancel_cmd);

    status = BSA_AvkCancelCmd(&bsa_avk_cancel_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_cancel_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_element_attr_command
 **
 ** Description      Example of get_element_attr_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_element_attr_command(UINT8 rc_handle)
{
    int status;
    tBSA_AVK_GET_ELEMENT_ATTR bsa_avk_get_elem_attr_cmd;

    UINT8 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetElementAttrCmdInit(&bsa_avk_get_elem_attr_cmd);
    bsa_avk_get_elem_attr_cmd.rc_handle = rc_handle;
    bsa_avk_get_elem_attr_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_avk_get_elem_attr_cmd.num_attr = num_attr;
    memcpy(bsa_avk_get_elem_attr_cmd.attrs, attrs, sizeof(attrs));

    status = BSA_AvkGetElementAttrCmd(&bsa_avk_get_elem_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_element_attr_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_command
 **
 ** Description      Example of app_avk_rc_list_player_attr_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_command(UINT8 rc_handle)
{
    int status;
    tBSA_AVK_LIST_PLAYER_ATTR bsa_avk_list_player_attr_cmd;

    /* Send command */
    status = BSA_AvkListPlayerAttrCmdInit(&bsa_avk_list_player_attr_cmd);
    bsa_avk_list_player_attr_cmd.rc_handle = rc_handle;
    bsa_avk_list_player_attr_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkListPlayerAttrCmd(&bsa_avk_list_player_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_list_player_attr_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_value_command
 **
 ** Description      Example of app_avk_rc_list_player_attr_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_value_command(UINT8 attr, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_LIST_PLAYER_VALUES bsa_avk_list_player_attr_cmd;

    /* Send command */
    status = BSA_AvkListPlayerValuesCmdInit(&bsa_avk_list_player_attr_cmd);
    bsa_avk_list_player_attr_cmd.rc_handle = rc_handle;
    bsa_avk_list_player_attr_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */
    bsa_avk_list_player_attr_cmd.attr = attr;

    status = BSA_AvkListPlayerValuesCmd(&bsa_avk_list_player_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_list_player_attr_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_player_value_command
 **
 ** Description      Example of get_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_player_value_command(UINT8 *attrs, UINT8 num_attr, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_GET_PLAYER_VALUE bsa_avk_get_player_val_cmd;

    /* Send command */
    status = BSA_AvkGetPlayerValueCmdInit(&bsa_avk_get_player_val_cmd);
    bsa_avk_get_player_val_cmd.rc_handle = rc_handle;
    bsa_avk_get_player_val_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */
    bsa_avk_get_player_val_cmd.num_attr = num_attr;

    memcpy(bsa_avk_get_player_val_cmd.attrs, attrs, sizeof(UINT8) * num_attr);

    status = BSA_AvkGetPlayerValueCmd(&bsa_avk_get_player_val_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_player_value_command %d", status);
    }
}



/*******************************************************************************
 **
 ** Function         app_avk_rc_set_player_value_command
 **
 ** Description      Example of set_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_player_value_command(UINT8 num_attr, UINT8 *attr_ids, UINT8 * attr_val, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_SET_PLAYER_VALUE bsa_avk_set_player_val_cmd;

    /* Send command */
    status = BSA_AvkSetPlayerValueCmdInit(&bsa_avk_set_player_val_cmd);
    bsa_avk_set_player_val_cmd.rc_handle = rc_handle;
    bsa_avk_set_player_val_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */
    bsa_avk_set_player_val_cmd.num_attr = num_attr;

    memcpy(bsa_avk_set_player_val_cmd.player_attr, attr_ids, sizeof(UINT8) * num_attr);
    memcpy(bsa_avk_set_player_val_cmd.player_value, attr_val, sizeof(UINT8) * num_attr);

    status = BSA_AvkSetPlayerValueCmd(&bsa_avk_set_player_val_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_player_value_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_play_status_command
 **
 ** Description      Example of get_play_status
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_play_status_command(UINT8 rc_handle)
{
    int status;
    tBSA_AVK_GET_PLAY_STATUS bsa_avk_play_status_cmd;

    /* Send command */
    status = BSA_AvkGetPlayStatusCmdInit(&bsa_avk_play_status_cmd);
    bsa_avk_play_status_cmd.rc_handle = rc_handle;
    bsa_avk_play_status_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkGetPlayStatusCmd(&bsa_avk_play_status_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_play_status_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_set_browsed_player_command
 **
 ** Description      Example of set_browsed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_browsed_player_command(UINT16  player_id, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_SET_BROWSED_PLAYER bsa_avk_set_browsed_player;

    /* Send command */
    status = BSA_AvkSetBrowsedPlayerCmdInit(&bsa_avk_set_browsed_player);
    bsa_avk_set_browsed_player.player_id = player_id;
    bsa_avk_set_browsed_player.rc_handle = rc_handle;
    bsa_avk_set_browsed_player.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkSetBrowsedPlayerCmd(&bsa_avk_set_browsed_player);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_browsed_player_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_set_addressed_player_command
 **
 ** Description      Example of set_addressed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_addressed_player_command(UINT16  player_id, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_SET_ADDR_PLAYER bsa_avk_set_player;

    /* Send command */
    status = BSA_AvkSetAddressedPlayerCmdInit(&bsa_avk_set_player);
    bsa_avk_set_player.player_id = player_id;
    bsa_avk_set_player.rc_handle = rc_handle;
    bsa_avk_set_player.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkSetAddressedPlayerCmd(&bsa_avk_set_player);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_addressed_player_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_change_path_command
 **
 ** Description      Example of change_path
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_change_path_command(UINT16   uid_counter, UINT8  direction, tAVRC_UID folder_uid, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_CHG_PATH bsa_change_path;

    /* Send command */
    status = BSA_AvkChangePathCmdInit(&bsa_change_path);

    bsa_change_path.rc_handle = rc_handle;
    bsa_change_path.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_change_path.uid_counter = uid_counter;
    bsa_change_path.direction = direction;
    memcpy(bsa_change_path.folder_uid, folder_uid, sizeof(tAVRC_UID));

    status = BSA_AvkChangePathCmd(&bsa_change_path);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_change_path_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_folder_items
 **
 ** Description      Example of get_folder_items
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_folder_items(UINT8  scope, UINT32  start_item, UINT32  end_item, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_GET_FOLDER_ITEMS bsa_get_folder_items;

    UINT32 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetFolderItemsCmdInit(&bsa_get_folder_items);

    bsa_get_folder_items.rc_handle = rc_handle;
    bsa_get_folder_items.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_get_folder_items.scope = scope;
    bsa_get_folder_items.start_item = start_item;
    bsa_get_folder_items.end_item = end_item;

    if(AVRC_SCOPE_PLAYER_LIST != scope)
    {
        bsa_get_folder_items.attr_count = num_attr;
        memcpy(bsa_get_folder_items.attrs, attrs, sizeof(attrs));
    }


    status = BSA_AvkGetFolderItemsCmd(&bsa_get_folder_items);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_folder_items %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_items_attr
 **
 ** Description      Example of get_items_attr
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_items_attr(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_GET_ITEMS_ATTR bsa_get_items_attr;

    UINT8 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetItemsAttrCmdInit(&bsa_get_items_attr);

    bsa_get_items_attr.rc_handle = rc_handle;
    bsa_get_items_attr.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_get_items_attr.scope = scope;
    memcpy(bsa_get_items_attr.uid, uid, sizeof(tAVRC_UID));
    bsa_get_items_attr.uid_counter = uid_counter;
    bsa_get_items_attr.attr_count = num_attr;
    memcpy(bsa_get_items_attr.attrs, attrs, sizeof(attrs));

    status = BSA_AvkGetItemsAttrCmd(&bsa_get_items_attr);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_items_attr %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_play_item
 **
 ** Description      Example of play_item
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_play_item(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_PLAY_ITEM bsa_play_item;


    /* Send command */
    status = BSA_AvkPlayItemCmdInit(&bsa_play_item);

    bsa_play_item.rc_handle = rc_handle;
    bsa_play_item.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_play_item.scope = scope;
    memcpy(bsa_play_item.uid, uid, sizeof(tAVRC_UID));
    bsa_play_item.uid_counter = uid_counter;

    status = BSA_AvkPlayItemCmd(&bsa_play_item);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_play_item %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_add_to_play
 **
 ** Description      Example of add_to_play
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_add_to_play(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_ADD_TO_PLAY bsa_add_play;


    /* Send command */
    status = BSA_AvkAddToPlayCmdInit(&bsa_add_play);

    bsa_add_play.rc_handle = rc_handle;
    bsa_add_play.label = app_avk_get_label(); /* Just used to distinguish commands */

    bsa_add_play.scope = scope;
    memcpy(bsa_add_play.uid, uid, sizeof(tAVRC_UID));
    bsa_add_play.uid_counter = uid_counter;

    status = BSA_AvkAddToPlayCmd(&bsa_add_play);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_add_to_play %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_set_abs_vol_rsp
 **
 ** Description      This function sends abs vol response
 **
 ** Returns          None
 **
 *******************************************************************************/
void app_avk_set_abs_vol_rsp(UINT8 volume, UINT8 rc_handle, UINT8 label)
{
    int status;
    tBSA_AVK_SET_ABS_VOLUME_RSP abs_vol;
    BSA_AvkSetAbsVolumeRspInit(&abs_vol);

    abs_vol.label = label;
    abs_vol.rc_handle = rc_handle;
    abs_vol.volume = volume;
    status = BSA_AvkSetAbsVolumeRsp(&abs_vol);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to send app_avk_set_abs_vol_rsp %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_reg_notfn_rsp
 **
 ** Description      This function sends reg notfn response (BSA as TG role in AVK)
 **
 ** Returns          none
 **
 *******************************************************************************/
void app_avk_reg_notfn_rsp(UINT8 volume, UINT8 rc_handle, UINT8 label, UINT8 event, tBTA_AV_CODE code)
{
    int status;
    tBSA_AVK_REG_NOTIF_RSP reg_notf;
    BSA_AvkRegNotifRspInit(&reg_notf);

    reg_notf.reg_notf.param.volume  = volume;
    reg_notf.reg_notf.event_id      = event;
    reg_notf.rc_handle              = rc_handle;
    reg_notf.label                  = label;
    reg_notf.code                   = code;

    status = BSA_AvkRegNotifRsp(&reg_notf);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to send app_avk_reg_notfn_rsp %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_send_get_capabilities
 **
 ** Description      Sample code for attaining capability for events
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_send_get_capabilities(UINT8 rc_handle)
{
    int status;
    tBSA_AVK_VEN_CMD bsa_avk_vendor_cmd;

    /* Send remote control command */
    status = BSA_AvkVendorCmdInit(&bsa_avk_vendor_cmd);

    bsa_avk_vendor_cmd.rc_handle = rc_handle;
    bsa_avk_vendor_cmd.ctype = BSA_AVK_CMD_STATUS;
    bsa_avk_vendor_cmd.data[0] = BSA_AVK_RC_VD_GET_CAPABILITIES;
    bsa_avk_vendor_cmd.data[1] = 0; /* reserved & packet type */
    bsa_avk_vendor_cmd.data[2] = 0; /* length high*/
    bsa_avk_vendor_cmd.data[3] = 1; /* length low*/
    bsa_avk_vendor_cmd.data[4] = 0x03; /* for event*/

    bsa_avk_vendor_cmd.length = 5;
    bsa_avk_vendor_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkVendorCmd(&bsa_avk_vendor_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV Vendor Command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_send_register_notification
 **
 ** Description      Sample code for attaining capability for events
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_send_register_notification(int evt, UINT8 rc_handle)
{
    int status;
    tBSA_AVK_VEN_CMD bsa_avk_vendor_cmd;

    /* Send remote control command */
    status = BSA_AvkVendorCmdInit(&bsa_avk_vendor_cmd);

    bsa_avk_vendor_cmd.rc_handle = rc_handle;
    bsa_avk_vendor_cmd.ctype = BSA_AVK_CMD_NOTIF;
    bsa_avk_vendor_cmd.data[0] = BSA_AVK_RC_VD_REGISTER_NOTIFICATION;
    bsa_avk_vendor_cmd.data[1] = 0; /* reserved & packet type */
    bsa_avk_vendor_cmd.data[2] = 0; /* length high*/
    bsa_avk_vendor_cmd.data[3] = 5; /* length low*/
    bsa_avk_vendor_cmd.data[4] = evt; /* length low*/

    bsa_avk_vendor_cmd.length = 9;
    bsa_avk_vendor_cmd.label = app_avk_get_label(); /* Just used to distinguish commands */

    status = BSA_AvkVendorCmd(&bsa_avk_vendor_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV Vendor Command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_add_connection
 **
 ** Description      This function adds a connection
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_add_connection(BD_ADDR bd_addr)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (bdcmp(app_avk_cb.connections[index].bda_connected, bd_addr) == 0)
        {
            app_avk_cb.connections[index].in_use = TRUE;
            return &app_avk_cb.connections[index];
        }

        if (app_avk_cb.connections[index].in_use == FALSE)
        {
            bdcpy(app_avk_cb.connections[index].bda_connected, bd_addr);
            app_avk_cb.connections[index].in_use = TRUE;
            app_avk_cb.connections[index].index = index;
            return &app_avk_cb.connections[index];
        }
    }
    return NULL;
}

/*******************************************************************************
 **
 ** Function         app_avk_reset_connection
 **
 ** Description      This function resets a connection
 **
 **
 *******************************************************************************/
void app_avk_reset_connection(BD_ADDR bd_addr)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (bdcmp(app_avk_cb.connections[index].bda_connected, bd_addr) == 0)
        {
            app_avk_cb.connections[index].in_use = FALSE;

            app_avk_cb.connections[index].m_uiAddressedPlayer = 0;
            app_avk_cb.connections[index].m_uidCounterAddrPlayer = 0;
            app_avk_cb.connections[index].m_uid_counter = 0;
            app_avk_cb.connections[index].m_bDeviceSupportBrowse = FALSE;
            app_avk_cb.connections[index].m_uiBrowsedPlayer = 0;
            app_avk_cb.connections[index].m_iCurrentFolderItemCount = 0;
            app_avk_cb.connections[index].m_bAbsVolumeSupported = FALSE;
            break;
        }
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_find_connection_by_av_handle
 **
 ** Description      This function finds the connection structure by its handle
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_find_connection_by_av_handle(UINT8 handle)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (app_avk_cb.connections[index].ccb_handle == handle)
            return &app_avk_cb.connections[index];
    }
    return NULL;
}


/*******************************************************************************
 **
 ** Function         app_avk_find_connection_by_rc_handle
 **
 ** Description      This function finds the connection structure by its handle
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_find_connection_by_rc_handle(UINT8 handle)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (app_avk_cb.connections[index].rc_handle == handle)
            return &app_avk_cb.connections[index];
    }
    return NULL;
}


/*******************************************************************************
 **
 ** Function         app_avk_find_connection_by_bd_addr
 **
 ** Description      This function finds the connection structure by its handle
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_find_connection_by_bd_addr(BD_ADDR bd_addr)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if ((bdcmp(app_avk_cb.connections[index].bda_connected, bd_addr) == 0) &&
                (app_avk_cb.connections[index].in_use == TRUE))
            return &app_avk_cb.connections[index];
    }
    return NULL;
}


/*******************************************************************************
 **
 ** Function         app_avk_find_streaming_connection
 **
 ** Description      This function finds the connection structure that is streaming
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_find_streaming_connection()
{
    int index;

    if(pStreamingConn && pStreamingConn->is_streaming_open && pStreamingConn->is_started)
        return pStreamingConn;

    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (app_avk_cb.connections[index].is_streaming_open == TRUE &&
                app_avk_cb.connections[index].is_started == TRUE )
            return &app_avk_cb.connections[index];
    }
    return NULL;
}

/*******************************************************************************
 **
 ** Function         app_avk_num_connections
 **
 ** Description      This function number of connections
 **
 ** Returns          number of connections
 **
 *******************************************************************************/
int app_avk_num_connections()
{
    int iCount = 0;
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if (app_avk_cb.connections[index].in_use == TRUE)
            iCount++;
    }
    return iCount;
}

/*******************************************************************************
 **
 ** Function         app_avk_find_connection_by_index
 **
 ** Description      This function finds the connection structure by its index
 **
 ** Returns          Pointer to the found structure or NULL
 **
 *******************************************************************************/
tAPP_AVK_CONNECTION *app_avk_find_connection_by_index(int index)
{
    if(index < APP_AVK_MAX_CONNECTIONS)
        return &app_avk_cb.connections[index];

    return NULL;
}


/*******************************************************************************
 **
 ** Function         app_avk_pause_other_streams
 **
 ** Description      This function pauses other streams when a new stream start
 **                  is received from device identified by bd_addr
 **
 ** Returns          None
 **
 *******************************************************************************/
void app_avk_pause_other_streams(BD_ADDR bd_addr)
{
    int index;
    for (index = 0; index < APP_AVK_MAX_CONNECTIONS; index++)
    {
        if ((bdcmp(app_avk_cb.connections[index].bda_connected, bd_addr)) &&
                app_avk_cb.connections[index].in_use  &&
                app_avk_cb.connections[index].is_streaming_open)
        {
            app_avk_cb.connections[index].is_started = FALSE;
        }
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_send_delay_report
 **
 ** Description      Sample code to send delay report
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_send_delay_report(UINT16 delay)
{
    tBSA_AVK_DELAY_REPORT report;
    int status;

    APP_DEBUG1("send delay report:%d", delay);

    BSA_AvkDelayReportInit(&report);
    report.delay = delay;
    status = BSA_AvkDelayReport(&report);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send delay report %d", status);
    }
}
