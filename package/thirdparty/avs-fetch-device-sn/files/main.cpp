#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <string>

#include <json-c/json.h>

static const char *get_chipid_or_serial_cmd =
    "result=$(cat /sys/class/sunxi_info/sys_info | grep -E \"sunxi_chipid|sunxi_serial\");"
    " if [ -n \"$result\" ]; then echo $result | awk '{print $3}';"
    " else echo \"NULL\";"
    " fi";

static char *get_chipid_or_serial() {
    FILE *fp = NULL;
    char *buffer = (char *)malloc(100);
    fp = popen(get_chipid_or_serial_cmd, "r");
    if (!fp) {
        printf("Failed to get sunxi_chipid or sunxi_serial\n");
        free(buffer);
        return NULL;
    }
    fgets(buffer, 100, fp);
    pclose(fp);
    int len = strlen(buffer);
    if (buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
    if (0 == strcmp(buffer, "NULL")) {
        printf("sunxi_chipid or sunxi_serial not found\n");
        free(buffer);
        return NULL;
    }
    return buffer;
}

static void usage()
{
    printf("USAGE: avs-fetch-device-sn <AVS_SDK_config_file>\n");
}

int main(int argc, char *argv[])
{
    if(argc < 2) {
        usage();
        return 0;
    }

    const char *target = argv[1];

    struct json_object* config = json_object_from_file(target);
    if(!config) {
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

    char *p = get_chipid_or_serial();
    if (!p)
        return -1;
    if(strcmp(json_object_get_string(j_deviceSerialNumber), "") == 0) {
        json_object_object_add(j_deviceInfo, "deviceSerialNumber", json_object_new_string(p));
    }

    if(json_object_to_file_ext(target, config, JSON_C_TO_STRING_PRETTY) < 0) {
        printf("save json to %s failed!, errno: (%s)\n",
                target, strerror(errno));
        return -1;
    }

    printf("update deviceSerialNumber: %s to %s successful!\n", p, target);
    if (p) {
        free(p);
    }

    return 0;
}
