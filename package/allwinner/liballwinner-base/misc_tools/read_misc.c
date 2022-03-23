#include "misc_message.h"

void usage(){
    LOGE("read_misc : read all the message\n");
    LOGE("read_misc command : read the command message\n");
    LOGE("read_misc status : read the status  message\n");
}
int main(int args, char* argv[]){

    struct bootloader_message temp;
    memset(&temp, 0, sizeof(temp));
    if (get_bootloader_message_block(&temp, MISC_DEVICE)){
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
