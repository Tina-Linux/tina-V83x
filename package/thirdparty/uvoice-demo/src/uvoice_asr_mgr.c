#include "uvoice_asr_mgr.h"

typedef struct asr_mgr_tag {
    struct data_config *uv_conf;
} *asr_mgr_t;

asr_mgr_t asr_mgr_create()
{
    asr_mgr_t mgr;
    mgr = (asr_mgr_t)malloc(sizeof(struct asr_mgr_tag));
    if(mgr == NULL) {
        LOGE("mgr malloc failed.");
        goto INIT_ERROR;
    }
    memset(mgr, 0, sizeof(struct asr_mgr_tag));

    mgr->uv_conf = malloc(sizeof(struct data_config));
    if(mgr->uv_conf == NULL) {
        LOGE("mgr->uv_conf malloc failed.");
        goto INIT_ERROR;
    }

    return mgr;

INIT_ERROR:
    if(mgr->uv_conf) {
        LOGE("mgr->uv_conf free.");
        free(mgr->uv_conf);
    }

    if(mgr) {
        LOGE("mgr free.");
        free(mgr);
    }
    return NULL;
}

int asr_mgr_init(asr_mgr_t p_mgr)
{
    int ret = -1;
    if(!p_mgr) {
        LOGE("p_mgr malloc error!");
        return -1;
    }
    int data_ctl = 1;

#if 0
    int all_buffers_size = 32000*2*16*3;
#else
    int all_buffers_size = 32000 * 16;
#endif
    int8_t wakeup_timeout_refresh = 1;
    int8_t ns_mode_set = 1;
    if(p_mgr->uv_conf == NULL) {
        p_mgr->uv_conf = malloc(sizeof(struct data_config));
    }
    p_mgr->uv_conf->card = 0; //R6:1, R328:0
    p_mgr->uv_conf->device = 0;
    p_mgr->uv_conf->channels = 2;
    p_mgr->uv_conf->rate = 16000;
    p_mgr->uv_conf->period_size = 1024;
    p_mgr->uv_conf->period_count = 4;
    p_mgr->uv_conf->capture_time = UINT_MAX;
    p_mgr->uv_conf->format = PCM_FORMAT_S16_LE;
    p_mgr->uv_conf->play_flag = 0;
    p_mgr->uv_conf->file = NULL;
    p_mgr->uv_conf->end_timer = WAKEUP_TIMEOUT_CONFIG;
    p_mgr->uv_conf->run_mode = 0;	//0:nomal  1:factory mode
    p_mgr->uv_conf->refresh_timer = 1;
    p_mgr->uv_conf->ns_mode = 1;

    refresh_wakeup_timer(p_mgr->uv_conf, wakeup_timeout_refresh);
    ns_flag_set(p_mgr->uv_conf,  ns_mode_set);

    ret = uvoice_init(p_mgr->uv_conf, all_buffers_size);
    if (ret != 0) {
        LOGE("uvoice_init [ret = %d]", ret);
    }

    ret = creat_record(p_mgr->uv_conf);
    if (ret != 0) {
        LOGE("create asr record thread [ret = %d]", ret);
    }

    if (data_ctl == 1) {
        ret = creat_uvoice_handle(p_mgr->uv_conf);
        if (ret != 0) {
            LOGE("create asr handle thread [ret = %d]", ret);
        }
    } else {
        int8_t *record_buf;
        record_buf = get_data(p_mgr->uv_conf);
    }
    LOGI(">==== Welcome to vendor voice asistant =====<");
    return ret;
}

uint8_t asr_mgr_wait_asrid(asr_mgr_t p_mgr)
{
    uint8_t asr_id = 0xff;
    if(!p_mgr) {
        return asr_id;
    }
    pthread_mutex_lock(&p_mgr->uv_conf->player_queue->play_mutex);
    pthread_cond_wait(&p_mgr->uv_conf->player_queue->player, &p_mgr->uv_conf->player_queue->play_mutex);
    asr_id = p_mgr->uv_conf->player_queue->data[0];
    pthread_mutex_unlock(&p_mgr->uv_conf->player_queue->play_mutex);

    LOGD("asr recognize id = %d", asr_id);
    return asr_id;
}

void asr_mgr_release_wait(asr_mgr_t p_mgr)
{
    if(!p_mgr) {
        return;
    }
    pthread_mutex_lock(&p_mgr->uv_conf->player_queue->play_mutex);
    pthread_cond_signal(&p_mgr->uv_conf->player_queue->player);
    pthread_mutex_unlock(&p_mgr->uv_conf->player_queue->play_mutex);
}

void asr_mgr_release(asr_mgr_t *p_mgr)
{
    if(p_mgr != NULL && *p_mgr != NULL) {
        LOGI("asr destory all start...");
        destory_all((*p_mgr)->uv_conf);
        LOGI("asr destory all success!");

        free(*p_mgr);
        *p_mgr = NULL;
    }
}

void asr_mgr_set_play_flag(asr_mgr_t p_mgr,int play_flag)
{
    if(!p_mgr) {
        return;
    }
    p_mgr->uv_conf->play_flag = play_flag;
}

void asr_mgr_playback(asr_mgr_t p_mgr, const char *playback_name)
{
    if(!p_mgr) {
        return;
    }
    playback((char *)playback_name, p_mgr->uv_conf);
}

void asr_mgr_change_run_mode(asr_mgr_t p_mgr, int8_t mode)
{
    int ret = -1;
    if(!p_mgr) {
        return;
    }
    //ret =
    change_run_mode(p_mgr->uv_conf, mode);
#if 0
    if (ret != 0) {
        log_e("change_run_mode fail\n");
    }
#endif
}

int get_music_name(uint16_t asr_id, char music_name[128])
{
    LOGD("asr_id:%d", asr_id);

    //const char *m_name = music_name[asr_id];
    //if(m_name == NULL) {
    //    LOGE("not find");
    //    return -1;
    //}
    //snprintf(music_name, 128, "%s/%s", music_path, m_name);
    strcpy(music_name, (char*)notice_music_name[asr_id]);

    return 0;
}






