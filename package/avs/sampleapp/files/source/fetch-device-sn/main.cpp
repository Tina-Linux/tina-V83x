#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <string>

#include <json-c/json.h>

static char* get_chipid() {
    FILE * fp;
    char *buffer = (char*)malloc(100);
    fp = popen("cat /sys/class/sunxi_info/sys_info | grep sunxi_chipid | awk '{print $3}'", "r");
    fgets(buffer, 100, fp);
    pclose(fp);
    int len = strlen(buffer);
    if(buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    return buffer;
}

static void usage()
{

}

int main(int argc, char *argv[])
{
    if(argc < 2) {
        usage();
        return 0;
    }

    const char *target = argv[1];

    struct json_object* config = json_object_from_file(target);
    if(config == nullptr) {
        printf("fetch json from %s failed, please check the json file!, errno: (%s)\n",
                target, strerror(errno));
        return -1;
    }

    struct json_object* j_deviceInfo, *j_deviceSerialNumber;

    if(!json_object_object_get_ex(config, "deviceInfo", &j_deviceInfo)) {
        printf("fetch deviceInfo for json failed, please check the json file!\n");
        return -1;
    }

    if(!json_object_object_get_ex(j_deviceInfo, "deviceSerialNumber", &j_deviceSerialNumber)) {
        printf("fetch deviceSerialNumber for json failed, please check the json file!\n");
        return -1;
    }

    char *p = get_chipid();
    if(strcmp(json_object_get_string(j_deviceSerialNumber), "") == 0) {
        json_object_object_add(j_deviceInfo, "deviceSerialNumber", json_object_new_string(p));
    }

    if(json_object_to_file_ext(target, config, JSON_C_TO_STRING_PRETTY) < 0) {
        printf("save json to %s failed!, errno: (%s)\n",
                target, strerror(errno));
        return -1;
    }

    printf("update deviceSerialNumber: %s to %s successful!\n", p, target);
    free(p);

    return 0;
}