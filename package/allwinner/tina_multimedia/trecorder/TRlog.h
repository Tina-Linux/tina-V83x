#ifndef _TRLOG_H
#define _TRLOG_H

#include "stdio.h"

#define TR_PROMPT           (1 << 0)
#define TR_WARN             (1 << 1)
#define TR_PRINT            (1 << 2)
#define TR_LOG              (1 << 3)
#define TR_LOG_VIDEO       (1 << 4)
#define TR_LOG_AUDIO        (1 << 5)
#define TR_LOG_DISPLAY      (1 << 6)
#define TR_LOG_ENCODER      (1 << 7)
#define TR_LOG_MUXER        (1 << 8)
#define TR_LOG_DECODER      (1 << 9)

extern unsigned int TR_log_mask;

/* print when error happens */
#define TRlog(flag, arg...) do{ \
    if(flag & TR_log_mask){ \
        switch(flag){ \
        case TR_LOG_VIDEO: \
            printf("[TR_LOG_VIDEO]"arg); \
            break; \
        case TR_LOG_AUDIO: \
            printf("[TR_LOG_AUDIO]"arg); \
            break; \
        case TR_LOG_DISPLAY: \
            printf("[TR_LOG_DISPLAY]"arg); \
            break; \
        case TR_LOG_ENCODER: \
            printf("[TR_LOG_ENCODER]"arg); \
            break; \
        case TR_LOG_MUXER: \
            printf("[TR_LOG_MUXER]"arg); \
            break; \
        case TR_LOG_DECODER: \
            printf("[TR_LOG_DECODER]"arg); \
            break; \
        default: \
            printf("[TR_LOG]"arg); \
            break; \
        } \
    } \
}while(0)

/* prompt information */
#define TRprompt(x,arg...) do{ \
    if(TR_log_mask & TR_PROMPT){ \
        printf("\033[;32m[TR_PROMPT]"x"\033[0m",##arg); \
        fflush(stdout); \
    } \
}while(0)

/* warning message */
#define TRwarn(x,arg...) do{ \
    if(TR_log_mask & TR_WARN){ \
        printf("\033[;33m[TR_WARN]"x"\033[0m",##arg); \
        fflush(stdout); \
    } \
}while(0)

/* print unconditional, for important info */
#define TRprint(x,arg...) do{ \
    if(TR_log_mask & TR_PRINT) \
        printf("[TR]"x,##arg); \
}while(0)

/* print when error happens */
#define TRerr(x,arg...) do{ \
    printf("\033[;31m[TR_ERR]"x"\033[0m",##arg); \
    fflush(stdout); \
}while(0)

#endif
