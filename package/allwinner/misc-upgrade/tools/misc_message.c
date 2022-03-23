#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include "misc_message.h"
#ifdef __cplusplus
extern "C" {
#endif
static int get_mtd_partition_index_byname(const char* name)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int index = 0;
    FILE* fp;
    fp = fopen("/proc/mtd","r");
    if(fp == NULL){
        LOGE("open /proc/mtd failed(%s)\n",strerror(errno));
        return -1;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if( strstr(line,name) == NULL )
            continue;
        index = line[3] - '0';
        break;
    }
    free(line);
    return index;
}
static int is_mmc_or_mtd()
{
    int part_index = 0;
    int is_mtd = access("/dev/mtd0", F_OK); //mode填F_OK试试
    if(is_mtd == -1)
        return 0; //mmc
#if 0
    const char* cmd = "device=${partitions#*misc@mtdblock}; device=${device%%:*}; echo $device";
    int bytes;
    char buf[10];
    FILE   *stream;
    stream = popen( cmd, "r" );
    if(!stream) return -1;
    bytes = fread( buf, sizeof(char), sizeof(buf), stream);
    pclose(stream);
    part_index = atoi(buf);
#endif
    part_index = get_mtd_partition_index_byname("misc");
    return part_index;//mtd
}

int get_bootloader_message_block(struct bootloader_message *out,
                                 const char* misc)
{
    char device[50];
    FILE* f;
//    int id = is_mmc_or_mtd();
//    if(id == 0){
        strcpy(device,misc);
//    }
//    else{
//        sprintf(device,"/dev/mtd%d",id);
//    }

    f = fopen(device, "rb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    struct bootloader_message temp;
    int count = fread(&temp, sizeof(temp), 1, f);
    if (count != 1) {
        LOGE("Failed reading %s\n(%s)\n", device, strerror(errno));
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    memcpy(out, &temp, sizeof(temp));
    return 0;
}

int set_bootloader_message_block(const struct bootloader_message *in,
                                 const char* misc)
{
    char device[50];
    FILE* f;
    int id = is_mmc_or_mtd();
//    if(id == 0){
        strcpy(device,misc);
//    }
//    else{
//        sprintf(device,"/dev/mtd%d",id);
//        system("mtd erase misc");
//    }

    f = fopen(device,"wb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    int count = fwrite(in, sizeof(*in), 1, f);
    if (count != 1) {
        LOGE("Failed writing %s\n(%s)\n", device, strerror(errno));
        fclose(f);
        return -1;
    }
    fflush(f);
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    return 0;
}
#ifdef __cplusplus
}
#endif
