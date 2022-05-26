#define IDSTRING "fbviewer 1.0"
#define DEFAULT_FRAMEBUFFER "/dev/fb0"
#define FBV_SUPPORT_JPEG
#define FBV_SUPPORT_PNG
#define FBV_SUPPORT_BMP

int fb_display(unsigned char *rgbbuff, unsigned char * alpha,
               unsigned int x_size, unsigned int y_size,
               unsigned int x_pan, unsigned int y_pan,
               unsigned int x_offs, unsigned int y_offs);
