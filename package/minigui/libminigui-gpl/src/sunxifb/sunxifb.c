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

/* SUNXIFB GAL video driver implementation; this is just enough to make an
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
 * [sunxifb]
 * defaultmode=1280x800-32bpp
 * flipbuffer=1
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#ifdef _MGGAL_SUNXIFB

#include "sunxifb.h"
#include "minigui.h"
#include "memops.h"

#define FBIO_CACHE_SYNC         0x4630
#define FBIO_ENABLE_CACHE       0x4631
/*#define SUNXIFB_DEBUG   1*/

/* Initialization/Query functions */
static int SUNXIFB_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **SUNXIFB_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags);
static GAL_Surface *SUNXIFB_SetVideoMode(_THIS, GAL_Surface *current, int width,
        int height, int bpp, Uint32 flags);
static int SUNXIFB_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors);
static void SUNXIFB_VideoQuit(_THIS);
static int SUNXIFB_FlipHWSurface(_THIS, GAL_Surface *surface, GAL_Rect *rects,
        BOOL enable);
static int SUNXIFB_DoubleBufferEnable(_THIS, GAL_Surface *current, BOOL enable);

/* Hardware surface functions */
static int SUNXIFB_AllocHWSurface(_THIS, GAL_Surface *surface);
static void SUNXIFB_FreeHWSurface(_THIS, GAL_Surface *surface);

/* SUNXIFB driver bootstrap functions */

static void* task_do_update(void* data);

/* for task_do_update */
static int run_flag = 0;
static int end_flag = 0;

#ifdef SUNXIFB_DEBUG
static void print_vinfo(struct fb_var_screeninfo *vinfo) {
    fprintf(stderr, "Printing vinfo:\n");
    fprintf(stderr, "txres: %d\n", vinfo->xres);
    fprintf(stderr, "tyres: %d\n", vinfo->yres);
    fprintf(stderr, "txres_virtual: %d\n", vinfo->xres_virtual);
    fprintf(stderr, "tyres_virtual: %d\n", vinfo->yres_virtual);
    fprintf(stderr, "txoffset: %d\n", vinfo->xoffset);
    fprintf(stderr, "tyoffset: %d\n", vinfo->yoffset);
    fprintf(stderr, "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    fprintf(stderr, "tgrayscale: %d\n", vinfo->grayscale);
    fprintf(stderr, "tnonstd: %d\n", vinfo->nonstd);
    fprintf(stderr, "tactivate: %d\n", vinfo->activate);
    fprintf(stderr, "theight: %d\n", vinfo->height);
    fprintf(stderr, "twidth: %d\n", vinfo->width);
    fprintf(stderr, "taccel_flags: %d\n", vinfo->accel_flags);
    fprintf(stderr, "tpixclock: %d\n", vinfo->pixclock);
    fprintf(stderr, "tleft_margin: %d\n", vinfo->left_margin);
    fprintf(stderr, "tright_margin: %d\n", vinfo->right_margin);
    fprintf(stderr, "tupper_margin: %d\n", vinfo->upper_margin);
    fprintf(stderr, "tlower_margin: %d\n", vinfo->lower_margin);
    fprintf(stderr, "thsync_len: %d\n", vinfo->hsync_len);
    fprintf(stderr, "tvsync_len: %d\n", vinfo->vsync_len);
    fprintf(stderr, "tsync: %d\n", vinfo->sync);
    fprintf(stderr, "tvmode: %d\n", vinfo->vmode);
    fprintf(stderr, "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    fprintf(stderr, "tgreen: %d/%d\n", vinfo->green.length,
            vinfo->green.offset);
    fprintf(stderr, "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    fprintf(stderr, "talpha: %d/%d\n", vinfo->transp.length,
            vinfo->transp.offset);
}

static void print_finfo(struct fb_fix_screeninfo *finfo) {
    fprintf(stderr, "Printing finfo:\n");
    fprintf(stderr, "tsmem_start = %p\n", (char *) finfo->smem_start);
    fprintf(stderr, "tsmem_len = %d\n", finfo->smem_len);
    fprintf(stderr, "ttype = %d\n", finfo->type);
    fprintf(stderr, "ttype_aux = %d\n", finfo->type_aux);
    fprintf(stderr, "tvisual = %d\n", finfo->visual);
    fprintf(stderr, "txpanstep = %d\n", finfo->xpanstep);
    fprintf(stderr, "typanstep = %d\n", finfo->ypanstep);
    fprintf(stderr, "tywrapstep = %d\n", finfo->ywrapstep);
    fprintf(stderr, "tline_length = %d\n", finfo->line_length);
    fprintf(stderr, "tmmio_start = %p\n", (char *) finfo->mmio_start);
    fprintf(stderr, "tmmio_len = %d\n", finfo->mmio_len);
    fprintf(stderr, "taccel = %d\n", finfo->accel);
}
#endif

static int SUNXIFB_Available(void) {
    return (1);
}

static void SUNXIFB_DeleteDevice(GAL_VideoDevice *device) {
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *SUNXIFB_CreateDevice(int devindex) {
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
    device->VideoInit = SUNXIFB_VideoInit;
    device->ListModes = SUNXIFB_ListModes;
    device->SetVideoMode = SUNXIFB_SetVideoMode;
    device->SetColors = SUNXIFB_SetColors;
    device->VideoQuit = SUNXIFB_VideoQuit;
#ifndef _MGRM_THREADS
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = SUNXIFB_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = SUNXIFB_FreeHWSurface;
    device->FlipHWSurface = SUNXIFB_FlipHWSurface;
    device->DoubleBufferEnable = SUNXIFB_DoubleBufferEnable;

    device->free = SUNXIFB_DeleteDevice;

    device->doubleBufferStatus = FALSE;
    device->YOffsetHWSurface = FALSE;

    return device;
}

VideoBootStrap SUNXIFB_bootstrap = { "sunxifb", "sunxifb video driver",
        SUNXIFB_Available, SUNXIFB_CreateDevice };

static int SUNXIFB_VideoInit(_THIS, GAL_PixelFormat *vformat) {

    fprintf(stderr, "NEWGAL>SUNXIFB: Calling init method!\n");

    struct GAL_PrivateVideoData* data = this->hidden;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    const char *GAL_fbdev;
    int i;

    if (GetMgEtcIntValue("sunxifb", "flipbuffer", &data->flipBuffer) < 0)
        data->flipBuffer = 0;

    pthread_mutex_init(&data->updateLock, NULL);
    pthread_cond_init(&data->drawCond, NULL);

    /* Initialize the library */
    GAL_fbdev = getenv("FRAMEBUFFER");
    if (GAL_fbdev == NULL) {
        GAL_fbdev = "/dev/fb0";
    }
    data->consoleFd = open(GAL_fbdev, O_RDWR, 0);
    if (data->consoleFd < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Unable to open %s\n", GAL_fbdev);
        return (-1);
    }

    /* Get the type of video hardware */
    if (ioctl(data->consoleFd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console hardware info\n");
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    /* Determine the current screen depth */
    if (ioctl(data->consoleFd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console pixel format\n");
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    /* Memory map the device, compensating for buggy PPC mmap() */
    data->mappedOffSet = (((long) finfo.smem_start)
            - (((long) finfo.smem_start) & ~(getpagesize() - 1)));
    data->mappedMemLen = finfo.smem_len + data->mappedOffSet;
    data->mappedOneBufLen = finfo.line_length * vinfo.yres;
    data->mappedBufNum = data->mappedMemLen / data->mappedOneBufLen;
    /*data->mappedBufNum = 2;*/
    data->w = vinfo.xres;
    data->h = vinfo.yres;

    /* If flip buffer, open the cache */
    if (data->flipBuffer && data->mappedBufNum > 1) {
        unsigned int args[2];
        args[0] = 1;
        if (ioctl(data->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
            GAL_SetError("NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE failed\n");
        }
    }

    data->mappedMem = mmap(NULL, data->mappedMemLen, PROT_READ | PROT_WRITE,
    MAP_SHARED, data->consoleFd, 0);

    if (data->mappedMem == (char *) -1) {
        GAL_SetError("NEWGAL>SUNXIFB: Unable to memory map the video hardware\n");
        data->mappedMem = NULL;
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    vformat->BitsPerPixel = vinfo.bits_per_pixel;
    if (vformat->BitsPerPixel < 8) {
        vformat->MSBLeft = !(vinfo.red.msb_right);
        return 0;
    }
    for (i = 0; i < vinfo.red.length; ++i) {
        vformat->Rmask <<= 1;
        vformat->Rmask |= (0x00000001 << vinfo.red.offset);
    }
    for (i = 0; i < vinfo.green.length; ++i) {
        vformat->Gmask <<= 1;
        vformat->Gmask |= (0x00000001 << vinfo.green.offset);
    }
    for (i = 0; i < vinfo.blue.length; ++i) {
        vformat->Bmask <<= 1;
        vformat->Bmask |= (0x00000001 << vinfo.blue.offset);
    }
    for (i = 0; i < vinfo.transp.length; ++i) {
        vformat->Amask <<= 1;
        vformat->Amask |= (0x00000001 << vinfo.transp.offset);
    }

#ifdef SUNXIFB_DEBUG
    print_vinfo(&vinfo);
    print_finfo(&finfo);
#endif

    /* Set flip address */
    if (data->mappedBufNum == 2) {
        data->flipAddress[0] = data->mappedMem + data->mappedOffSet;
        data->flipAddress[1] = data->flipAddress[0] + data->mappedOneBufLen;
    }

    /* We're done! */
    return (0);
}

static GAL_Rect **SUNXIFB_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags) {
    if (format->BitsPerPixel < 8) {
        return NULL;
    }

    return (GAL_Rect**) -1;
}

static GAL_Surface *SUNXIFB_SetVideoMode(_THIS, GAL_Surface *current, int width,
        int height, int bpp, Uint32 flags) {
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    struct GAL_PrivateVideoData* data = this->hidden;
    int i;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;

    /* Set the video mode and get the final screen format */
    if (ioctl(data->consoleFd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console screen info");
        return (NULL);
    }

    Rmask = 0;
    for (i = 0; i < vinfo.red.length; ++i) {
        Rmask <<= 1;
        Rmask |= (0x00000001 << vinfo.red.offset);
    }
    Gmask = 0;
    for (i = 0; i < vinfo.green.length; ++i) {
        Gmask <<= 1;
        Gmask |= (0x00000001 << vinfo.green.offset);
    }
    Bmask = 0;
    for (i = 0; i < vinfo.blue.length; ++i) {
        Bmask <<= 1;
        Bmask |= (0x00000001 << vinfo.blue.offset);
    }
    Amask = 0;
    for (i = 0; i < vinfo.transp.length; ++i) {
        Amask <<= 1;
        Amask |= (0x00000001 << vinfo.transp.offset);
    }

    if (!GAL_ReallocFormat(current, vinfo.bits_per_pixel, Rmask, Gmask, Bmask,
            Amask)) {
        return (NULL);
    }
    if (vinfo.bits_per_pixel < 8) {
        current->format->MSBLeft = !(vinfo.red.msb_right);
    }

    /* Get the fixed information about the console hardware.
     This is necessary since finfo.line_length changes.
     */
    if (ioctl(data->consoleFd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console hardware info");
        return (NULL);
    }

    data->cacheVinfo = vinfo;

    /* Set up the new mode framebuffer */
    current->flags = (GAL_FULLSCREEN | GAL_HWSURFACE);
    current->w = vinfo.xres;
    current->h = vinfo.yres;
    current->pitch = finfo.line_length;
    current->pixels = data->mappedMem + data->mappedOffSet;

    if (data->flipBuffer) {
        /* The number of buffers is greater than 1 to flip pages */
        if (data->mappedBufNum > 1) {
            current->flags |= GAL_DOUBLEBUF;
            this->doubleBufferStatus = TRUE;
        }
        if (data->mappedBufNum > 2) {
            /* Calculate drawing and display buffer index */
            data->mappedDispIndex = vinfo.yoffset / data->h;
            data->mappedDrawIndex =
                    data->mappedDispIndex >= (data->mappedBufNum - 1) ?
                            0 : data->mappedDispIndex + 1;
            /* The draw index buffer is used to draw the image */
            current->pixels = data->mappedMem
                    + data->mappedDrawIndex * data->mappedOneBufLen;

            /* UI refresh thread */
            pthread_attr_t new_attr;
            run_flag = 1;
            end_flag = 0;
            pthread_attr_init(&new_attr);
            pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&data->updateTh, &new_attr, task_do_update, this);
            pthread_attr_destroy(&new_attr);
        } else if (data->mappedBufNum == 2) {
            /* Ensure that the last image is not cleared */
            if (vinfo.yoffset == 0) {
                data->flipPage = !data->flipPage;
                current->pixels = data->flipAddress[data->flipPage];
            }
        }
    }
    /* We're done */
    return (current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int SUNXIFB_AllocHWSurface(_THIS, GAL_Surface *surface) {
    return (-1);
}
static void SUNXIFB_FreeHWSurface(_THIS, GAL_Surface *surface) {
    surface->pixels = NULL;
}

static int SUNXIFB_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors) {
    /* do nothing of note. */
    return (1);
}

/* Note:  If we are terminated, this could be called in the middle of
 * another video routine -- notably UpdateRects.
 */
static void SUNXIFB_VideoQuit(_THIS) {

    if (this->hidden->updateTh > 0) {
        run_flag = 0;
        /* waiting task_do_update end */
        for (;;) {
            if (end_flag != 0) {
                break;
            }
        }
    }

    pthread_mutex_destroy(&this->hidden->updateLock);
    pthread_cond_destroy(&this->hidden->drawCond);

    /* Close console and input file descriptors */
    if (this->hidden->consoleFd > 0) {
        if (this->hidden->flipBuffer && this->hidden->mappedBufNum > 1) {
            unsigned int args[2];
            args[0] = 0;
            if (ioctl(this->hidden->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE disable failed\n");
            }
        }

        /* Unmap the video framebuffer and I/O registers */
        if (this->hidden->mappedMem) {
            munmap(this->hidden->mappedMem, this->hidden->mappedMemLen);
            this->hidden->mappedMem = NULL;
        }

        /* We're all done with the framebuffer */
        close(this->hidden->consoleFd);
        this->hidden->consoleFd = -1;
    }
}

static int SUNXIFB_FlipHWSurface(_THIS, GAL_Surface *surface, GAL_Rect *rects,
        BOOL enable) {

    pthread_mutex_lock(&this->hidden->updateLock);

    struct GAL_PrivateVideoData* data = this->hidden;

    if (this->doubleBufferStatus) {
        if (data->mappedBufNum > 2) {
            int tempIndex = data->mappedDrawIndex + 1;
            if (tempIndex > data->mappedBufNum - 1)
                tempIndex = 0;
            /* Draw buffer full */
            if (tempIndex == data->mappedDispIndex)
                pthread_cond_wait(&data->drawCond, &data->updateLock);

            unsigned int args[2];
            args[0] = (unsigned int) data->mappedMem
                    + data->mappedDrawIndex * data->mappedOneBufLen;
            args[1] = data->mappedOneBufLen;
            if (ioctl(data->consoleFd, FBIO_CACHE_SYNC, args) < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
            }

            /* Copy the drawn image to the buffer to be displayed */
            GAL_memcpy(data->mappedMem + tempIndex * data->mappedOneBufLen,
                    data->mappedMem
                            + data->mappedDrawIndex * data->mappedOneBufLen,
                    data->mappedOneBufLen);
            data->mappedDrawIndex = tempIndex;
            /* Set minigui drawing address */
            this->screen->pixels = data->mappedMem
                    + data->mappedDrawIndex * data->mappedOneBufLen;
        } else if (data->mappedBufNum == 2) {
            unsigned int args[2];
            args[0] = (unsigned int) data->flipAddress[data->flipPage];
            args[1] = data->mappedOneBufLen;
            if (ioctl(data->consoleFd, FBIO_CACHE_SYNC, args) < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
            }

            data->cacheVinfo.yoffset = data->flipPage * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
                return (-1);
            }

            GAL_memcpy(data->flipAddress[!data->flipPage],
                    data->flipAddress[data->flipPage], data->mappedOneBufLen);

            data->flipPage = !data->flipPage;
            this->screen->pixels = data->flipAddress[data->flipPage];

#ifdef SUNXIFB_DEBUG
            static struct timeval new, old;
            static int fps;
            gettimeofday(&new, NULL);
            if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
                printf("flip double buffer fps is %d\n", fps);
                old = new;
                fps = 0;
            } else {
                fps++;
            }
#endif
        }
    }
    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static int SUNXIFB_DoubleBufferEnable(_THIS, GAL_Surface *current, BOOL enable) {
    pthread_mutex_lock(&this->hidden->updateLock);
    struct GAL_PrivateVideoData* data = this->hidden;

    unsigned int args[2];
    args[0] = enable;
    if (ioctl(data->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE disable failed\n");
        return (-1);
    }
    if (data->mappedMem) {
        munmap(data->mappedMem, data->mappedMemLen);
        data->mappedMem = NULL;
    }

    data->mappedMem = mmap(NULL, data->mappedMemLen,
    PROT_READ | PROT_WRITE, MAP_SHARED, data->consoleFd, 0);
    if (data->mappedMem == (char *) -1) {
        GAL_SetError("NEWGAL>SUNXIFB: Unable to memory map the video hardware\n");
        data->mappedMem = NULL;
        return (-1);
    }

    current->pixels = data->mappedMem + data->mappedOffSet;

    /* Set flip address */
    if (data->mappedBufNum > 2) {
        /* The last buffer is used to draw the image */
        this->screen->pixels = data->mappedMem
                + data->mappedDrawIndex * data->mappedOneBufLen;

        if (!enable) {
            /* Must be assigned first, otherwise task_do_update may still be running,
             * resulting in yoffset value is not normal */
            this->doubleBufferStatus = enable;

            data->cacheVinfo.yoffset = data->mappedDrawIndex * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
            }
        } else {
            /* Copy the current draw image to the disp buffer */
            GAL_memcpy(
                    data->mappedMem
                            + data->mappedDispIndex * data->mappedOneBufLen,
                    data->mappedMem
                            + data->mappedDrawIndex * data->mappedOneBufLen,
                    data->mappedOneBufLen);

            data->cacheVinfo.yoffset = data->mappedDispIndex * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
            }
        }

        pthread_mutex_unlock(&data->updateLock);
    } else if (data->mappedBufNum == 2) {
        data->flipAddress[0] = data->mappedMem + this->hidden->mappedOffSet;
        data->flipAddress[1] = data->flipAddress[0] + data->mappedOneBufLen;

        pthread_mutex_unlock(&data->updateLock);

        if (!enable) {
            data->flipPage = !data->flipPage;
            this->screen->pixels = data->flipAddress[data->flipPage];
        } else {
            GAL_Rect rc;
            rc.x = 0;
            rc.y = 0;
            rc.w = current->w;
            rc.h = current->h;
            /* Must be assigned first, otherwise Flip cannot be */
            this->doubleBufferStatus = enable;
            SUNXIFB_FlipHWSurface(this, current, &rc, TRUE);
        }
    }
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
        /* After flip off, not in flip */
        if (this->doubleBufferStatus) {
            int tempIndex = this->hidden->mappedDispIndex + 1;
            if (tempIndex > this->hidden->mappedBufNum - 1)
                tempIndex = 0;
            if (tempIndex != this->hidden->mappedDrawIndex) {
                /* Already drawn one frame and can display */
                unsigned int args[2];
                args[0] = (unsigned int) this->hidden->mappedMem
                        + tempIndex * this->hidden->mappedOneBufLen;
                args[1] = this->hidden->mappedOneBufLen;
                if (ioctl(this->hidden->consoleFd, FBIO_CACHE_SYNC, args) < 0) {
                    GAL_SetError("NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
                }

                this->hidden->cacheVinfo.yoffset = tempIndex * this->hidden->h;
                if (ioctl(this->hidden->consoleFd, FBIOPAN_DISPLAY,
                        &this->hidden->cacheVinfo) < 0) {
                    GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
                }

                this->hidden->mappedDispIndex = tempIndex;
                pthread_cond_signal(&this->hidden->drawCond);
#ifdef SUNXIFB_DEBUG
                static struct timeval new, old;
                static int fps;
                gettimeofday(&new, NULL);
                if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
                    printf("task do update fps is %d\n", fps);
                    old = new;
                    fps = 0;
                } else {
                    fps++;
                }
#endif
            } else {
                /* Avoid deadlocks */
                pthread_cond_signal(&this->hidden->drawCond);
            }
        }

        /* Waiting for the screen refresh cycle */
        /*        if (ioctl(this->hidden->consoleFd, FBIO_WAITFORVSYNC, NULL) < 0) {
         GAL_SetError("NEWGAL>SUNXIFB: FBIO_WAITFORVSYNC failed\n");
         }*/

        usleep(1000 * 1);
    }

    end_flag = 1;
    return NULL;
}

#endif /* _MGGAL_SUNXIFB */
