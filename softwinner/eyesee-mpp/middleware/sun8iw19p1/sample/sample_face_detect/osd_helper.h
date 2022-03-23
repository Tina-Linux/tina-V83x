#ifndef __OSD_HELPER__
#define __OSD_HELPER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#define MAX(x, y)   ((x) > (y) ? (x) : (y))

#define GETR(y,u,v) ((int)((y) + 1.14*(v-128)))
#define GETG(y,u,v) ((int)((y) - 0.39*(u-128) - 0.58*(v-128)))
#define GETB(y,u,v) ((int)((y) + 2.03*(u-128)))

#define GETY(r,g,b) ((int)(0.299*(r) + 0.587*(g) + 0.114*(b)))
#define GETU(r,g,b) ((int)(-0.147*(r) - 0.289*(g) + 0.436*(b)) + 128)
#define GETV(r,g,b) ((int)(0.615*(r) - 0.515*(g) - 0.100*(b)) + 128)

#define FONTSIZE            (16)        //Notes:目前只支持16
#define ENGLISH_DOT_LIB     "ASC16"     //英文字库路径
#define CHINESE_DOT_LIB     "HZK16"     //中文字库路径

void GenTextBuffer(char* szText, char* pTextBuffer, char* pRGB8888);
void GenRectBuffer(char* pRGB8888, int region_w, int region_h, int x, int y, int w, int h, int len_w);
void DrawRect_Nv21(char* pNV21_y, char* pNV21_vu, int region_w, int region_h, int x, int y, int w, int h, int len_w);

#endif
