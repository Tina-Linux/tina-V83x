/*************************************************************************
    > File Name: convert.h
    > Author:
    > Mail:
    > Created Time:
 ************************************************************************/

#ifndef _CONVERT_H
#define _CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define WORD  unsigned short
#define DWORD unsigned int
#define LONG  unsigned int


/* Bitmap header */
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

/* Bitmap info header */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER;

typedef int (*ConverFunc)(void *rgbData,void *yuvData,int width,int height);

int NV21ToRGB24(void *RGB24,void *NV21,int width,int height);
int YUVToBMP(const char *bmp_path,unsigned char *yuv_data,ConverFunc func,int width,int height);

#ifdef __cplusplus
}
#endif

#endif
