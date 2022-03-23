#ifndef YUV_ROTATE_H
#define YUV_ROTATE_H

#ifdef __cplusplus
extern "C"
{
#endif

void nv_rotage90(unsigned int width, unsigned int height, unsigned char *src_addr, unsigned char *dst_addr);
void nv_rotage270(unsigned int width, unsigned int height, unsigned char *src_addr, unsigned char *dst_addr);

#ifdef __cplusplus
}
#endif

#endif
