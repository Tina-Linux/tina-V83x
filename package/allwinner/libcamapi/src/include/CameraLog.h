#ifndef _CAMERA_LOG_H
#define _CAMERA_LOG_H

#include "stdio.h"

#define CAMERA_DEBUG			(1 << 0)
#define CAMERA_INFO				(1 << 1)
#define CAMERA_WARN				(1 << 2)

#define CAMERA_LOG_MASK 0x0111

#define CamDebug(x,arg...) do{ \
    if(CAMERA_LOG_MASK & CAMERA_DEBUG){ \
        printf("\033[;32m[CAMERA_DEBUG]"x"\033[0m",##arg); \
        fflush(stdout); \
    } \
}while(0)

#define CamInfo(x,arg...) do{ \
    if(CAMERA_LOG_MASK & CAMERA_INFO) \
        printf("\033[;34m[CAMERA_INFO]"x"\033[0m",##arg); \
}while(0)

#define CamWarn(x,arg...) do{ \
    if(CAMERA_LOG_MASK & CAMERA_WARN){ \
        printf("\033[;33m[CAMERA_WARN]"x"\033[0m",##arg); \
        fflush(stdout); \
    } \
}while(0)

#define CamErr(x,arg...) do{ \
    printf("\033[;31m[CAMERA_ERR]"x"\033[0m",##arg); \
    fflush(stdout); \
}while(0)

#endif
