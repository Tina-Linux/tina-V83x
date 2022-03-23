#include <unistd.h>
#include "misc_message.h"

void usage()
{
    LOGE("1111args1: command eg: efex,boot-recovery ...\nargs2: status\nargs3: version\n");
}
int main(int argc, char* argv[])
{
    if(argc == 1){
        usage();
        return 0;
    }

    struct bootloader_message boot, temp;
    memset(&boot, 0, sizeof(struct bootloader_message));

    if (get_bootloader_message_block(&boot, MISC_DEVICE)){
        LOGE("get misc failed\n");
        return -1;
    }
    for (;;) {
        int c = getopt(argc, argv, "hc:s:v:");
        if (c < 0) {
            break;
        }

        switch (c) {
            case 'c':
                strncpy(boot.command,optarg,sizeof(boot.command));
                break;
            case 's':
                strncpy(boot.status,optarg,sizeof(boot.status));
                break;
            case 'v':
                strncpy(boot.version,optarg,sizeof(boot.version));
                break;
            case 'h':
                usage();
                return 0;
            default:
                usage();
                return 0;
        }
    }

    if (set_bootloader_message_block(&boot, MISC_DEVICE) ){
        LOGE("set misc failed\n");
        return -1;
    }

    //read for compare
    memset(&temp, 0, sizeof(temp));
    if (get_bootloader_message_block(&temp, MISC_DEVICE)){
        LOGE("get misc failed\n");
        return -1;
    }
    if( memcmp(&boot, &temp, sizeof(struct bootloader_message)) ){
        LOGE("set misc failed1\n");
        return -1;
    }
    return 0;

}
