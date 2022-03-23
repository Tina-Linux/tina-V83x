#ifndef __BOOTANIMATION_H
#define __BOOTANIMATION_H

#include <libubox/ulog.h>

#ifdef DEBUG
#define LOG  	ULOG_INFO
#define ERROR 	ULOG_ERR
#else
#define LOG(fmt, arg...)
#define ERROR(fmt, arg...)
#endif

#define DEFAULT_FRAMEBUFFER "/dev/fb0"
//#define FBV_SUPPORT_JPEG
#define FBV_SUPPORT_PNG
//#define FBV_SUPPORT_BMP


#ifndef BOOT_ANIMATION_PRIORITY
#define BOOT_ANIMATION_PRIORITY 0
#endif

#ifndef BOOT_MUSIC_PRIORITY
#define BOOT_MUSIC_PRIORITY 0
#endif

int bootanimation(void);

void bootanimation_exit(void);



#endif
