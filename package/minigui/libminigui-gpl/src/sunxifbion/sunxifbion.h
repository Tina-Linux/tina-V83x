/*
 *   This file is part of MiniGUI, a mature cross-platform windowing
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 *
 *   Copyright (C) 2002~2018, Beijing FMSoft Technologies Co., Ltd.
 *   Copyright (C) 1998~2002, WEI Yongming
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Or,
 *
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 *
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 *
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/en/about/licensing-policy/>.
 */

#ifndef _GAL_sunxifbion_h
#define _GAL_sunxifbion_h

#ifdef _MGGAL_SUNXIFBION

#include <ion_mem_alloc.h>
#include <videoOutPort.h>
#include "sysvideo.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	GAL_VideoDevice *this

/* Private display data */

struct GAL_PrivateVideoData {
    int w, h;
    int flipPage; /* 0~1 */
    int fbNum;
    int bufLen;
    int dirty; /* true for dirty, and should reset to false after refreshing the dirty area */
    void *pVirBuf[3];
    void *pPhyBuf[3];
    pthread_t updateTh;
    renderBuf rBuf;
    videoParam vparam;
    dispOutPort *mDispOutPort;
    struct SunxiMemOpsS *pMemops;
    pthread_mutex_t updateLock;
};

#endif /* _MGGAL_SUNXIFBION */
#endif /* _GAL_sunxifbion_h */
