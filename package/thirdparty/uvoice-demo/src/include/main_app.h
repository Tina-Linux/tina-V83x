#ifndef __MAIN_APP_H__
#define __MAIN_APP_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "dev_msg_frame_handle.h"
#include "dev_msg_protocol_parse.h"
#include "app_msg.h"
#include "log.h"
#include "main_app.h"
#include <uvoice_config.h>
#include "app_uart.h"
#include <pthread.h>
#include "uvoice_asr_mgr.h"
#include "usr_config.h"
#include "demo_show.h"
#include "asr_config.h"
/*
* asr config
* enable: add asr
* disable: rm asr
*/

#define ASR_ENABLE
#define PLAYBACK_ENABLE


pthread_t    voiceplaybackThread;

#endif













