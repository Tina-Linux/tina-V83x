
#ifndef SUNXI_VOP_H_
#define SUNXI_VOP_H_

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
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <sys/ioctl.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

#define HDMI_DEFAULT_CS SUNXI_COLOR_SPACE_YCBCR_444
#define HDMI_DEFAULT_MODE DISP_TV_MOD_1080P_30HZ
#define CVBS_DEFAULT_MODE DISP_TV_MOD_PAL

/**
 * @brief General purpose rectangle.
 */
typedef struct
{
    int X;         ///< x.
    int Y;         ///< y.
    int Width;     ///< Width.
    int Height;    ///< Height.
} SUNXIRect;

/**
 * @brief Video output devices.
 */
typedef enum
{
    SUNXI_OUTPUT_DEVICE_HDMI = 0,
    SUNXI_OUTPUT_DEVICE_CVBS,
} SUNXIVOPDevice_e;

/**
 * @brief Video output port color space.
 */
typedef enum
{
    SUNXI_COLOR_SPACE_UNKOWN = 0,   ///< UNKNOWN color space.
    SUNXI_COLOR_SPACE_RGB,          ///< RGB color space.
    SUNXI_COLOR_SPACE_YCBCR_420,    ///< YCbCr 420 color space.
    SUNXI_COLOR_SPACE_YCBCR_422,    ///< YCbCr 422 color space.
    SUNXI_COLOR_SPACE_YCBCR_444,    ///< YCbCr 444 color space.
} SUNXIVOPColorSpace_e;

/**
 * @brief Video aspect ratio
 */
typedef enum
{
    SUNXI_ASPECT_RATIO_UNKNOWN = 0, ///< Unknown.
    SUNXI_ASPECT_RATIO_AUTO,        ///< 4x3 for SD and 480p, 16x9 for HD.
    SUNXI_ASPECT_RATIO_4x3,         ///< 4x3.
    SUNXI_ASPECT_RATIO_16x9,        ///< 16x9.
    SUNXI_ASPECT_RATIO_MAX          ///< Max.
} SUNXIAspectRatio_e;

/**
 * @brief Video output format.
 */
typedef enum
{
    SUNXI_VIDEO_FORMAT_UNKNOWN = 0,                                 ///< unknown/not supported video format.
    SUNXI_VIDEO_FORMAT_NTSC,                                        ///< NTSC
    SUNXI_VIDEO_FORMAT_PAL,                                         ///< PAL
    SUNXI_VIDEO_FORMAT_480P,                                        ///< NTSC Progressive (27Mhz).
    SUNXI_VIDEO_FORMAT_576P,                                        ///< HD PAL Progressive 50hz for Australia.
    SUNXI_VIDEO_FORMAT_1080I,                                       ///< HD 1080 Interlaced.
    SUNXI_VIDEO_FORMAT_1080I_50HZ,                                  ///< European 50hz HD 1080.
    SUNXI_VIDEO_FORMAT_1080P_24HZ,                                  ///< HD 1080 Progressive, 24hz.
    SUNXI_VIDEO_FORMAT_1080P_25HZ,                                  ///< HD 1080 Progressive, 25hz.
    SUNXI_VIDEO_FORMAT_1080P_30HZ,                                  ///< HD 1080 Progressive, 30hz.
    SUNXI_VIDEO_FORMAT_1080P_50HZ,                                  ///< HD 1080 Progressive, 50hz.
    SUNXI_VIDEO_FORMAT_1080P_60HZ,                                  ///< HD 1080 Progressive, 60hz.
    SUNXI_VIDEO_FORMAT_1080P = SUNXI_VIDEO_FORMAT_1080P_60HZ,       ///< HD 1080 Progressive, 60hz.
    SUNXI_VIDEO_FORMAT_720P,                                        ///< HD 720 Progressive.
    SUNXI_VIDEO_FORMAT_720P_24HZ,                                   ///< HD 720p 24hz.
    SUNXI_VIDEO_FORMAT_720P_25HZ,                                   ///< HD 720p 25hz.
    SUNXI_VIDEO_FORMAT_720P_30HZ,                                   ///< HD 720p 30hz.
    SUNXI_VIDEO_FORMAT_720P_50HZ,                                   ///< HD 720p 50hz for Australia.
    SUNXI_VIDEO_FORMAT_3840x2160P_24HZ,                             ///< UHD 3840x2160 24Hz.
    SUNXI_VIDEO_FORMAT_3840x2160P_25HZ,                             ///< UHD 3840x2160 25Hz.
    SUNXI_VIDEO_FORMAT_3840x2160P_30HZ,                             ///< UHD 3840x2160 30Hz.
    SUNXI_VIDEO_FORMAT_3840x2160P_50HZ,                             ///< UHD 3840x2160 50Hz.
    SUNXI_VIDEO_FORMAT_3840x2160P_60HZ,                             ///< UHD 3840x2160 60Hz.
    SUNXI_VIDEO_FORMAT_4096x2160P_24HZ,                             ///< UHD 4096x2160 24Hz.
    SUNXI_VIDEO_FORMAT_4096x2160P_25HZ,                             ///< UHD 4096x2160 25Hz.
    SUNXI_VIDEO_FORMAT_4096x2160P_30HZ,                             ///< UHD 4096x2160 30Hz.
    SUNXI_VIDEO_FORMAT_4096x2160P_50HZ,                             ///< UHD 4096x2160 50Hz.
    SUNXI_VIDEO_FORMAT_4096x2160P_60HZ,                             ///< UHD 4096x2160 60Hz.
    SUNXI_VIDEO_FORMAT_3D_1080P_24HZ,                               ///< HD 3D 1080P 24Hz, 2750 sample per line, SMPTE 274M-1998.
    SUNXI_VIDEO_FORMAT_MAX                                          ///< Total number of video formats.
} SUNXIVOPVideoFormat_e;

typedef struct
{
    int                     Connect;                                         ///< 1 if Rx device is connected; device may be ON or OFF.
    int                     RxPowerState;                                    ///< not support
    int                     IsHDCPAuthenticated;                             ///< not support
    SUNXIVOPVideoFormat_e   VideoFormat;                                     ///< Current video format.
    SUNXIAspectRatio_e      AspectRatio;                                     ///< value is SUNXI_ASPECT_RATIO_16x9
    SUNXIVOPColorSpace_e    ColorSpace;                                      ///< Current color space.
    char                    MonitorName[14];                                 ///< NULL-terminated string for the monitor name.
    int                     IsHdmiDevice;                                    ///< 1 if Rx supports HDMI, 0 if supports only DVI.
    SUNXIRect               ScreenSize;                                      ///< Monitor's screen size, mm unit.
    SUNXIVOPVideoFormat_e   PreferredVideoFormat;                            ///< Monitor's preferred video format.
    int                     VideoFormatSupported[SUNXI_VIDEO_FORMAT_MAX];    ///< Monitor's supported video format.
    int                     MaxAudioPcmChannnels;                            ///< Max number of PCM audio channels supported by the receiver.
    unsigned char PhysicalAddr[4];					     ///< Physical Address for HDMI node A,B,C,D
} SUNXIVOPHDMIMonitoStatus;

int SUNXIVOPEnable(SUNXIVOPDevice_e device);
int SUNXIVOPDisable(SUNXIVOPDevice_e device);

int SUNXIVOPSetColorSpace(SUNXIVOPDevice_e device, SUNXIVOPColorSpace_e colorspace);
int SUNXIVOPSetVideoFormat(SUNXIVOPDevice_e device, SUNXIVOPVideoFormat_e format);
int SUNXIVOPGetHDMIMonitorStatus(SUNXIVOPHDMIMonitoStatus *pstatus);

#ifdef __cplusplus
}
#endif

#endif /// SUNXI_VOP_H_

