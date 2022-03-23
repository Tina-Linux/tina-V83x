/*
 * =====================================================================================
 *
 *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 *      All rights reserved.
 *
 *       Filename:  omx_common.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018/08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SWC-PPD
 *      Company: Allwinner Technology Co. Ltd.
 *
 *       History :
 *        Author : AL3
 *        Date   : 2013/05/05
 *    Comment : complete the design code
 *
 * =====================================================================================
 */

#ifndef __OMX_COMMON_H__
#define __OMX_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif
typedef enum OMX_VENC_COMMANDTYPE
{
    OMX_Venc_Cmd_Open,
    OMX_Venc_Cmd_Close,
    OMX_Venc_Cmd_Stop,
    OMX_Venc_Cmd_Enc_Idle,
    OMX_Venc_Cmd_ChangeBitrate,
    OMX_Venc_Cmd_ChangeColorFormat,
    OMX_Venc_Cmd_RequestIDRFrame,
    OMX_Venc_Cmd_ChangeFramerate,
    OMX_Venc_Cmd_SetAvcIntraPeriod,
    OMX_Venc_Cmd_SetFixQP,
} OMX_VENC_COMMANDTYPE;
typedef enum ThrCmdType
{
    MAIN_THREAD_CMD_SET_STATE    = 0,
    MAIN_THREAD_CMD_FLUSH        = 1,
    MAIN_THREAD_CMD_STOP_PORT    = 2,
    MAIN_THREAD_CMD_RESTART_PORT = 3,
    MAIN_THREAD_CMD_MARK_BUF     = 4,
    MAIN_THREAD_CMD_STOP         = 5,
    MAIN_THREAD_CMD_FILL_BUF     = 6,
    MAIN_THREAD_CMD_EMPTY_BUF    = 7,
} ThrCmdType;

typedef enum OMX_VENC_INPUTBUFFER_STEP
{
    OMX_VENC_STEP_GET_INPUTBUFFER,
    OMX_VENC_STEP_GET_ALLOCBUFFER,
    OMX_VENC_STEP_ADD_BUFFER_TO_ENC,
} OMX_VENC_INPUTBUFFER_STEP;

#ifdef __cplusplus
}
#endif
#endif // __OMX_COMMON_H__
