#ifndef _USER_COMMON_H_
#define _USER_COMMON_H_
#include "mcrc_speech_user_intf.h"
#include "tts_speaker.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//add by 926 @20190605
/**
 * \brief show configs
 * \param pcm_params
 * \param func/line
 * \param enable:0,disable ; 1,enable show
 * \return null
 */

void show_mcrc_speech_user_configs(struct mcrc_speech_user *user, char *func, uint32_t line, uint8_t enable)
{
        if (enable == 0x01) {
                printf("/--------------------------------/\n");
                printf("func: mcrc_speech_user_config user\n");
                printf(" * func:(%s, %d)\n", func, line);
                printf(" * user.tid = %#x\n", user->tid);
                printf(" * user.running = %#x\n", user->running);
                //printf(" * user.kmsg = %#x\n", user->msg);
                printf(" * user.config = %s\n", user->config);
                printf(" * user.is_mute = %d\n", user->is_mute);
                printf(" * user.keep_silense = %d\n", user->keep_silence);
                printf(" * user.is_online_process = %d\n", user->is_online_process);
                printf(" * user.msui_event_cb = %d\n", user->_handler);
                printf(" * user.fwver = %s\n", user->fwver);
                printf("/--------------------------------/\n");
        }
        return;
}
//end

void show_speech_state_cell_configs(struct speech_state_cell *cell, char *func, uint32_t line, uint8_t enable)
{
        if (enable == 0x01) {
                printf("/--------------------------------/\n");
                printf("func: %s\n", __func__);
                printf(" * func:(%s, %d)\n", func, line);
		printf("state: idle-0, listen-1, think-2, speak-3\n");
                printf(" * cell.prev_state = %d\n", cell->prev_state);
                printf(" * cell.now_state = %d\n", cell->now_state);
                printf("/--------------------------------/\n");
        }
        return;
}


void show_tts_speaker_configs(struct tts_speaker *speaker, char *func, uint32_t line, uint8_t enable)
{
        if (enable == 0x01) {
                printf("/--------------------------------/\n");
                printf("func: %s\n", __func__);
                printf(" * func:(%s, %d)\n", func, line);
                printf(" * speaker.tid = %#x\n", speaker->tid);
                printf(" * speaker.st_tid = %#x\n", speaker->st_tid);
                printf(" * speaker.running = %#x\n", speaker->running);
                printf(" * speaker.st_running = %#x\n", speaker->st_running);
                printf(" * speaker.kmsg = %#x\n", speaker->msg);
		show_mcrc_speech_user_configs(speaker->msui, __func__, __LINE__, 0x01);
		show_speech_state_cell_configs(speaker->cell, __func__, __LINE__, 0x01);
                //printf(" * speaker.msui/user = %s\n", speaker->msui);
                printf(" * speaker.greet_aplayer = %#x\n", speaker->greet_aplayer);
                printf(" * speaker.aplayaer = %#x\n", speaker->aplayer);
                printf(" * speaker.stream_aplayer = %#x\n", speaker->stream_aplayer);
                printf(" * speaker.greet_aplayer_ev = %d\n", speaker->greet_aplayer_ev);
                printf(" * speaker.stream_aplayer_ev = %d\n", speaker->stream_aplayer_ev);
                printf(" * speaker.aplayer_ev = %d\n", speaker->aplayer_ev);
                printf(" * speaker.greet_aplayer_amp = %f\n", speaker->greet_aplayer_amp);
                printf(" * speaker.stream_aplayer_amp = %f\n", speaker->stream_aplayer_amp);
                printf(" * speaker.aplayer_amp = %f\n", speaker->aplayer_amp);
                printf(" * speaker.cur_playing_data = %s\n", speaker->cur_playing_data);
                printf(" * speaker.saved_recipe = %s\n", speaker->saved_recipe);
                printf(" * speaker.cur_spk_state = %d\n", speaker->cur_spk_state);
                printf(" * speaker.pause_state = %d\n", speaker->pause_state);
                printf(" * speaker.url = %s\n", speaker->url);
                printf("/--------------------------------/\n");
        }
        return;
}
//end
#endif

