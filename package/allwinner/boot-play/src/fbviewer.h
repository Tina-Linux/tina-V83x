#ifndef __FBV_H__
#define __FBV_H__

#include <linux/fb.h>
#include <sys/types.h>

#define FH_ERROR_OK     0
#define FH_ERROR_FILE   1	/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */

#define FBV_SUPPORT_PNG

struct framebuffer {
	int fbh;
	unsigned char *fb_base;
	unsigned char *fb_tmp;
	unsigned int screen_x;
	unsigned int screen_y;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	unsigned int x_stride;
	int cpp;
	int displayed_buffer;
};
struct Image
{
	int width, height;
	unsigned char *rgb;
	unsigned char *fbbuffer;
	unsigned char *alpha;
	int do_free;
	unsigned int x_offs;
	unsigned int y_offs;
};

int fb_init(struct framebuffer *fb);
void fb_release(struct framebuffer *fb);
int fb_show(struct Image *i, struct framebuffer *fb);
int fb_display(unsigned char *rgbbuff, unsigned char * alpha,
               unsigned int x_size, unsigned int y_size,
               unsigned int x_pan, unsigned int y_pan,
               unsigned int x_offs, unsigned int y_offs);
int getCurrentRes(int *x, int *y);

#ifdef FBV_SUPPORT_BMP
int fh_bmp_id(char *name);
int fh_bmp_load(char *name, unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_bmp_getsize(char *name, int *x, int *y);
#endif

#ifdef FBV_SUPPORT_JPEG
int fh_jpeg_id(char *name);
int fh_jpeg_load(char *name, unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_jpeg_getsize(char *name, int *x, int *y);
#endif

#ifdef FBV_SUPPORT_PNG
int fh_png_id(char *name);
int fh_png_load(char *name, unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_png_getsize(char *name, int *x, int *y);
#endif


#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * alpha_resize(unsigned char * alpha,int ox,int oy,int dx,int dy);
unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * rotate(unsigned char *i, int ox, int oy, int rot);
unsigned char * alpha_rotate(unsigned char *i, int ox, int oy, int rot);

#endif

