/*************************************************************************
    > File Name: convert.c
    > Author:
    > Mail:
    > Created Time:
 ************************************************************************/

#include "convert.h"

int NV21ToRGB24(void *RGB24,void *NV21,int width,int height)
{
    unsigned char *src_y = (unsigned char *)NV21;
    unsigned char *src_v = (unsigned char *)NV21 + width * height;
    unsigned char *src_u = (unsigned char *)NV21 + width * height + 1;

    unsigned char *dst_RGB = (unsigned char *)RGB24;

    int temp[3];

    if(RGB24 == NULL || NV21 == NULL || width <= 0 || height <= 0)
    {
        printf(" NV21ToRGB24 incorrect input parameter!\n");
        return -1;
    }

    for(int y = 0;y < height;y ++)
    {
        for(int x = 0;x < width;x ++)
        {
            int Y = y*width + x;
            int U = ( (y >> 1)*(width >>1) + (x >> 1) )<<1;
            int V = U;

            temp[0] = src_y[Y] + ((7289 * src_u[U])>>12) - 228;  //b
            temp[1] = src_y[Y] - ((1415 * src_u[U])>>12) - ((2936 * src_v[V])>>12) + 136;  //g
            temp[2] = src_y[Y] + ((5765 * src_v[V])>>12) - 180;  //r

            dst_RGB[3*Y] = (temp[0]<0? 0: temp[0]>255? 255: temp[0]);
            dst_RGB[3*Y+1] = (temp[1]<0? 0: temp[1]>255? 255: temp[1]);
            dst_RGB[3*Y+2] = (temp[2]<0? 0: temp[2]>255? 255: temp[2]);
        }
    }

    return 0;

}

int YUVToBMP(const char *bmp_path,unsigned char *yuv_data,ConverFunc func,int width,int height)
{
    unsigned char *rgb_24 = NULL;
    FILE *fp = NULL;

    BITMAPFILEHEADER BmpFileHeader;
    BITMAPINFOHEADER BmpInfoHeader;

    if(bmp_path == NULL || yuv_data == NULL || func == NULL || width <= 0 || height <= 0)
    {
        printf(" YUVToBMP incorrect input parameter!\n");
        return -1;
    }

   /* Fill header information */
   BmpFileHeader.bfType = 0x4d42;
   BmpFileHeader.bfSize = width*height*3 + sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
   BmpFileHeader.bfReserved1 = 0;
   BmpFileHeader.bfReserved2 = 0;
   BmpFileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

   BmpInfoHeader.biSize = sizeof(BmpInfoHeader);
   BmpInfoHeader.biWidth = width;
   BmpInfoHeader.biHeight = -height;
   BmpInfoHeader.biPlanes = 0x01;
   BmpInfoHeader.biBitCount = 24;
   BmpInfoHeader.biCompression = 0;
   BmpInfoHeader.biSizeImage = 0;
   //BmpInfoHeader.biXPelsPerMeter = 0;
   //BmpInfoHeader.biYPelsPerMeter = 0;
   BmpInfoHeader.biClrUsed = 0;
   BmpInfoHeader.biClrImportant = 0;

    rgb_24 = (unsigned char *)malloc(width*height*3);
    if(rgb_24 == NULL)
    {
       printf(" YUVToBMP alloc failed!\n");
       return -1;
    }

    func(rgb_24,yuv_data,width,height);

    /* Create bmp file */
    fp = fopen(bmp_path,"wb+");
    if(!fp)
    {
		free(rgb_24);
        printf(" Create bmp file:%s faled!\n", bmp_path);
        return -1;
    }

    fwrite(&BmpFileHeader,sizeof(BmpFileHeader),1,fp);

    fwrite(&BmpInfoHeader,sizeof(BmpInfoHeader),1,fp);

    fwrite(rgb_24,width*height*3,1,fp);

    free(rgb_24);

    fclose(fp);

    return 0;
}
