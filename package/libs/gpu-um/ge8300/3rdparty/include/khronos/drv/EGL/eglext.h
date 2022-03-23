/*************************************************************************/ /*!
@File
@Title          EGL extensions.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __drv_eglext_h__
#define __drv_eglext_h__

/* We want all the prototypes */
#define EGL_EGLEXT_PROTOTYPES

/* NOTE: If you want to override an extension, you could put it here,
 * before we include the Khronos header, so the khronos header doesn't
 * define it first.
 */

/* Include the Khronos header first */
#include <EGL/eglext.h>

/* NOTE: Extensions following might be overridden by the platform's
 * version of eglext.h. For example, EGL_ANDROID_blob_cache will
 * already be defined by this point for Android, but not Linux.
 */

#ifndef EGL_IMG_cl_image
#define EGL_IMG_cl_image 1
#define EGL_CL_IMAGE_IMG						0x6010  /* experimental eglCreateImageKHR target */
#endif

#ifndef EGL_IMG_rl_image
#define EGL_IMG_rl_image 1
#define EGL_RL_TEXTURE_2D_IMG					0x6011  /* experimental eglCreateImageKHR target */
#endif

#define EGL_OPENRL_API_IMG									0x30A3
#define EGL_CONTEXT_OPENRL_ATTRIBUTE_IMG					0x30A4
#define EGL_OPENRL_BIT_IMG									0x00000080

#ifndef EGL_IMG_image_plane_attribs
#define EGL_IMG_image_plane_attribs 1
#define EGL_NATIVE_BUFFER_MULTIPLANE_SEPARATE_IMG 0x3105
#define EGL_NATIVE_BUFFER_PLANE_OFFSET_IMG        0x3106
#endif

#ifndef EGL_IMG_image_debug_dump
#define EGL_IMG_image_debug_dump 1

typedef void (EGLAPIENTRYP IMAGEDUMPCALLBACKPROC) (const void *userParam, EGLenum target, EGLImageKHR image, EGLClientBuffer buffer);

typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEDUMPCALLBACKPROC) (EGLDisplay dpy, EGLContext ctx, IMAGEDUMPCALLBACKPROC callback, const void *userParam);

#if defined(EGL_EGLEXT_PROTOTYPES)
EGLAPI EGLBoolean EGLAPIENTRY eglImageDumpCallbackIMG (EGLDisplay dpy, EGLContext ctx, IMAGEDUMPCALLBACKPROC callback, const void *userParam);
#endif
#endif

#endif /* __drv_eglext_h__ */
