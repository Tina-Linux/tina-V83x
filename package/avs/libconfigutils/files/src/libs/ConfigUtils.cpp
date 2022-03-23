#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mount.h>
#include <json-c/json.h>
#include <sys/time.h>
#include "ConfigUtils.h"

#define CONFIGURE_PATH "/etc/avs/configure"

#define PRIVATE_PATH "/tmp/private"
#define PRIVATE_SNUM_PATH PRIVATE_PATH"/ULI/factory/snum.txt"

#define plogd(fmt, arg...) printf(fmt"\n", ##arg)
#define ploge(fmt, arg...) printf(fmt"\n", ##arg)

#define CHECK(_x)                   \
do {                                \
    int _result = (_x);             \
    if (_result != 0) {       \
        ploge("%s(%d): @@@ CHECK failed, result=%d\n", __FILE__, __LINE__, _result); \
        return _result;             \
    }                               \
} while(0)

#define CHECK_POINTER(_p)                     \
do {                                          \
    if ((_p) == NULL) {                       \
        ploge("%s(%d): @@@ NULL pointer parameter\n", __FILE__, __LINE__); \
        return -1;            \
    }                                         \
} while(0)

#define DELETE_POINTER(_p)          \
do{                                 \
    if((_p) != NULL){               \
        ploge("delete: %p",_p);     \
        delete _p;                  \
        _p = NULL;                  \
    }                               \
}while(0)

#define FREE_DATA(x) \
do \
{ \
    if((x) != NULL) { \
        free(x); \
        (x) = NULL; \
    } \
} while(0)

static int get_mac_addr(const char* name,char* hwaddr)
{
    struct ifreq ifreq;
    int sock;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return 2;
    }
    strcpy(ifreq.ifr_name,name);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        return 3;
    }
    sprintf(hwaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

    return   0;
}

static int read_from_file(const char *path, char **buf)
{
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        ploge("fopen %s error(%s)", path, strerror(errno));
        return -1;
    }else{
        fseek(fp, 0L, SEEK_END);
        int flen = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        *buf = (char*)malloc(flen + 1);
        if (buf == NULL) {
            ploge("%s(%d): @@@ NULL pointer parameter\n", __FILE__, __LINE__);
            fclose(fp);
            return -1;
        }
        bzero(*buf, flen + 1);
        fread(*buf, flen + 1, 1, fp);
        fflush(fp);
        fclose(fp);
        return flen;
    }
}

namespace Application{

const char* ConfigUtils::getPropertyValue(const char *ablum, const char *key)
{
    struct json_object *j_ablum, *j_key, *j_value;

    if(ablum == nullptr) return nullptr;

    if(!json_object_object_get_ex(configure, ablum, &j_ablum))
        return nullptr;

    if(key == nullptr) return json_object_get_string(j_ablum);

    if(!json_object_object_get_ex(j_ablum, key, &j_key))
        return nullptr;

    if(!json_object_object_get_ex(j_key, "value", &j_value))
        return json_object_get_string(j_key);

    return json_object_get_string(j_value);
}

int ConfigUtils::setPropertyValue(const char *ablum, const char *key, const char *value)
{
    if(value == nullptr) return -1;

    struct json_object *j_ablum, *j_key, *j_value;
    if(!json_object_object_get_ex(configure, ablum, &j_ablum))
        return -1;

    if(key == nullptr) {
        json_object_object_del(configure, ablum);
        json_object_object_add(configure, ablum, json_object_new_string(value));
        return save_configure();
    }

    if(!json_object_object_get_ex(j_ablum, key, &j_key))
        return -1;

    json_object_object_del(j_key, "value");
    json_object_object_del(j_key, "timestamp");

    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    char timestamp[100];
    snprintf(timestamp, 100, "%02d-%02d-%02d-%02d-%02d-%06ld",  1+p->tm_mon, \
                                                p->tm_mday, \
                                                p->tm_hour, \
                                                p->tm_min, \
                                                p->tm_sec, \
                                                tv.tv_usec);

    json_object_object_add(j_key, "value", json_object_new_string(value));
    json_object_object_add(j_key, "timestamp", json_object_new_string(timestamp));
    //plogd("%s",json_object_to_json_string(configure));
    return save_configure();
}

int ConfigUtils::save_configure()
{
    return json_object_to_file_ext(configure_path.data(), configure, JSON_C_TO_STRING_PRETTY);
}

int ConfigUtils::reupdate_wifi_mac()
{
    const char *value = getPropertyValue("wifi", "mac");
    if(value == nullptr) return -1;

    if(!strcmp(value, "#by-hardware")){
        int len = 100;
        char *buf = (char*)malloc(100);
        CHECK_POINTER(buf);
        bzero(buf, len);
        get_mac_addr(getPropertyValue("wifi", "interface"), buf);
        setPropertyValue("wifi", "mac", buf);
        free(buf);
    }
    return 0;
}
int ConfigUtils::reupdate_device_id()
{
    const char *value = getPropertyValue("device", "id");
    if(value == nullptr) return -1;

    if(!strcmp(value, "#by-private")){
        //get id  from private partition
        plogd("get ID from private!!");
        char *buf = NULL;
        int len = read_from_file(PRIVATE_SNUM_PATH, &buf);
        if(len < 0) {
            ploge("fopen and fwrite %s error(%s)", PRIVATE_SNUM_PATH, strerror(errno));
            if (buf) {
                free(buf);
            }
            throw;
        }
        setPropertyValue("device", "id", buf);
        if (buf) {
            free(buf);
        }

    }else if(!strcmp(getPropertyValue("device", "id"), "#by-hardware")){
        plogd("get ID from wifi mac!!");
        setPropertyValue("device", "id", getPropertyValue("wifi", "mac"));
    }

    return 0;
}

int ConfigUtils::reupdate_bluetooth_mac()
{
    const char *value = getPropertyValue("bluetooth", "mac");
    if(value == nullptr) return -1;

    if(!strcmp(value, "#by-wifi")){
        const char *bt_mac = getPropertyValue("wifi", "mac");
        if(bt_mac == nullptr) return -1;

        setPropertyValue("bluetooth", "mac", bt_mac);

        FILE *fp = fopen(getPropertyValue("bluetooth", "mac-file"), "wb");
        if(fp == nullptr) {
            ploge("open %s error: %s", getPropertyValue("bluetooth", "mac-file"), strerror(errno));
            return -1;
        }

        fwrite(bt_mac, strlen(bt_mac), 1, fp);
        fclose(fp);
    }else if(!strcmp(getPropertyValue("bluetooth", "mac"), "#by-private")){
        plogd("TODO: get BT mac from private!!");
    }

    return 0;
}

int ConfigUtils::init(const char *path)
{
    //mount private
    //.........

    mkdir(PRIVATE_PATH, 0777);
    int ret = mount("/dev/by-name/private", PRIVATE_PATH, "vfat", MS_RDONLY, NULL);
    if(ret < 0){
        ploge("mount private failed: (%s),ignored",strerror(errno));
    }
    configure_path = path;

    //get configure
    char *buf = NULL;
    int len = read_from_file(configure_path.data(), &buf);
    if(len < 0) {
        ploge("fopen and fwrite %s error(%s)",CONFIGURE_PATH, strerror(errno));
        if (buf) {
            free(buf);
        }
        throw;
    }
    plogd("%s", buf);
    release();

    configure = json_tokener_parse(buf);
    if (buf) {
        free(buf);
    }

    if(configure == NULL) {
        ploge("get configure json fail");
        return -1;
    }

    reupdate_wifi_mac();
    reupdate_device_id();
    reupdate_bluetooth_mac();

    //umount private
    umount(PRIVATE_PATH);

    return 0;
}

void ConfigUtils::release()
{
    if(configure != nullptr)
        json_object_put(configure);
}

int ConfigUtils::setConfigPropertyListener(const char *ablum, const char *key, ConfigPropertyListener *l)
{
    struct json_object *j_ablum, *j_key;
    if(!json_object_object_get_ex(configure, ablum, &j_ablum))
        return -1;

    if(!json_object_object_get_ex(j_ablum, key, &j_key))
        return -1;

    //TODO
    return -1;
}

}
