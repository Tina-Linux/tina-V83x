#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "misc_message.h"
void usage(){
    LOGE("read_misc : read all the message\n");
    LOGE("read_misc command : read the command message\n");
    LOGE("read_misc status : read the status  message\n");
}
int main(int args, char* argv[]){

    struct bootloader_message temp;
    FILE *fp;
    char device[50];
    char *tmp;
    char *tmp1;
    char *tmp2;
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
    if (get_bootloader_message_block(&temp, device)){
        LOGE("get misc failed\n");
        return -1;
    }
    if(args == 1){
        LOGE("command: %s\n",temp.command);
        LOGE("status:  %s\n",temp.status);
        LOGE("version: %s\n",temp.version);
        return 0;
    }
    if(!strcmp(argv[1],"command")){
        LOGE("%s\n",temp.command);
        return 0;
    }
    if(!strcmp(argv[1],"status")){
        LOGE("%s\n",temp.status);
        return 0;
    }
    if(!strcmp(argv[1],"version")){
        LOGE("%s\n",temp.version);
        return 0;
    }
    return 0;

}
