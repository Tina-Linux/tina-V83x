#ifndef __MISC_MESSAGE_H__
#define __MISC_MESSAGE_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MISC_LIMITED_SIZE (4*1024)
//#define MISC_DEVICE "/dev/by-name/misc"

#define LOGE(...) printf( __VA_ARGS__)

#define CHECK_MISC_MESSAGE \
    ((sizeof(bootloader_message) > MISC_LIMIT_SIZE) ? -1 : 0)

#ifdef __cplusplus
extern "C" {
#endif
struct bootloader_message {
    char command[32];
    char status[32];
    char version[32];
    //char reserved[1024];
};

int get_bootloader_message_block(struct bootloader_message *out,
                                 const char* misc);

int set_bootloader_message_block(const struct bootloader_message *in,
                                 const char* misc);
#ifdef __cplusplus
}
#endif
#endif /*__MISC_MESSAGE_H__*/
