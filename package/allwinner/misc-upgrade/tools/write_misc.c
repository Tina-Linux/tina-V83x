#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "misc_message.h"

void usage()
{
    LOGE("args1: command eg: efex,boot-recovery ...\nargs2: status\nargs3: version\n");
}
int main(int argc, char* argv[])
{
    if(argc == 1){
        usage();
        return 0;
    }

    struct bootloader_message boot, temp;
    char device[50];
    char *tmp;
    char *tmp1;
    char *tmp2;
    memset(&boot, 0, sizeof(struct bootloader_message));
    memset(&temp, 0, sizeof(temp));
    memset(device, 0, 50);
    tmp = getenv("partitions");
    if(tmp == NULL)
    {
        LOGE("get env failed\n");
        return -1;
    }
    tmp1 = strstr(tmp, "misc");
    if(tmp1 == NULL)
    {
        LOGE("get misc from partitions failed\n");
        return -1;
    }
    tmp1 += 5;
    tmp2 = strtok(tmp1, ":");
    if(tmp2 == NULL)
    {
        LOGE("strtok misc partition failed\n");
        return -1;
    }
    sprintf(device,"/dev/%s", tmp2);

    if (get_bootloader_message_block(&boot, device)){
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

    if (set_bootloader_message_block(&boot, device) ){
        LOGE("set misc failed\n");
        return -1;
    }

    //read for compare
    memset(&temp, 0, sizeof(temp));
    if (get_bootloader_message_block(&temp, device)){
        LOGE("get misc failed\n");
        return -1;
    }
    if( memcmp(&boot, &temp, sizeof(struct bootloader_message)) ){
        LOGE("set misc failed1\n");
        return -1;
    }
    return 0;

}
