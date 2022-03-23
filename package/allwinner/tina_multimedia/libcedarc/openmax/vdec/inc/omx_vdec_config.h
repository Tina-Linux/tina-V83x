/*
 * =====================================================================================
 *
 *       Filename:  omx_vdec_config.h
 *
 *    Description: vdec config
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company:
 *
 * =====================================================================================
 */
#ifndef __OMX_VDEC_CONFIG_H__
#define __OMX_VDEC_CONFIG_H__

#include "omx_pub_def.h"

/*
*   E N N A B L E
*/

#define ENABLE_SCALEDOWN_WHEN_RESOLUTION_MOER_THAN_1080P 0

#define ENABLE_SAVE_ES_DATA                              0

#define ENABLE_SAVE_PICTURE                              0

#define ENABLE_SHOW_BUFINFO_STATUS                       0

#define ENABLE_STATISTICS_TIME                           0

/*
 *  D E C L A R A T I O N S
 */
#define ANDROID_SHMEM_ALIGN                   32
#define ALIGN_SIZE_32                         32

#define CALLING_PROCESS_MXPLAYER              "com.mxtech.videoplayer.pro"

#define WIDTH_DEFAULT                         176
#define HEIGHT_DEFAULT                        144
#define COLOR_FORMAT_DEFAULT                  OMX_COLOR_FormatYVU420Planar

//This value restricts the submit thread to continue
#define MAX_INOUT_DIFF_NUM                    16

#define DISPLAY_HOLH_BUFFER_NUM_DEFAULT       4

#define OMX_SPEC_VERSION                      0x00000101

#define OMX_NOPORT                            0xFFFFFFFE

#define NUM_IN_BUFFERS                        2              // Input Buffers
#define NUM_IN_BUFFERS_SECURE                 1              // Input Buffers(secure)
#define NUM_OUT_BUFFERS                       2              // Output Buffers
#define MAX_NUM_OUT_BUFFERS                   4             // max Output Buffers
#define MAX_NUM_OUT_BUFFERS_SECURE            4              // max Output Buffers

// Timeout value in milisecond
#define OMX_TIMEOUT                           10
// Count of Maximum number of times the component can time out
#define OMX_MAX_TIMEOUTS                      500

// 6 MB, we meet the 3.4 MB streamSize case
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE       (6*1024*1024)
// 1MB, (256 kB is not enought)
#define OMX_VIDEO_DEC_INPUT_BUFFER_SIZE_SECURE (1*1024*1024)

// be used in the zero copy process
#define OMX_MAX_VIDEO_BUFFER_NUM              32

#define CODEC_SPECIFIC_DATA_LENGTH            20*1024

/*
*  T Y P E S
*/
typedef enum
{
    AW_OMX_MSG_SET_STATE,
    AW_OMX_MSG_STOP_PORT,
    AW_OMX_MSG_RESTART_PORT,
    AW_OMX_MSG_MARK_BUF,
    AW_OMX_MSG_FILL_BUF,
    AW_OMX_MSG_EMPTY_BUF,
    AW_OMX_MSG_FLUSH,
    AW_OMX_MSG_START,
    AW_OMX_MSG_QUIT,
}AW_OMX_MSG_TYPE;

typedef enum
{
    AW_OMX_CB_EMPTY_BUFFER_DONE,
    AW_OMX_CB_FILL_BUFFER_DONE,
    AW_OMX_CB_NOTIFY_EOS,
    AW_OMX_CB_PORT_CHANGE,
    AW_OMX_CB_EVENT_ERROR,
    AW_OMX_CB_SUSPEND_DECODE,
    AW_OMX_CB_SUSPEND_DRAIN,
    AW_OMX_CB_RESUME_DRAIN,
    AW_OMX_CB_NOTIFY_RECT,
    AW_OMX_CB_FLUSH_ALL_PIC,
    AW_OMX_CB_FININSH_REOPEN_VE,
    AW_OMX_CB_CONTINUE_SUBMIT,

}AW_OMX_CALLBACK_MSG;

typedef enum OutBufferStatus {
    OWNED_BY_NONE     = 0,
    OWNED_BY_RENDER   = 1,
    OWNED_BY_DECODER  = 2,
    OWNED_BY_US       = 3,
}OutBufferStatus;


/*
*    INLINE FUNCTIONS
*/
static const inline char *statusToString(OutBufferStatus s) {
    switch (s) {
        STRINGIFY(OWNED_BY_US);
        STRINGIFY(OWNED_BY_DECODER);
        STRINGIFY(OWNED_BY_RENDER);
        STRINGIFY(OWNED_BY_NONE);
        default:    return "?";
    }
}

static const inline char *cmdToString(AW_OMX_MSG_TYPE cmd)
{
    switch (cmd) {
        STRINGIFY(AW_OMX_MSG_SET_STATE);
        STRINGIFY(AW_OMX_MSG_STOP_PORT);
        STRINGIFY(AW_OMX_MSG_RESTART_PORT);
        STRINGIFY(AW_OMX_MSG_MARK_BUF);
        STRINGIFY(AW_OMX_MSG_FILL_BUF);
        STRINGIFY(AW_OMX_MSG_EMPTY_BUF);
        STRINGIFY(AW_OMX_MSG_FLUSH);
        STRINGIFY(AW_OMX_MSG_START);
        STRINGIFY(AW_OMX_MSG_QUIT);
        default:    return "?";
    }
}

#endif
