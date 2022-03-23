#ifndef __SOCKET_DB_DEBUG
#define __SOCKET_DB_DEBUG

#define SOCKET_DB_DEBUG

#ifdef SOCKET_DB_DEBUG

#define DEBUG(fmt, arg...) \
do { \
    printf("[DEBUG]: " fmt, ##arg); \
    fflush(stdout); \
}while(0)

#define ERROR(fmt, arg...) \
do { \
    printf("[ERROR]: " fmt, ##arg); \
    fflush(stdout); \
}while(0)

#else
#define DEBUG(fmt,arg...)

#define ERROR(fmt,arg...)
#endif

#endif
