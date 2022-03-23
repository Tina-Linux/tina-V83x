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

/* SUNXIFBION GAL video driver implementation; this is just enough to make an
 *  GAL-based application THINK it's got a working video driver, for
 *  applications that call GAL_Init(GAL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting GAL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that GAL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 */

/* MiniGUI.cfg
 * [sunxifbion]
 * defaultmode=1280x800-32bpp
 * fbnum=3
 *
 * TODO: 16bit, 24bit
 *       copy boot logo
 *       wait sync
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#ifdef _MGGAL_SUNXIFBION

#include "sunxifbion.h"
#include "minigui.h"
#include "memops.h"

/* Initialization/Query functions */
static int SUNXIFBION_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **SUNXIFBION_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags);
static GAL_Surface *SUNXIFBION_SetVideoMode(_THIS, GAL_Surface *current,
        int width, int height, int bpp, Uint32 flags);
static int SUNXIFBION_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors);
static void SUNXIFBION_VideoQuit(_THIS);
static int SUNXIFBION_FlipHWSurface(_THIS, GAL_Surface *surface,
        GAL_Rect *rects, BOOL enable);

/* Hardware surface functions */
static int SUNXIFBION_AllocHWSurface(_THIS, GAL_Surface *surface);
static void SUNXIFBION_FreeHWSurface(_THIS, GAL_Surface *surface);

/* SUNXIFBION driver bootstrap functions */

static void* task_do_update(void* data);

/* for task_do_update */
static int run_flag = 0;
static int end_flag = 0;

static int SUNXIFBION_Available(void) {
    return (1);
}

static void SUNXIFBION_DeleteDevice(GAL_VideoDevice *device) {
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *SUNXIFBION_CreateDevice(int devindex) {
    GAL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice *) malloc(sizeof(GAL_VideoDevice));
    if (device) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData *) malloc(
                (sizeof *device->hidden));
    }
    if ((device == NULL) || (device->hidden == NULL)) {
        GAL_OutOfMemory();
        if (device) {
            free(device);
        }
        return (0);
    }
    memset(device->hidden, 0, (sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = SUNXIFBION_VideoInit;
    device->ListModes = SUNXIFBION_ListModes;
    device->SetVideoMode = SUNXIFBION_SetVideoMode;
    device->SetColors = SUNXIFBION_SetColors;
    device->VideoQuit = SUNXIFBION_VideoQuit;
#ifndef _MGRM_THREADS
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = SUNXIFBION_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = SUNXIFBION_FreeHWSurface;
    device->FlipHWSurface = SUNXIFBION_FlipHWSurface;

    device->free = SUNXIFBION_DeleteDevice;

    return device;
}

VideoBootStrap SUNXIFBION_bootstrap = { "sunxifbion", "sunxifbion video driver",
        SUNXIFBION_Available, SUNXIFBION_CreateDevice };

static int SUNXIFBION_VideoInit(_THIS, GAL_PixelFormat *vformat) {
    struct GAL_PrivateVideoData* data = this->hidden;

    fprintf(stderr, "NEWGAL>SUNXIFBION: Calling init method!\n");

    if (GetMgEtcIntValue("sunxifbion", "fbnum", &data->fbNum) < 0)
        data->fbNum = 1;

    if (data->fbNum < 1 || data->fbNum > 3) {
        data->fbNum = 1;
        GAL_SetError("NEWGAL>SUNXIFBION: fbnum is set 1\n");
    }

    /* Determine the screen depth (use default 8-bit depth) */
    /* we change this during the GAL_SetVideoMode implementation... */
    vformat->BitsPerPixel = 32;
    switch (vformat->BitsPerPixel) {
    case 8:
        vformat->BytesPerPixel = 1;
        break;
    case 12:
        vformat->BitsPerPixel = 16;
        vformat->BytesPerPixel = 2;
        vformat->Rmask = 0x00000F00;
        vformat->Gmask = 0x000000F0;
        vformat->Bmask = 0x0000000F;
        break;
    case 16:
        vformat->BytesPerPixel = 2;
        vformat->Rmask = 0x0000F800;
        vformat->Gmask = 0x000007E0;
        vformat->Bmask = 0x0000001F;
        break;
    case 32:
        vformat->BytesPerPixel = 4;
        vformat->Amask = 0xFF000000;
        vformat->Rmask = 0x00FF0000;
        vformat->Gmask = 0x0000FF00;
        vformat->Bmask = 0x000000FF;
        break;
    default:
        GAL_SetError ("NEWGAL>QVFB: Not supported depth: %d, "
                "please try to use Shadow NEWGAL engine with targetname qvfb.\n", vformat->BitsPerPixel);
        return -1;
    }

    data->mDispOutPort = CreateVideoOutport(0);
    if (data->mDispOutPort == NULL) {
        GAL_SetError("NEWGAL>SUNXIFBION: CreateVideoOutport ERR\n");
        return -1;
    }

    pthread_mutex_init(&data->updateLock, NULL);
    /* We're done! */
    return (0);
}

static GAL_Rect **SUNXIFBION_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags) {
    if (format->BitsPerPixel < 8) {
        return NULL;
    }

    return (GAL_Rect**) -1;
}

static GAL_Surface *SUNXIFBION_SetVideoMode(_THIS, GAL_Surface *current,
        int width, int height, int bpp, Uint32 flags) {
    int pitch;
    VoutRect rect;
    struct GAL_PrivateVideoData* data = this->hidden;

    rect.x = 0;
    rect.y = 0;
    rect.width = width;
    rect.height = height;

    data->mDispOutPort->init(data->mDispOutPort, 0, 0, &rect);

    rect.width = data->mDispOutPort->getScreenWidth(data->mDispOutPort);
    rect.height = data->mDispOutPort->getScreenHeight(data->mDispOutPort);

    data->mDispOutPort->setRect(data->mDispOutPort, &rect);

    data->pMemops = GetMemAdapterOpsS();
    SunxiMemOpen(data->pMemops);

    pitch = width * ((bpp + 7) / 8);
    pitch = (pitch + 3) & ~3;

    data->bufLen = pitch * height * data->fbNum;
    data->pVirBuf[0] = SunxiMemPalloc(data->pMemops, data->bufLen);
    data->pPhyBuf[0] = SunxiMemGetPhysicAddressCpu(data->pMemops,
            data->pVirBuf[0]);

    if (!data->pVirBuf[0]) {
        fprintf(stderr, "NEWGAL>SUNXIFBION: "
                "Couldn't allocate buffer for requested mode\n");
        SUNXIFBION_VideoQuit(this);
        return NULL;
    }

    if (data->fbNum == 3) {
        data->pVirBuf[1] = data->pVirBuf[0] + pitch * height;
        data->pPhyBuf[1] = data->pPhyBuf[0] + pitch * height;
        data->pVirBuf[2] = data->pVirBuf[1] + pitch * height;
        data->pPhyBuf[2] = data->pPhyBuf[1] + pitch * height;

        this->doubleBufferStatus = TRUE;
        data->rBuf.y_phaddr = (unsigned long) data->pPhyBuf[1];
    } else if (data->fbNum == 2) {
        data->pVirBuf[1] = data->pVirBuf[0] + pitch * height;
        data->pPhyBuf[1] = data->pPhyBuf[0] + pitch * height;

        this->doubleBufferStatus = TRUE;
        data->rBuf.y_phaddr = (unsigned long) data->pPhyBuf[1];
    } else {
        data->rBuf.y_phaddr = (unsigned long) data->pPhyBuf[0];
    }
    data->rBuf.v_phaddr = 0;
    data->rBuf.u_phaddr = 0;
    data->rBuf.isExtPhy = VIDEO_USE_EXTERN_ION_BUF;

    memset(data->pVirBuf[0], 0, data->bufLen);

    SunxiMemFlushCache(data->pMemops, data->pVirBuf[0], data->bufLen);

    /* Allocate the new pixel format for the screen */
    if (!GAL_ReallocFormat(current, bpp, 0, 0, 0, 0)) {
        free(data->pVirBuf[0]);
        data->pVirBuf[0] = NULL;
        fprintf(stderr, "NEWGAL>SUNXIFBION: "
                "Couldn't allocate new pixel format for requested mode\n");
        SUNXIFBION_VideoQuit(this);
        return (NULL);
    }

    data->vparam.srcInfo.crop_x = 0;
    data->vparam.srcInfo.crop_y = 0;
    data->vparam.srcInfo.crop_w = width;
    data->vparam.srcInfo.crop_h = height;

    data->vparam.srcInfo.w = width;
    data->vparam.srcInfo.h = height;
    data->vparam.srcInfo.color_space = VIDEO_BT601;
    data->vparam.srcInfo.format = VIDEO_PIXEL_FORMAT_ARGB;

    /* Set up the new mode framebuffer */
    if (data->fbNum == 2 || data->fbNum == 3) {
        current->flags = (GAL_FULLSCREEN | GAL_HWSURFACE | GAL_DOUBLEBUF);
    } else {
        current->flags = (GAL_FULLSCREEN | GAL_HWSURFACE);
    }
    data->w = current->w = width;
    data->h = current->h = height;
    current->pitch = pitch;
    if (data->fbNum == 3) {
        current->pixels = data->pVirBuf[2];
    } else {
        current->pixels = data->pVirBuf[0];
    }

    data->mDispOutPort->queueToDisplay(data->mDispOutPort,
            data->vparam.srcInfo.w * data->vparam.srcInfo.h * 4, &data->vparam,
            &data->rBuf);
    data->mDispOutPort->SetZorder(data->mDispOutPort, VIDEO_ZORDER_BOTTOM);
    data->mDispOutPort->setEnable(data->mDispOutPort, 1);

    if (data->fbNum == 3) {
        /* UI refresh thread */
        pthread_attr_t new_attr;
        run_flag = 1;
        end_flag = 0;
        pthread_attr_init(&new_attr);
        pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&data->updateTh, &new_attr, task_do_update, this);
        pthread_attr_destroy(&new_attr);
    }
    /* We're done */
    return (current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int SUNXIFBION_AllocHWSurface(_THIS, GAL_Surface *surface) {
    return (-1);
}
static void SUNXIFBION_FreeHWSurface(_THIS, GAL_Surface *surface) {
    surface->pixels = NULL;
}

static int SUNXIFBION_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors) {
    /* do nothing of note. */
    return (1);
}

/* Note:  If we are terminated, this could be called in the middle of
 * another video routine -- notably UpdateRects.
 */
static void SUNXIFBION_VideoQuit(_THIS) {

    if (this->hidden->updateTh > 0) {
        run_flag = 0;
        /* waiting task_do_update end */
        for (;;) {
            if (end_flag != 0) {
                break;
            }
        }
    }

    if (this->hidden->pVirBuf[0] != NULL) {
        SunxiMemPfree(this->hidden->pMemops, this->hidden->pVirBuf[0]);
        SunxiMemClose(this->hidden->pMemops);
        this->hidden->pVirBuf[0] = NULL;
    }
    pthread_mutex_destroy(&this->hidden->updateLock);
    DestroyVideoOutport(this->hidden->mDispOutPort);
}

static int SUNXIFBION_FlipHWSurface(_THIS, GAL_Surface *surface,
        GAL_Rect *rects, BOOL enable) {

    pthread_mutex_lock(&this->hidden->updateLock);

    struct GAL_PrivateVideoData* data = this->hidden;

    if (data->fbNum == 3) {
        SunxiMemFlushCache(data->pMemops, data->pVirBuf[2],
                data->bufLen / data->fbNum);
        GAL_memcpy(data->pVirBuf[data->flipPage], data->pVirBuf[2],
                data->bufLen / data->fbNum);
        data->flipPage = !data->flipPage;
        data->dirty = TRUE;
    } else if (data->fbNum == 2) {
        SunxiMemFlushCache(data->pMemops, data->pVirBuf[data->flipPage],
                data->bufLen / 2);
        data->rBuf.y_phaddr = (unsigned long) data->pPhyBuf[data->flipPage];

        data->mDispOutPort->queueToDisplay(data->mDispOutPort,
                data->vparam.srcInfo.w * data->vparam.srcInfo.h * 4,
                &data->vparam, &data->rBuf);

        /* Waiting for the screen refresh cycle */
        usleep(1000 * 16);

        GAL_memcpy(data->pVirBuf[!data->flipPage],
                data->pVirBuf[data->flipPage], data->bufLen / 2);

        data->flipPage = !data->flipPage;
        this->screen->pixels = data->pVirBuf[data->flipPage];
    }

    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static void* task_do_update(void* data) {
    _THIS;
    this = data;

    /* waiting for __gal_screen */
    for (;;) {
        if (__gal_screen != NULL) {
            break;
        }
    }

    while (run_flag) {
        if (this->hidden->dirty) {
            pthread_mutex_lock(&this->hidden->updateLock);

            SunxiMemFlushCache(this->hidden->pMemops,
                    this->hidden->pVirBuf[!this->hidden->flipPage],
                    this->hidden->bufLen / this->hidden->fbNum);
            this->hidden->rBuf.y_phaddr =
                    (unsigned long) this->hidden->pPhyBuf[!this->hidden->flipPage];

            this->hidden->mDispOutPort->queueToDisplay(
                    this->hidden->mDispOutPort,
                    this->hidden->vparam.srcInfo.w
                            * this->hidden->vparam.srcInfo.h * 4,
                    &this->hidden->vparam, &this->hidden->rBuf);
            this->hidden->dirty = FALSE;

            pthread_mutex_unlock(&this->hidden->updateLock);
        }

        /* Waiting for the screen refresh cycle */
        usleep(1000 * 16);
    }

    end_flag = 1;
    return NULL;
}

#endif /* _MGGAL_SUNXIFBION */
