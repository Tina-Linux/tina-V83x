#ifndef __UVOICE_ASR_MGR_H__
#define __UVOICE_ASR_MGR_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "log.h"
#include "main_app.h"
#include <uvoice_config.h>
#include "media_playback.h"
#include "usr_config.h"
#include "asr_config.h"

#ifdef __cplusplus
extern "c"
#endif


typedef struct asr_mgr_tag *asr_mgr_t;
asr_mgr_t asr_mgr_create();
int asr_mgr_init(asr_mgr_t p);
uint8_t asr_mgr_wait_asrid(asr_mgr_t p);
void asr_mgr_release_wait(asr_mgr_t p);
void asr_mgr_release(asr_mgr_t *p);
void asr_mgr_playback(asr_mgr_t p_mgr, const char *playback_name);
void asr_mgr_set_play_flag(asr_mgr_t p_mgr,int play_flag);
void asr_mgr_change_run_mode(asr_mgr_t p_mgr, int8_t mode);

#ifdef __cplusplus
};
#endif

#endif    //__UVOICE_ASR_MGR_H__ end













