#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <ion_mem_alloc.h>
#include <videoOutPort.h>

char* pPhyBuf;
unsigned int fbAddr[3];
videoParam vparam;
renderBuf rBuf;

dispOutPort *mDispOutPort;
struct SunxiMemOpsS *pMemops;

struct test_layer_info
{
	int fb_width, fb_height;
	int width,height;//screen size
	int dispfh;//device node handle
	int fh;//picture resource file handle
    int format;
	int buffer_num;//is double buffer
	int clear;//is clear layer
	char filename[32];
    char formatname[32];
};
struct test_layer_info test_info;
/* Signal handler */
static void terminate(int sig_no)
{
	int val[6];
	memset(val,0,sizeof(val));
	printf("Got signal %d, exiting ...\n", sig_no);
	if(test_info.dispfh == -1)
	{
			printf("EXIT:");
			exit(1);
	}

	close(test_info.dispfh);
	printf("EXIT:");
	exit(1);
}

int parse_cmdline(int argc, char **argv, struct test_layer_info *p)
{
	int err = 0;
	int i = 0;
    int format;

	i = 0;
	while(i < argc) {
		if ( ! strcmp(argv[i], "-format")) {
            char nv21[32] = "nv21";
            char nv12[32] = "nv12";
            char i420[32] = "i420";
            char yv12[32] = "yv12";
            char mb32[32] = "mb32";
			char yuyv[32] = "yuyv";
			if (argc > i+1) {
				i++;
				p->formatname[0] = '\0';
				sprintf(p->formatname,"%s",argv[i]);
				printf("formatname=%s\n", p->formatname);

                if(strcmp(p->formatname, nv21) == 0)
                    p->format = VIDEO_PIXEL_FORMAT_NV21;
                else if(strcmp(p->formatname, nv12) == 0)
                    p->format = VIDEO_PIXEL_FORMAT_NV12;
                else if(strcmp(p->formatname, i420) == 0)
                    p->format = VIDEO_PIXEL_FORMAT_YUV_PLANER_420;
                else if(strcmp(p->formatname, yv12) == 0)
                    p->format = VIDEO_PIXEL_FORMAT_YV12;
                else if(strcmp(p->formatname, mb32) == 0)
                    p->format = VIDEO_PIXEL_FORMAT_YUV_MB32_420;
				else if(strcmp(p->formatname, yuyv) == 0)
					p->format = VIDEO_PIXEL_FORMAT_YUYV;
                else
                    p->format = VIDEO_PIXEL_FORMAT_DEFAULT;
            }
        }

		if ( ! strcmp(argv[i], "-file")) {
			if (argc > i+1) {
				i++;
				p->filename[0] = '\0';
				sprintf(p->filename,"%s",argv[i]);
				printf("filename=%s\n", p->filename);
			}	else {
				printf("no file described!!\n");
				err ++;
			}
		}

		if ( ! strcmp(argv[i], "-size")) {
			if (argc > i+2) {
				i++;
				p->fb_width = atoi(argv[i]);
				i++;
				p->fb_height = atoi(argv[i]);
			}	else {
				printf("-size para err!\n\n");
				err ++;
			}
		}

		if ( ! strcmp(argv[i], "-buffer_num")) {
			if (argc > i+1) {
				i++;
				p->buffer_num = atoi(argv[i]);
				p->buffer_num = (p->buffer_num == 0)? 1:p->buffer_num;
            }
        }
		i++;
    }

	if(err > 0) {
        printf("For example\n");
		printf("open image : yuvtest -format nv21 -size 800 480 -buffer_num 1 -file /pic/pic_nv21_600x480_1.dat\n");
		return -1;
	} else
		 return 0;
}

static void install_sig_handler(void)
{
	signal(SIGBUS, terminate);
	signal(SIGFPE, terminate);
	signal(SIGHUP, terminate);
	signal(SIGILL, terminate);
	signal(SIGINT, terminate);
	signal(SIGIOT, terminate);
	signal(SIGPIPE, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGSEGV, terminate);
	signal(SIGSYS, terminate);
	signal(SIGTERM, terminate);
	signal(SIGTRAP, terminate);
	signal(SIGUSR1, terminate);
	signal(SIGUSR2, terminate);
}

static void addrSetting();

int main(int argc, char **argv)
{
	int rv;
	int fb_width,fb_height;

	VoutRect rect;
    int enable = 1;
	int rotate = 0;
	char* pVirBuf;

	install_sig_handler();
	memset(&test_info, 0, sizeof(struct test_layer_info));
    test_info.clear = 0;
    test_info.buffer_num = 1;
    test_info.format = VIDEO_PIXEL_FORMAT_DEFAULT;
	rv = parse_cmdline(argc,argv, &test_info);
	if(rv < 0) {
		printf("layer_request:parse_command");
		return -1;
	}

    if(test_info.clear == 1)
        enable = 0;

	mDispOutPort = CreateVideoOutport(0);
	if(mDispOutPort == NULL){
		printf("CreateVideoOutport ERR");
		return -1;
	}
	rect.x = 0;
	rect.y = 0;
	rect.width = test_info.fb_width;
	rect.height = test_info.fb_height;
	mDispOutPort->init(mDispOutPort, enable, rotate, &rect);

	rect.x = 0;
	rect.y = 0;
	rect.width = mDispOutPort->getScreenWidth(mDispOutPort);
	rect.height = mDispOutPort->getScreenHeight(mDispOutPort);
	mDispOutPort->setRect(mDispOutPort,&rect);

	fb_width = test_info.fb_width;
	fb_height = test_info.fb_height;

	pMemops = GetMemAdapterOpsS();
    SunxiMemOpen(pMemops);
	pVirBuf = (char*)SunxiMemPalloc(pMemops, fb_width * fb_height * 4 * test_info.buffer_num);
	pPhyBuf = (char*)SunxiMemGetPhysicAddressCpu(pMemops, pVirBuf);
	printf("--vir_adr=0x%08p phy_adr=0x%08p\n", pVirBuf, pPhyBuf);
	if((test_info.fh = open(test_info.filename, O_RDONLY)) == -1) {
		printf("open file %s fail. \n", test_info.filename);
	}
	memset((void*)pVirBuf, 0x0, fb_width*fb_height*4*test_info.buffer_num);
	if(test_info.fh != -1)
		read(test_info.fh, (void*)pVirBuf, fb_width*fb_height*4*test_info.buffer_num);

	SunxiMemFlushCache(pMemops, (void*)pVirBuf, fb_width*fb_height*4*test_info.buffer_num);

    addrSetting();

	vparam.srcInfo.crop_x = 0;
	vparam.srcInfo.crop_y = 0;
	vparam.srcInfo.crop_w = fb_width;
	vparam.srcInfo.crop_h = fb_height;

	vparam.srcInfo.w = fb_width;
	vparam.srcInfo.h = fb_height;
	vparam.srcInfo.color_space = VIDEO_BT601;

    printf("format = %d\n",vparam.srcInfo.format);
    printf("y_phaddr = 0x%08p\n",rBuf.y_phaddr);
    printf("v_phaddr = 0x%08p\n",rBuf.v_phaddr);
    printf("u_phaddr = 0x%08p\n",rBuf.u_phaddr);
    rBuf.isExtPhy = VIDEO_USE_EXTERN_ION_BUF;
	mDispOutPort->queueToDisplay(mDispOutPort, vparam.srcInfo.w*vparam.srcInfo.h*4, &vparam, &rBuf);
	mDispOutPort->SetZorder(mDispOutPort, VIDEO_ZORDER_MIDDLE);
	mDispOutPort->setEnable(mDispOutPort, 1);

	SunxiMemPfree(pMemops, pVirBuf);
	SunxiMemClose(pMemops);

	close(test_info.fh);

	return 0;
}

static void addrSetting(void) {
    int width;
    int height;
    int format;

    width = test_info.fb_width;
    height = test_info.fb_height;
    format = test_info.format;

    vparam.srcInfo.format = test_info.format;
    switch(format){
        case VIDEO_PIXEL_FORMAT_NV21 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_NV12 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_YUV_MB32_420 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_YUV_PLANER_420 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[0] + width * height * 5 / 4);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[1];
            rBuf.v_phaddr = fbAddr[2];
            break;

        case VIDEO_PIXEL_FORMAT_YV12 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[0] + width * height * 5 / 4);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[2];
            rBuf.v_phaddr = fbAddr[1];
            break;
        case VIDEO_PIXEL_FORMAT_YUYV:
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[1] + width * height * 2);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[1];
            rBuf.v_phaddr = fbAddr[2];
        	break;

        default :
            fbAddr[0] = (unsigned long)pPhyBuf;
            rBuf.y_phaddr = fbAddr[0];
            break;
    }
}
