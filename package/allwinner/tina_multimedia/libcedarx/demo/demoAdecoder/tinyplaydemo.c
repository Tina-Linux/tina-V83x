/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tinyplaydemo.c
 * Description : demoAdecoder
 * History :
 *
 */

#include "tinyplaydemo.h"
#include "demo_utils.h"
#include <fcntl.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG "Tinyplaydemo"
#endif
#define DEMO_DEBUG 1

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

typedef struct name_map_t
{
    char name_linux[32];
    char name_android[32];
}name_map;

#define AUDIO_MAP_CNT   16
#define AUDIO_NAME_CODEC    "AUDIO_CODEC"
#define AUDIO_NAME_HDMI     "AUDIO_HDMI"
#define AUDIO_NAME_SPDIF    "AUDIO_SPDIF"
#define AUDIO_NAME_I2S      "AUDIO_I2S"
#define AUDIO_NAME_RT3261 "AUDIO_CODEC_RT"
#define AUDIO_NAME_RT3261_RAW "RT3261_RAW"
#define AUDIO_NAME_MIX "AUDIO_MIX"
#define AUDIO_NAME_BT       "AUDIO_BT"

static name_map audio_name_map[AUDIO_MAP_CNT] =
{
    {"snddaudio",       AUDIO_NAME_CODEC},//A80
    {"audiocodec",      AUDIO_NAME_CODEC},//A31,A20
    {"sndacx00codec",   AUDIO_NAME_CODEC},//H6
    {"sndhdmi",         AUDIO_NAME_HDMI},
    {"rt3261aif2",      AUDIO_NAME_RT3261_RAW},
    {"sndspdif",        AUDIO_NAME_SPDIF},
    {"rt3261",          AUDIO_NAME_RT3261},
    {"snddaudio2",      AUDIO_NAME_BT},
    {"sndahub",         AUDIO_NAME_MIX},
};

enum SND_AUIDO_RAW_DATA_TYPE
{
    SND_AUDIO_RAW_DATA_UNKOWN = 0,
    SND_AUDIO_RAW_DATA_PCM = 1,
    SND_AUDIO_RAW_DATA_AC3 = 2,
    SND_AUDIO_RAW_DATA_MPEG1 = 3,
    SND_AUDIO_RAW_DATA_MP3 = 4,
    SND_AUDIO_RAW_DATA_MPEG2 = 5,
    SND_AUDIO_RAW_DATA_AAC = 6,
    SND_AUDIO_RAW_DATA_DTS = 7,
    SND_AUDIO_RAW_DATA_ATRAC = 8,
    SND_AUDIO_RAW_DATA_ONE_BIT_AUDIO = 9,
    SND_AUDIO_RAW_DATA_DOLBY_DIGITAL_PLUS = 10,
    SND_AUDIO_RAW_DATA_DTS_HD = 11,
    SND_AUDIO_RAW_DATA_MAT = 12,
    SND_AUDIO_RAW_DATA_DST = 13,
    SND_AUDIO_RAW_DATA_WMAPRO = 14
};

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

static int find_name_map(AudioDsp *adev, char * in, char * out)
{
    int index = 0;
    CDX_UNUSE(adev);
    if (in == 0 || out == 0)
    {
        demo_loge("error params");
        return -1;
    }

    for (; index < AUDIO_MAP_CNT; index++)
    {
        if (strlen(audio_name_map[index].name_linux) == 0)
        {

            //sprintf(out, "AUDIO_USB%d", adev->usb_audio_cnt++);
            sprintf(out, "AUDIO_USB_%s", in);
            strcpy(audio_name_map[index].name_linux, in);
            strcpy(audio_name_map[index].name_android, out);
            demo_logd("linux name = %s, android name = %s",
                audio_name_map[index].name_linux,
                audio_name_map[index].name_android);
            return 0;
        }

        if (!strcmp(in, audio_name_map[index].name_linux))
        {
            strcpy(out, audio_name_map[index].name_android);
            demo_logd("linux name = %s, android name = %s",
                audio_name_map[index].name_linux,
                audio_name_map[index].name_android);
            return 0;
        }
    }

    return 0;
}

static int do_init_audio_card(AudioDsp *adev, int card)
{
    int ret = -1;
    int fd = 0;
    char * snd_path = "/sys/class/sound";
    char snd_card[128], snd_node[128];
    char snd_id[32], snd_name[32];

    memset(snd_card, 0, sizeof(snd_card));
    memset(snd_node, 0, sizeof(snd_node));
    memset(snd_id, 0, sizeof(snd_id));
    memset(snd_name, 0, sizeof(snd_name));

    sprintf(snd_card, "%s/card%d", snd_path, card);
    ret = access(snd_card, F_OK);
    if(ret == 0)
    {
        // id / name
        sprintf(snd_node, "%s/card%d/id", snd_path, card);
        demo_logd("1... read card %s/card%d/id",snd_path, card);
        fd = open(snd_node, O_RDONLY);
        if (fd > 0)
        {
            ret = read(fd, snd_id, sizeof(snd_id));
            if (ret > 0)
            {
                snd_id[ret - 1] = '\0';
                demo_logd("2.... %s, %s, card: %d", snd_node, snd_id, card);
            }
            close(fd);
        }
        else
        {
            demo_logd("3.... fd fail ....%s, %s, len: %d", snd_node, snd_id, card);
            return -1;
        }
        strcpy(adev->dev_manager[card].card_id, snd_id);
        find_name_map(adev, snd_id, snd_name);
        strcpy(adev->dev_manager[card].name, snd_name);

        adev->dev_manager[card].card = card;
        adev->dev_manager[card].device = 0;
        adev->dev_manager[card].flag_exist = true;

        // playback device
        sprintf(snd_node, "%s/card%d/pcmC%dD0p", snd_path, card, card);
        ret = access(snd_node, F_OK);
        if(ret == 0)
        {
            // there is a playback device
            adev->dev_manager[card].flag_out = AUDIO_OUT;
            adev->dev_manager[card].flag_out_active = 0;
        }

        // capture device
        sprintf(snd_node, "%s/card%d/pcmC%dD0c", snd_path, card, card);
        ret = access(snd_node, F_OK);
        if(ret == 0)
        {
            // there is a capture device
            adev->dev_manager[card].flag_in = AUDIO_IN;
            adev->dev_manager[card].flag_in_active = 0;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

static void init_audio_devices(AudioDsp *adev)
{
    int card = 0;

    memset(adev->dev_manager, 0, sizeof(adev->dev_manager));

    for (card = 0; card < MAX_AUDIO_DEVICES; card++)
    {
        if (do_init_audio_card(adev, card) == 0)
        {
            // break;
            demo_logd("card: %d, name: %s, capture: %d, playback: %d",
                card, adev->dev_manager[card].name,
                adev->dev_manager[card].flag_in == AUDIO_IN,
                adev->dev_manager[card].flag_out == AUDIO_OUT);
        }
    }
}

static void getCardNumbyName(AudioDsp *adev, char *name, int *card)
{
    int index;
    for (index = 0; index < MAX_AUDIO_DEVICES; index++) {
        if (!strcmp(adev->dev_manager[index].name, name)) {
            *card = index;
            demo_logd("getCardNumbyName: name = %s , card = %d",name, index);
            demo_logd("NAME = %s",adev->dev_manager[index].name);
            return;
        }
    }

    for (index = 0; index < MAX_AUDIO_DEVICES; index++) {
        if (!strncmp(adev->dev_manager[index].name, "AUDIO_USB", 9)
        && !strncmp(name, "AUDIO_USB", 9)
        && adev->dev_manager[index].flag_exist) {
            *card = index;
            demo_logd("USBNAME = %s",adev->dev_manager[index].name);
            demo_logd("getCardNumbyName: name = %s , card = %d",name, index);
            break;
        }
    }
    demo_logd("card 0 name = %s",adev->dev_manager[0].name);
    if (index == MAX_AUDIO_DEVICES) {
    demo_logd("BT");
        *card = -1;
        demo_loge("%s card does not exist",name);
    }
}

#ifdef H6_AUDIO_HUB
void enble_hub_hdmi(struct mixer *mixer)
{
    demo_logd("enble_hub_hdmi");
    struct mixer_ctl *ctl;
    //i2s1->APBIF_TXDIF0
    ctl = mixer_get_ctl_by_name(mixer, "I2S1 Src Select");
    mixer_ctl_set_value(ctl, 0, 1);
    //enble i2s1
    ctl = mixer_get_ctl_by_name(mixer, "I2S1OUT Switch");
    mixer_ctl_set_value(ctl, 0, 1);
}
void close_hub_hdmi(struct mixer *mixer)
{
    demo_logd("close_hub_hdmi");
    struct mixer_ctl *ctl;
    //i2s1->APBIF_TXDIF0
    ctl = mixer_get_ctl_by_name(mixer, "I2S1 Src Select");
    mixer_ctl_set_value(ctl, 0, 0);
    //close i2s1
    ctl = mixer_get_ctl_by_name(mixer, "I2S1OUT Switch");
    mixer_ctl_set_value(ctl, 0, 0);
}

static int     set_ahub_rontiue(int ahub_card_id, const char* name, int on_off)
{
    struct mixer *audio_hub_mixer = mixer_open(ahub_card_id);
    if(!audio_hub_mixer)
    {
        demo_loge("Unable to open the audio_hub_mixer, aborting...");
        return -1;
    }
    //Now only fix the HDMI scenoris
    if(on_off != 0)//enable...
    {
        if(!strcmp(name, AUDIO_NAME_HDMI))
        {
            demo_logi("ahub enable hdmi hub...");
            enble_hub_hdmi(audio_hub_mixer);
        }
    }
    else//shut down...
    {
        if(!strcmp(name, AUDIO_NAME_HDMI))
        {
            demo_logi("ahub shut down hdmi hub...");
            close_hub_hdmi(audio_hub_mixer);
        }
    }

    mixer_close(audio_hub_mixer);
    audio_hub_mixer = NULL;
    return 0;
}
#endif

static int set_raw_flag(AudioDsp* dsp, int card, int raw_flag)
{
    demo_logi("set_raw_flag(card=%d, raw_flag=%d)", card, raw_flag);
    struct mixer *mixer = mixer_open(card);
    if (!mixer) {
        demo_loge("Unable to open the mixer, aborting...");
        return -1;
    }
    const char *control_name = (card == dsp->cardHDMI) ? "hdmi audio format Function":
                                (card == dsp->cardMIX) ? "ahub audio format Function":
                                                         "spdif audio format Function";
    demo_logi("control_name : %s, card : %d", control_name, card);
    const char *control_value = (raw_flag==SND_AUDIO_RAW_DATA_AC3) ? "AC3" :
                       (raw_flag==SND_AUDIO_RAW_DATA_DOLBY_DIGITAL_PLUS) ? "DOLBY_DIGITAL_PLUS":
                       (raw_flag==SND_AUDIO_RAW_DATA_MAT) ? "MAT":
                       (raw_flag==SND_AUDIO_RAW_DATA_DTS) ? "DTS" : "pcm";
    struct mixer_ctl *audio_format = mixer_get_ctl_by_name(mixer, control_name);
    if (audio_format)
        mixer_ctl_set_enum_by_string(audio_format, control_value);
    mixer_close(mixer);
    return 0;
}

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        demo_loge("%s is %u%s, device only supports >= %u%s", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        demo_loge("%s is %u%s, device only supports <= %u%s", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

static int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                        unsigned int rate, unsigned int bits, unsigned int period_size,
                        unsigned int period_count)
{
    struct pcm_params *params;
    int can_play;
    demo_loge("line : %d, card:%u, device:%u",__LINE__, card, device);
    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        demo_loge("Unable to open PCM device %u", device);
        return 0;
    }
    demo_loge("line : %d, card:%u, device:%u",__LINE__, card, device);

    can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", "Hz");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", "Hz");

    pcm_params_free(params);

    return can_play;
}

int DspWrite(AudioDsp* dsp, void *pPcmData, int nPcmDataLen)
{
    void* buf_conv = NULL;
    int   buf_conv_len = 0;
    int   samples = 0;

    if(!dsp)
        return -1;

    if(!dsp->start)
    {
        dsp->config.channels = dsp->ch;
        dsp->config.rate = dsp->spr;
        dsp->config.period_size = dsp->period_size;
        dsp->config.period_count = dsp->period_count;
        dsp->config.raw_flag = dsp->raw_flag;
        if (dsp->bps == 32)
            dsp->config.format = PCM_FORMAT_S32_LE;
        else if (dsp->bps == 24)
            dsp->config.format = PCM_FORMAT_S24_LE;
        else
            dsp->config.format = PCM_FORMAT_S16_LE;

        demo_logi("dsp->bps : %d", dsp->bps);
        dsp->config.start_threshold = 0;
        dsp->config.stop_threshold = 0;
        dsp->config.silence_threshold = 0;

        if(set_raw_flag(dsp, dsp->cardHDMI, dsp->raw_flag) < 0){
            demo_loge("can not set raw flag...");
            return -1;
        }
        demo_logi("hdmi , raw_flag : %d, ch : %d, spr : %d, period_size : %d, period_count : %d",
            dsp->config.raw_flag, dsp->config.channels , dsp->config.rate, dsp->config.period_size,
            dsp->config.period_count);

#ifdef H6_AUDIO_HUB
        dsp->hubconfig = dsp->config;

        if(set_raw_flag(dsp, dsp->cardMIX, dsp->raw_flag) < 0){
            demo_loge("can not set raw flag...");
            return -1;
        }
        demo_logi("ahub , raw_flag : %d, ch : %d, spr : %d, period_size : %d, period_count : %d",
                    dsp->hubconfig.raw_flag, dsp->hubconfig.channels , dsp->hubconfig.rate,
                    dsp->hubconfig.period_size, dsp->hubconfig.period_count);

        set_ahub_rontiue(dsp->cardMIX, AUDIO_NAME_HDMI, 1);

        dsp->pcmhub = pcm_open(dsp->cardMIX, dsp->device, PCM_OUT, &dsp->hubconfig);
        if (!dsp->pcmhub || !pcm_is_ready(dsp->pcmhub)) {
            demo_loge("Unable to open PCM device %d (%s)\n",
            dsp->device, pcm_get_error(dsp->pcmhub));
            return -1;
        }
#else
        dsp->pcm = pcm_open(dsp->cardHDMI, dsp->device, PCM_OUT, &dsp->config);
        if (!dsp->pcm || !pcm_is_ready(dsp->pcm)) {
            demo_loge("Unable to open PCM device %u (%s)\n",
            dsp->device, pcm_get_error(dsp->pcm));
            return -1;
        }
#endif
        dsp->start = 1;
    }

    if(nPcmDataLen > 0)
    {
        if (dsp->bps == 24)
        {
            int idx = 0, jdx = 0;
            char* dst = NULL;
            char* src = NULL;
            samples = nPcmDataLen * 8/ dsp->ch / dsp->bps;
            buf_conv_len = nPcmDataLen*4/3;

            DEMO_VERBOS_SAFE_MALLOC(buf_conv, buf_conv_len,
                demo_loge("Error no mem for conventer buffer...");\
                return -1;
            )

            dst = (char*)buf_conv;
            for(idx = 0; idx < dsp->ch; idx++)
            {
                dst = ((char*)buf_conv) + 4*idx;
                src = ((char*)pPcmData) + 3*idx;
                for(jdx = 0; jdx < samples; jdx++){
                    dst[0] = src[0]&0xff;
                    dst[1] = src[1]&0xff;
                    dst[2] = src[2]&0xff;
                    dst[3] = (dst[2]&0x80)?0xff:0x00;
                    dst += 4*dsp->ch;
                    src += 3*dsp->ch;
                }
            }
#ifdef H6_AUDIO_HUB
            if (pcm_write(dsp->pcmhub, buf_conv, buf_conv_len) < 0)
#else
            if (pcm_write(dsp->pcm, buf_conv, buf_conv_len) < 0)
#endif
            {
                demo_loge("Error playing sample...");
                DEMO_SAFE_FREE(buf_conv, "buf_conv", 0)
                return -1;
            }
            DEMO_SAFE_FREE(buf_conv, "buf_conv", 0)
        }
        else
        {
#ifdef H6_AUDIO_HUB
            if (pcm_write(dsp->pcmhub, pPcmData, nPcmDataLen) < 0)
#else
            if (pcm_write(dsp->pcm, pPcmData, nPcmDataLen) < 0)
#endif
            {
                demo_loge("Error playing sample...");
                return -1;
            }
        }
    }
    else
        demo_loge("Invaild write len : %d", nPcmDataLen);
    return nPcmDataLen;
}

int DspStop(AudioDsp* dsp)
{
    if(!dsp)
        return -1;

    if(dsp->start)
    {
#ifdef H6_AUDIO_HUB
        if(dsp->pcmhub)
        {
            pcm_close(dsp->pcmhub);
            dsp->pcmhub = NULL;
            set_ahub_rontiue(dsp->cardMIX, AUDIO_NAME_HDMI, 0);
            demo_logi("Dsp pcmhub device closed ...");
        }
#else
        if(dsp->pcm)
        {
            pcm_close(dsp->pcm);
            dsp->pcm = NULL;
            demo_logi("Dsp pcm device closed ...");
        }
#endif
        dsp->start = 0;
    }
    else
    {
        demo_logd("dsp stop while unactive...");
    }
    return 0;
}

void DspWaitForDevConsume(int waitms)
{
    usleep(waitms * 1000);
}

extern void NotifyHalDirect(int on_off);

AudioDsp* CreateAudioDsp()
{
    AudioDsp* pMem = NULL;
    DEMO_VERBOS_SAFE_MALLOC(pMem, sizeof(AudioDsp),
        demo_loge("CreateAudioDsp fail for no mem...");\
        return NULL;
    );
    NotifyHalDirect(1);
    init_audio_devices(pMem);
    getCardNumbyName(pMem, AUDIO_NAME_CODEC, &pMem->cardCODEC);
    getCardNumbyName(pMem, AUDIO_NAME_HDMI, &pMem->cardHDMI);
    getCardNumbyName(pMem, AUDIO_NAME_SPDIF, &pMem->cardSPDIF);
    getCardNumbyName(pMem, AUDIO_NAME_BT, &pMem->cardBT);
    getCardNumbyName(pMem, AUDIO_NAME_MIX, &pMem->cardMIX);
    demo_logi("cardCODEC : %d, cardHDMI: %d, cardSPDIF: %d, cardBT: %d, cardMIX: %d",
            pMem->cardCODEC, pMem->cardHDMI, pMem->cardSPDIF, pMem->cardBT, pMem->cardMIX);
    return pMem;
}

void DeleteAudioDsp(AudioDsp* dsp)
{
    AudioDsp* pMem = NULL;
    pMem = dsp;

    if(!pMem)
    {
        demo_loge("DeleteAudioDsp strange for null ptr...");
        return;
    }
    DspStop(pMem);
    NotifyHalDirect(0);
    DEMO_SAFE_FREE(pMem, "AudioDsp", 1)
}
