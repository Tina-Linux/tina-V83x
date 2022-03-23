/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awHDCPModule.h
* Description :
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#ifndef AW_HDCP_MODULE_H
#define AW_HDCP_MODULE_H

typedef enum {
    /* Do not change these values (starting with HDCP_STREAM_TYPE_),
     * keep them in sync with header file "DX_Hdcp_Types.h" from Discretix.
     */
    HDCP_STREAM_TYPE_UNKNOWN,
    HDCP_STREAM_TYPE_VIDEO,
    HDCP_STREAM_TYPE_AUDIO,
    HDCP_STREAM_TYPE_INVALUD = 0x7FFFFFFF
} HdcpStreamType;

int32_t HDCP_Init(void **);
void HDCP_Deinit(void *);
uint32_t HDCP_Decrypt(void *, const uint8_t privateData[16],
        const uint8_t *msgIn, uint8_t *msgOut, uint32_t msgLen, int streamType);
#endif//AW_HDCP_MODULE_H
