#ifndef _GPU_YUV2RGB_H_
#define _GPU_YUV2RGB_H_

#ifdef SUNXI_DISPLAY_GPU
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <EGL/fbdev_window.h>
#include <libdrm/drm_fourcc.h>
#include <time.h>
#include <sys/time.h>

/*FORMAT*/
#define FORMAT_YUV420 0
#define FORMAT_NV12 1
#define FORMAT_NV21 2
#define FORMAT_YU12 3
#define FORMAT_YV12 4
#define FORMAT_YUYV 5
#define FORMAT_RGBA8888 6
#define FORMAT_RGB565 7
#define FORMAT_ARGB8888 8
/*EGL*/
static EGLDisplay dpy;
static EGLSurface surface;
static EGLContext context;
fbdev_window Ewin;

EGLImageKHR input_img;
EGLImageKHR output_img;

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;
static GLint vTexSamplerHandler;
GLuint outputFbo;
GLuint outputRbo;

#define GPU_IONBUF 0
#define GPU_FRAMEBUFFER 1

typedef struct {
    unsigned int window_height;
    unsigned int window_width;
    unsigned int depth;
    unsigned int fsaa;
} mmark_context;

typedef struct {
    int cropx;
    int cropy;
    int cropwidth;
    int cropheight;
    int srcwidth;
    int srcheight;
    int srcformat;
} Srcresource;

typedef struct {
    int dstx;
    int dsty;
    int dstwidth;
    int dstheight;
    int dstformat;
    int screenheight;
} Dstresource;

void InitGLResource(int flag);
void DestoryGLResource(int flag);
int ShaderYUVtoRGB(Dstresource * dst_resource);
int ShaderYUVtoRGB_toionbuf(Dstresource * dst_resource);
int gl_createSRCKHR(int dma_fd, Srcresource * src_resource);
void DestorySrcKHR();
EGLImageKHR gl_createDSTKHR(int output_fd, Dstresource * dst_resource);
void DestoryDstKHR(EGLImageKHR swap_img);
#endif
#endif
