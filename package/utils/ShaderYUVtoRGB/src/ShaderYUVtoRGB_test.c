#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/fb.h>
#include "ShaderYUVtoRGB.h"

#define SRC_WIDTH 640
#define SRC_HEIGHT 480
#define DST_X 0
#define DST_Y 0
#define DST_WIDTH 1280
#define DST_HEIGHT 800

Dstresource dstresource;
Srcresource srcresource;
void *out_addr_vir;
void *input_addr_vir;
int input_fd;
int output_fd;

void Init_Resource(void)
{
	srcresource.srcwidth = SRC_WIDTH;
	srcresource.srcheight = SRC_HEIGHT;
	srcresource.srcformat = FORMAT_NV21;
	dstresource.dstx = DST_X;
	dstresource.dsty = DST_Y;
	dstresource.dstwidth = DST_WIDTH;
	dstresource.dstheight = DST_HEIGHT;
	dstresource.dstformat = FORMAT_ARGB8888;
}

static int init_dma_buf(char *path, int size)
{
        int ret;
        int ion_fd = -1;
	int file_fd;
	void* addr_file,*addr_dma;
        printf("deal######## %s ####size=%d #####.\n",path,size);
        sunxi_ion_alloc_open();
        input_addr_vir = sunxi_ion_alloc_palloc(size);
        input_fd = sunxi_ion_alloc_get_bufferFd(input_addr_vir);

        out_addr_vir = sunxi_ion_alloc_palloc(dstresource.dstwidth*dstresource.dstheight*4);
        output_fd = sunxi_ion_alloc_get_bufferFd(out_addr_vir);

        file_fd = open(path, O_RDWR);
        if (file_fd < 0) {
                printf("open error\n");
        }
        addr_file = (void *)mmap((void *)0, size, PROT_WRITE, MAP_SHARED, file_fd, 0);
        memcpy(input_addr_vir, addr_file , size);
        printf("%d\n", *(unsigned long*)input_addr_vir);
        printf("%d\n", &input_addr_vir[1]);
        printf("%d\n", &input_addr_vir[2]);
        printf("%d\n", &input_addr_vir[3]);

        printf("%d\n", *(unsigned long *)addr_file);
        printf("%d\n", &addr_file[1]);
        printf("%d\n", &addr_file[2]);
        printf("%d\n", &addr_file[3]);

	sunxi_ion_alloc_flush_cache(input_addr_vir, size);
	return input_fd;
}

int main()
{
	FILE *fp;
	int dma_fd;
	Init_Resource();
	dma_fd = init_dma_buf("/pic_bin/source_NV21.yuv", srcresource.srcwidth * srcresource.srcheight * 3/2);
	InitResource();
	ShaderYUVtoRGB_toionbuf(dma_fd, output_fd, &srcresource, &dstresource);
	/*ShaderYUVtoRGB_tofb(dma_fd, &srcresource, &dstresource);*/
	DestoryResource();
	if((fp = fopen("/mnt/write", "w")) == NULL){
        	printf("cannot open file\n");
        	exit(0);
	}
	fwrite(out_addr_vir, dstresource.dstwidth*dstresource.dstheight*4, 1, fp);
	fclose(fp);
        return 0;
}

