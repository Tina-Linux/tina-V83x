
#include "sunxi_vop.h"
#include "edid/sunxi_edid.h"
#include "edid/cea_vic.h"
#include "sunxi_display2.h"

#define true 1
#define false 0
#define TABLE_LINE 48
//#define DRV_DEBUG	printf
#define DRV_DEBUG(format, ...) do{}while(0);
#define ERROR		printf

static int g_fd = -1;
static SunxiEdidHandle g_edid = NULL;

/* hdmi device */
static struct disp_device_config g_config;
static pthread_mutex_t g_hdmi_mutex;
static bool g_hdmi_enable;		//enable flag for hdmi
static bool g_hdmi_open;		//open flag for hdmi

/* cvbs device, only need type and mode for hardware */
SUNXIVOPVideoFormat_e g_cvbs_mode = SUNXI_VIDEO_FORMAT_PAL;
static pthread_mutex_t g_cvbs_mutex;
static bool g_cvbs_enable;		//enable flag for cvbs
static bool g_cvbs_open;		//open flag for cvbs

static pthread_mutex_t g_cnt_mutex;	//protect g_cvbs_enable/g_hdmi_enable

/* SUNXIVOPVideoFormat_e <=> enum disp_tv_mode */
static unsigned char g_cvbs_match_table[TABLE_LINE][2] = {
	{SUNXI_VIDEO_FORMAT_NTSC, DISP_TV_MOD_NTSC},
	{SUNXI_VIDEO_FORMAT_PAL, DISP_TV_MOD_PAL},
};

/* SUNXIVOPVideoFormat_e <=> enum disp_tv_mode */
static unsigned char g_hdmi_match_table[TABLE_LINE][2] = {
	{SUNXI_VIDEO_FORMAT_480P, DISP_TV_MOD_480P},
	{SUNXI_VIDEO_FORMAT_576P, DISP_TV_MOD_576P},
	{SUNXI_VIDEO_FORMAT_720P_50HZ, DISP_TV_MOD_720P_50HZ},
	{SUNXI_VIDEO_FORMAT_720P, DISP_TV_MOD_720P_60HZ},
	{SUNXI_VIDEO_FORMAT_1080I_50HZ, DISP_TV_MOD_1080I_50HZ},
	{SUNXI_VIDEO_FORMAT_1080P_24HZ, DISP_TV_MOD_1080P_24HZ},
	{SUNXI_VIDEO_FORMAT_1080P_25HZ, DISP_TV_MOD_1080P_25HZ},
	{SUNXI_VIDEO_FORMAT_1080P_30HZ, DISP_TV_MOD_1080P_30HZ},
	{SUNXI_VIDEO_FORMAT_1080P_50HZ, DISP_TV_MOD_1080P_50HZ},
	{SUNXI_VIDEO_FORMAT_1080P_60HZ, DISP_TV_MOD_1080P_60HZ},
	{SUNXI_VIDEO_FORMAT_3840x2160P_24HZ, DISP_TV_MOD_3840_2160P_24HZ},
	{SUNXI_VIDEO_FORMAT_3840x2160P_25HZ, DISP_TV_MOD_3840_2160P_25HZ},
	{SUNXI_VIDEO_FORMAT_3840x2160P_30HZ, DISP_TV_MOD_3840_2160P_30HZ},
	{SUNXI_VIDEO_FORMAT_3840x2160P_50HZ, DISP_TV_MOD_3840_2160P_50HZ},
	{SUNXI_VIDEO_FORMAT_3840x2160P_60HZ, DISP_TV_MOD_3840_2160P_60HZ},
	{SUNXI_VIDEO_FORMAT_4096x2160P_24HZ, DISP_TV_MOD_4096_2160P_24HZ},
	{SUNXI_VIDEO_FORMAT_4096x2160P_25HZ, DISP_TV_MOD_4096_2160P_25HZ},
	{SUNXI_VIDEO_FORMAT_4096x2160P_30HZ, DISP_TV_MOD_4096_2160P_30HZ},
	{SUNXI_VIDEO_FORMAT_4096x2160P_50HZ, DISP_TV_MOD_4096_2160P_50HZ},
	{SUNXI_VIDEO_FORMAT_4096x2160P_60HZ, DISP_TV_MOD_4096_2160P_60HZ},
	{SUNXI_VIDEO_FORMAT_3D_1080P_24HZ, DISP_TV_MOD_1080P_24HZ_3D_FP},
};

/* SUNXIVOPVideoFormat_e <=> enum hdmi_vic */
static unsigned char g_vic_match_table[TABLE_LINE][2] = {
	{SUNXI_VIDEO_FORMAT_480P, HDMI_720x480p60_16x9},
	{SUNXI_VIDEO_FORMAT_576P, HDMI_720x576p50_16x9},
	{SUNXI_VIDEO_FORMAT_720P_50HZ, HDMI_1280x720p50_16x9},
	{SUNXI_VIDEO_FORMAT_720P, HDMI_1280x720p60_16x9},
	{SUNXI_VIDEO_FORMAT_1080I_50HZ, HDMI_1920x1080i50_16x9},
	{SUNXI_VIDEO_FORMAT_1080P_24HZ, HDMI_1920x1080p24_16x9},
	{SUNXI_VIDEO_FORMAT_1080P_25HZ, HDMI_1920x1080p25_16x9},
	{SUNXI_VIDEO_FORMAT_1080P_30HZ, HDMI_1920x1080p30_16x9},
	{SUNXI_VIDEO_FORMAT_1080P_50HZ, HDMI_1920x1080p50_16x9},
	{SUNXI_VIDEO_FORMAT_1080P_60HZ, HDMI_1920x1080p60_16x9},
	{SUNXI_VIDEO_FORMAT_3840x2160P_24HZ, HDMI_3840x2160p24_16x9},
	{SUNXI_VIDEO_FORMAT_3840x2160P_25HZ, HDMI_3840x2160p25_16x9},
	{SUNXI_VIDEO_FORMAT_3840x2160P_30HZ, HDMI_3840x2160p30_16x9},
	{SUNXI_VIDEO_FORMAT_3840x2160P_50HZ, HDMI_3840x2160p50_16x9},
	{SUNXI_VIDEO_FORMAT_3840x2160P_60HZ, HDMI_3840x2160p60_16x9},
	{SUNXI_VIDEO_FORMAT_4096x2160P_24HZ, HDMI_4096x2160p24_256x135},
	{SUNXI_VIDEO_FORMAT_4096x2160P_25HZ, HDMI_4096x2160p25_256x135},
	{SUNXI_VIDEO_FORMAT_4096x2160P_30HZ, HDMI_4096x2160p30_256x135},
	{SUNXI_VIDEO_FORMAT_4096x2160P_50HZ, HDMI_4096x2160p50_256x135},
	{SUNXI_VIDEO_FORMAT_4096x2160P_60HZ, HDMI_4096x2160p60_256x135},
};

/* enum hdmi_vic => SUNXIVOPVideoFormat_e */
static signed char match_vic(enum hdmi_vic vic, unsigned char (*tmp)[2])
{
	unsigned char i;

	for (i = 0; i < TABLE_LINE; i++) {
		if (vic == tmp[i][1])
			return i;
	}

	return -1;
}

/* SUNXIVOPVideoFormat_e => enum hdmi_vic */
static signed char reverse_vic(SUNXIVOPVideoFormat_e format, unsigned char (*tmp)[2])
{
	unsigned char i;

	for (i = 0; i < TABLE_LINE; i++) {
		if (format == tmp[i][0])
			return i;
	}

	return -1;
}

/* SUNXIVOPVideoFormat_e => enum disp_tv_mode */
static signed char match_table(SUNXIVOPVideoFormat_e format, unsigned char (*tmp)[2])
{
	unsigned char i;

	for (i = 0; i < TABLE_LINE; i++) {
		if (format == tmp[i][0])
			return i;
	}

	return -1;
}

/* enum disp_tv_mode => SUNXIVOPVideoFormat_e */
static signed char reverse_table(enum disp_tv_mode mode, unsigned char (*tmp)[2])
{
	unsigned char i;

	for (i = 0; i < TABLE_LINE; i++) {
		if (mode == tmp[i][1])
			return i;
	}

	return -1;
}

/* SUNXIVOPDevice_e => enum disp_output_type */
static enum disp_output_type match_type(SUNXIVOPDevice_e device)
{
	enum disp_output_type type;

	if (device == SUNXI_OUTPUT_DEVICE_HDMI)
		type = DISP_OUTPUT_TYPE_HDMI;
	else if (device == SUNXI_OUTPUT_DEVICE_CVBS)
		type = DISP_OUTPUT_TYPE_TV;
	else
		type = DISP_OUTPUT_TYPE_NONE;

	return type;
}

/* SUNXIVOPColorSpace_e => enum disp_csc_type */
static int match_cs(SUNXIVOPColorSpace_e cs)
{
	int type;

	if (cs == SUNXI_COLOR_SPACE_RGB)
		type = DISP_CSC_TYPE_RGB;
	else if (cs == SUNXI_COLOR_SPACE_YCBCR_420)
		type = DISP_CSC_TYPE_YUV420;
	else if (cs == SUNXI_COLOR_SPACE_YCBCR_422)
		type = DISP_CSC_TYPE_YUV422;
	else if (cs == SUNXI_COLOR_SPACE_YCBCR_444)
		type = DISP_CSC_TYPE_YUV444;
	else {
		ERROR("not support color space: %d\n", cs);
		type = -1;
	}

	return type;
}

/* enum disp_csc_type => SUNXIVOPColorSpace_e */
static int reverse_cs(enum disp_csc_type type)
{
	signed int cs;

	if (type == DISP_CSC_TYPE_RGB)
		cs = SUNXI_COLOR_SPACE_RGB;
	else if (type == DISP_CSC_TYPE_YUV420)
		cs = SUNXI_COLOR_SPACE_YCBCR_420;
	else if (type == DISP_CSC_TYPE_YUV422)
		cs = SUNXI_COLOR_SPACE_YCBCR_422;
	else if (type == DISP_CSC_TYPE_YUV444)
		cs = SUNXI_COLOR_SPACE_YCBCR_444;
	else {
		cs  = -1;
	}

	return cs;
}


/* SUNXIVOPDevice_e =>  screen_id */
static int match_screen_id(SUNXIVOPDevice_e device)
{
	if (device == SUNXI_OUTPUT_DEVICE_HDMI)
		/* primary disp */
		return 0;
	else
		/* extend disp */
		return 1;
}

/* open the node of /dev/disp, set default devise param to g_config */
static s32 SUNXIVOPOpenDevice(SUNXIVOPDevice_e device)
{
	unsigned long args[4] = {0};
	struct disp_device_config config;

	DRV_DEBUG("SUNXIVOPOpenDevice, device=%d\n", device);
	if (g_fd == -1) {
		if ((g_fd = open("/dev/disp", O_RDWR)) == -1) {
			ERROR("open /dev/disp failed!\n");
			return -1;
		}
		pthread_mutex_init(&g_cnt_mutex, NULL);
	}

	/* handle for cvbs */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		if (g_cvbs_open == false) {
			g_cvbs_open = true;
			pthread_mutex_init(&g_cvbs_mutex, NULL);
		}
		return 0;
	}

	/* handle for hdmi */
	if (g_hdmi_open == false) {
		g_hdmi_open = true;
		pthread_mutex_init(&g_hdmi_mutex, NULL);
	} else
		return 0;
	memset(&config, 0, sizeof(struct disp_device_config));
	args[0] = match_screen_id(device);
	args[1] = (unsigned long)&config;
	if (ioctl(g_fd, DISP_DEVICE_GET_CONFIG, (void *)args) < 0) {
		g_config.mode = HDMI_DEFAULT_MODE;
		g_config.format = DISP_CSC_TYPE_YUV444;
	} else {
		g_config.mode = config.mode;
		g_config.format = config.format;
	}

	g_config.bits = DISP_DATA_8BITS;
	g_config.eotf = DISP_EOTF_GAMMA22;
	g_config.range = DISP_COLOR_RANGE_16_235;
	g_config.scan = DISP_SCANINFO_NO_DATA;
	g_config.aspect_ratio = 4;
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		g_config.type = DISP_OUTPUT_TYPE_TV;
		g_config.cs = DISP_BT601_F;
	} else {
		g_config.type = DISP_OUTPUT_TYPE_HDMI;
		g_config.cs = DISP_BT709;
	}

	return 0;
}

static void SUNXIVOPCloseDevice(SUNXIVOPDevice_e device)
{
	if (device == SUNXI_OUTPUT_DEVICE_HDMI) {
		g_hdmi_open = false;
		pthread_mutex_lock(&g_cnt_mutex);
		g_hdmi_enable = false;
		pthread_mutex_unlock(&g_cnt_mutex);
		pthread_mutex_destroy(&g_hdmi_mutex);
		if (g_edid != NULL) {
			SunxiEdidUnload(g_edid);
			g_edid = NULL;
		}
	} else {
		g_cvbs_open = false;
		pthread_mutex_lock(&g_cnt_mutex);
		g_cvbs_enable = false;
		pthread_mutex_unlock(&g_cnt_mutex);
		pthread_mutex_destroy(&g_cvbs_mutex);
	}

	if (g_fd != -1 && g_hdmi_open == false && g_cvbs_open == false) {
		pthread_mutex_destroy(&g_cnt_mutex);
		close(g_fd);
		g_fd = -1;
	}
}

/* enable disp output */
int SUNXIVOPEnable(SUNXIVOPDevice_e device)
{
	unsigned long args[4] = {0};
	int ret;

	DRV_DEBUG("SUNXIVOPEnable, device=%d\n", device);
	if (g_cvbs_open == false || g_hdmi_open == false)
		SUNXIVOPOpenDevice(device);

	/* handle for cvbs */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		struct disp_device_config config;
		enum disp_tv_mode mode;
		signed char ret = 0;

		pthread_mutex_lock(&g_cvbs_mutex);
		if ((ret = match_table(g_cvbs_mode, g_cvbs_match_table)) != -1)
			mode = (enum disp_tv_mode)g_cvbs_match_table[(unsigned char)ret][1];
		else {
			ERROR("SUNXIVOPEnable, not support format=%d\n", g_cvbs_mode);
			pthread_mutex_unlock(&g_cvbs_mutex);
			return -1;
		}
		config.type = DISP_OUTPUT_TYPE_TV;
		config.mode = mode;
		args[0] = match_screen_id(device);
		args[1] = (unsigned long)&config;
		DRV_DEBUG("SUNXIVOPEnable Information:\n");
		DRV_DEBUG("screen%d\n", match_screen_id(device));
		DRV_DEBUG("config.type=%d\n", config.type);
		DRV_DEBUG("config.mode=%d\n", config.mode);
		if (ioctl(g_fd, DISP_DEVICE_SET_CONFIG, (void *)args) < 0) {
			ERROR("SUNXIVOPEnable, ioctl:DISP_DEVICE_SET_CONFIG is failed.\n");
			pthread_mutex_unlock(&g_cvbs_mutex);
			return -1;
		}
		pthread_mutex_unlock(&g_cvbs_mutex);

		g_cvbs_enable = true;
	}

	/* handle for hdmi */
	if (g_edid == NULL)
		g_edid = SunxiEdidLoad("/sys/class/hdmi/hdmi/attr/edid");
	else
		SunxiEdidReload(g_edid);

	ret = SunxiEdidGetSinkType(g_edid);
	pthread_mutex_lock(&g_hdmi_mutex);
	if (ret== 2)
		g_config.dvi_hdmi = 2;
	else
		g_config.dvi_hdmi = 1;
	if (device == SUNXI_OUTPUT_DEVICE_HDMI)
		g_config.type = DISP_OUTPUT_TYPE_HDMI;
	else
		g_config.type = DISP_OUTPUT_TYPE_TV;
	DRV_DEBUG("SUNXIVOPEnable Information:\n");
	DRV_DEBUG("screen%d\n", match_screen_id(device));
	DRV_DEBUG("g_config.type=%d\n", g_config.type);
	DRV_DEBUG("g_config.mode=%d\n", g_config.mode);
	DRV_DEBUG("g_config.format=%d\n", g_config.format);
	DRV_DEBUG("g_config.bits=%d\n", g_config.bits);
	DRV_DEBUG("g_config.eotf=%d\n", g_config.eotf);
	DRV_DEBUG("g_config.cs=%d\n", g_config.cs);
	DRV_DEBUG("g_config.dvi_hdmi=%d\n", g_config.dvi_hdmi);
	DRV_DEBUG("g_config.range=%d\n", g_config.range);
	DRV_DEBUG("g_config.scan=%d\n", g_config.scan);
	DRV_DEBUG("g_config.aspect_ratio=%d\n", g_config.aspect_ratio);
	args[0] = match_screen_id(device);
	args[1] = (unsigned long)&g_config;
	if (ioctl(g_fd, DISP_DEVICE_SET_CONFIG, (void *)args) < 0) {
		ERROR("SUNXIVOPEnable, ioctl:DISP_DEVICE_SET_CONFIG is failed.\n");
		pthread_mutex_unlock(&g_hdmi_mutex);
		return -1;
	}
	pthread_mutex_unlock(&g_hdmi_mutex);

	g_hdmi_enable = true;

	return 0;
}

/* disable disp output */
int SUNXIVOPDisable(SUNXIVOPDevice_e device)
{
	unsigned long args[4] = {0};

	DRV_DEBUG("SUNXIVOPDisable, device=%d\n", device);
	/* if a device not enable by app , return 0, here */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		if (g_cvbs_enable == false) {
			ERROR("SUNXIVOPDisable, app not enable cvbs device\n");
			return 0;
		}
	} else {
		if (g_hdmi_enable == false) {
			ERROR("SUNXIVOPDisable, app not enable hdmi device\n");
			return 0;
		}
	}

	if (g_cvbs_open == false || g_hdmi_open == false)
		SUNXIVOPOpenDevice(device);

	args[0] = match_screen_id(device);
	args[1] = (unsigned long)DISP_OUTPUT_TYPE_NONE;
	if (ioctl(g_fd, DISP_DEVICE_SWITCH, (void *)args) < 0) {
		ERROR("SUNXIVOPDisable, ioctl:DISP_DEVICE_SWITCH is failed. type = %d\n", match_type(device));
		return -1;
	}

	/* handle for cvbs */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		pthread_mutex_lock(&g_cnt_mutex);
		g_cvbs_enable = false;
		pthread_mutex_unlock(&g_cnt_mutex);
	} else {
		/* handle for hdmi */
		pthread_mutex_lock(&g_cnt_mutex);
		g_config.type = DISP_OUTPUT_TYPE_NONE;
		g_hdmi_enable = false;
		pthread_mutex_unlock(&g_cnt_mutex);
	}

	return 0;
}

int SUNXIVOPSetColorSpace(SUNXIVOPDevice_e device, SUNXIVOPColorSpace_e colorspace)
{
	DRV_DEBUG("SUNXIVOPSetColorSpace, device=%d\n", device);
	if (g_cvbs_open == false || g_hdmi_open == false)
		SUNXIVOPOpenDevice(device);

	/* handle for cvbs */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS)
		return 0;

	/* handle for hdmi */
	if (match_cs(colorspace) == -1) {
		ERROR("SUNXIVOPSetColorSpace: colorspace=%d is not support\n", colorspace);
		return -1;
	} else {
		pthread_mutex_lock(&g_hdmi_mutex);
		g_config.format = (enum disp_csc_type)match_cs(colorspace);
		pthread_mutex_unlock(&g_hdmi_mutex);

		pthread_mutex_lock(&g_cnt_mutex);
		if (g_hdmi_enable) {
			SUNXIVOPEnable(device);
		}
		pthread_mutex_unlock(&g_cnt_mutex);
		return 0;
	}
}

int SUNXIVOPSetVideoFormat(SUNXIVOPDevice_e device, SUNXIVOPVideoFormat_e format)
{
	struct disp_device_config config;
	unsigned long args[4] = {0};
	enum disp_tv_mode mode;
	signed char ret = 0;
	int cs = 0;
	int index;

	DRV_DEBUG("SUNXIVOPSetVideoFormat, device=%d, g_hdmi_open=%d, g_cvbs_open=%d, g_hdmi_enable=%d, g_cvbs_enable=%d, format=%d\n",
			device, g_hdmi_open, g_cvbs_open, g_hdmi_enable, g_cvbs_enable, format);
	if (g_cvbs_open == false|| g_hdmi_open == false)
		SUNXIVOPOpenDevice(device);

	/* handle for cvbs */
	if (device == SUNXI_OUTPUT_DEVICE_CVBS) {
		pthread_mutex_lock(&g_cvbs_mutex);
		g_cvbs_mode = format;
		pthread_mutex_unlock(&g_cvbs_mutex);

		pthread_mutex_lock(&g_cnt_mutex);
		if (g_cvbs_enable) {
			SUNXIVOPEnable(device);
		}
		pthread_mutex_unlock(&g_cnt_mutex);

		return 0;
	}

	/* handle for hdmi */
	if (g_edid == NULL)
		g_edid = SunxiEdidLoad("/sys/class/hdmi/hdmi/attr/edid");
	else
		SunxiEdidReload(g_edid);

	if (device == SUNXI_OUTPUT_DEVICE_HDMI) {
		cs = DISP_BT709;
		if ((ret = match_table(format, g_hdmi_match_table)) != -1)
			mode = (enum disp_tv_mode)g_hdmi_match_table[(unsigned char)ret][1];
		else {
			mode = HDMI_DEFAULT_MODE;
			ERROR("not support format: [need=%d match=%d]", format, ret);
		}
	} else {
		ERROR("not support disp type: device-type=%d", device);
		return -1;
	}
	/* for 4k, give priority to YUV420 */
	if ((SunxiEdidGetSinkType(g_edid) == 1) || (SunxiEdidSupportedRGBOnly(g_edid))) {
		config.format = DISP_CSC_TYPE_RGB;
	} else if (mode >= DISP_TV_MOD_3840_2160P_30HZ && mode <= DISP_TV_MOD_4096_2160P_50HZ) {
		index = reverse_vic(format, g_vic_match_table);
		if (index == -1) {
			ERROR("SUNXIVOPSetVideoFormat: not support format = %d\n", format);
			return -1;
		} else if (SunxiEdidIsSupportY420Sampling(g_edid, g_vic_match_table[index][1])) {
			config.format = DISP_CSC_TYPE_YUV420;
		} else if (SunxiEdidIsSupportRegularSampling(g_edid, g_vic_match_table[index][1])) {
			config.format = DISP_CSC_TYPE_YUV444;
		} else {
			ERROR("SUNXIVOPSetVideoFormat: not support match sampling, format = %d\n", format);
			return -1;
		}
	} else {
		config.format = DISP_CSC_TYPE_YUV444;
	}

	pthread_mutex_lock(&g_hdmi_mutex);
	g_config.mode = mode;
	g_config.cs = (enum disp_color_space)cs;
	g_config.format = config.format;
	pthread_mutex_unlock(&g_hdmi_mutex);

	pthread_mutex_lock(&g_cnt_mutex);
	if (g_hdmi_enable) {
		SUNXIVOPEnable(device);
	}
	pthread_mutex_unlock(&g_cnt_mutex);

	return 0;
}

int SUNXIVOPGetHDMIMonitorStatus(SUNXIVOPHDMIMonitoStatus *pstatus)
{
	unsigned long args[4] = {0};
	int hdmi_fd = -1, index = -1, mode, cs;
	struct disp_device_config config;
	int *tmp = NULL;
	int format_index = 0, i;
	int fd;
	int cnt = 0;

	if (g_fd == -1 || g_hdmi_open == false)
		SUNXIVOPOpenDevice(SUNXI_OUTPUT_DEVICE_HDMI);

	if (g_edid == NULL)
		g_edid = SunxiEdidLoad("/sys/class/hdmi/hdmi/attr/edid");
	else
		SunxiEdidReload(g_edid);

	pstatus->AspectRatio = SUNXI_ASPECT_RATIO_16x9;
	if ((hdmi_fd = open("/sys/class/switch/hdmi/state", O_RDONLY)) != -1) {
		char val = 0;

		if (read(hdmi_fd, &val, 1) == 1 && val == '1') {
			pstatus->Connect = 1;
		} else
			pstatus->Connect = 0;
		close(hdmi_fd);
	}

	fd = open("/dev/cec", O_RDWR);
	if (fd < 0) {
		ERROR("Warring: open /dev/cec failed\n");
		pstatus->PhysicalAddr[0] = 0;
		pstatus->PhysicalAddr[1] = 0;
		pstatus->PhysicalAddr[2] = 0;
		pstatus->PhysicalAddr[3] = 0;
		close(fd);
	} else {
		#define CEC_G_PHYS_ADDR             _IOR('c', 2, unsigned short)
		unsigned long arg[4];
		unsigned long phyaddr = 0;
		arg[0] = (unsigned long)(&phyaddr);
		ioctl(fd, CEC_G_PHYS_ADDR, arg);
		pstatus->PhysicalAddr[0] = (unsigned char)(phyaddr >> 12) & 0x0f;
		pstatus->PhysicalAddr[1] = (unsigned char)(phyaddr >> 8) & 0x0f;
		pstatus->PhysicalAddr[2] = (unsigned char)(phyaddr >> 4) & 0x0f;
		pstatus->PhysicalAddr[3] = (unsigned char)(phyaddr) & 0x0f;
		close(fd);
	}

	if (SunxiEdidGetSinkType(g_edid) == 2)
		pstatus->IsHdmiDevice = 1;
	else
		pstatus->IsHdmiDevice = 0;

	pthread_mutex_lock(&g_hdmi_mutex);
	if (pstatus->IsHdmiDevice)
		g_config.dvi_hdmi = 2;
	else
		g_config.dvi_hdmi = 1;

	mode = g_config.mode;
	index = reverse_table((enum disp_tv_mode)mode , g_hdmi_match_table);
	if (index != -1) {
		pstatus->VideoFormat = (SUNXIVOPVideoFormat_e)g_hdmi_match_table[index][0];
	} else {
		ERROR("can not get VideoFormat, return VideoFormat = HDMI_DEFAULT_MODE\n");
		pstatus->VideoFormat = (SUNXIVOPVideoFormat_e)HDMI_DEFAULT_MODE;
	}

	cs = reverse_cs(g_config.format);
	if (cs != -1) {
		pstatus->ColorSpace = (SUNXIVOPColorSpace_e)cs;
	} else {
		ERROR("can not get ColorSpace, return ColorSpace = HDMI_DEFAULT_CS\n");
		pstatus->ColorSpace = HDMI_DEFAULT_CS;
	}
	pthread_mutex_unlock(&g_hdmi_mutex);

	memcpy(pstatus->MonitorName, SunxiEdidGetMonitorName(g_edid), 14);
	pstatus->ScreenSize.Width = SunxiEdidGetDispSizeWidth(g_edid);
	pstatus->ScreenSize.Height = SunxiEdidGetDispSizeHeight(g_edid);

	tmp = SunxiEdidGetSupportedVideoFormat(g_edid);
	for (i =0; i < SUNXI_VIDEO_FORMAT_MAX; i++)
		pstatus->VideoFormatSupported[i] = 0;

	for (i = 0; (i < SUNXI_VIDEO_FORMAT_MAX) && (tmp[format_index] != 0); i++) {
		signed char ret;

		ret = match_vic(tmp[format_index], g_vic_match_table);
		if (ret != -1) {
			pstatus->VideoFormatSupported[cnt++] = g_vic_match_table[(unsigned char)ret][0];
			if (tmp[format_index] == SunxiEdidGetPreferredVideoFormat(g_edid))
				pstatus->PreferredVideoFormat = g_vic_match_table[(unsigned char)ret][0];
		}
		format_index++;
	}

	pstatus->MaxAudioPcmChannnels = SunxiEdidGetMaxAudioPcmChs(g_edid);

	return 0;
}
