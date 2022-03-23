#ifndef __MLOG_H
#define __MLOG_H

#include <stdio.h>

#define BASE 1
#define DATA 2

#if DEBUG_LEVEL
void init_log(void);
extern FILE *tty;
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

/* ERROR */
#if DEBUG_LEVEL && defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(tty, "\033[31m" "[ERROR]: (%s-%d): " fmt "\033[0m", \
            __func__, __LINE__, ##arg); \
}while(0)

#elif DEBUG_LEVEL && !defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(tty, "[ERROR]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)

#elif !DEBUG_LEVEL && defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "\033[31m" "[ERROR]: (%s-%d): " fmt "\033[0m", \
            __func__, __LINE__, ##arg); \
}while(0)

#elif !DEBUG_LEVEL && !defined(COLOUR_LOG)
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "[ERROR]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)
#endif

/* DEBUG */
#if DEBUG_LEVEL && defined(COLOUR_LOG)
#define DEBUG(level, fmt, arg...) \
do { \
    if (level <= DEBUG_LEVEL) { \
        if (level == BASE) { \
            fprintf(tty, "\033[32m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
        else if (level == DATA) { \
            fprintf(tty, "\033[33m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
    } \
    fflush(tty); \
}while(0)

#elif DEBUG_LEVEL && !defined(COLOUR_LOG)
#define DEBUG(level, fmt, arg...) \
do { \
    if (DEBUG_LEVEL == level || DEBUG_LEVEL > DATA) \
        fprintf(tty, "[DEBUG]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
    fflush(tty); \
}while(0)

#else
#define DEBUG(level, fmt, arg...)
#endif

#endif
