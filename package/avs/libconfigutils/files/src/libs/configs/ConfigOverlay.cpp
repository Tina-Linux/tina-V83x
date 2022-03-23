#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "utils/JsonUtils.h"
#include "utils/FileUtils.h"
#include "configs/ConfigOverlay.h"

namespace AW {

int ConfigOverlay::overlay(struct json_object *base, struct json_object *overlay)
{
    /*
    json_object_object_foreach(overlay, key, val) {
    struct json_object *j_value;
        if(!JsonUtils::json_object_object_get_ex(base, key, &j_value)) continue;
        JsonUtils::json_object_object_add(base, key, JsonUtils::json_object_get(val));
    }
    */
    JsonUtils_json_object_object_foreach(base, key, val) {
        struct json_object *j_value;
        if(JsonUtils::json_object_object_get_ex(overlay, key, &j_value)) continue;
        JsonUtils::json_object_object_add(overlay, key, JsonUtils::json_object_get(val));
    }
    return 0;
}

int ConfigOverlay::update(struct json_object *base, struct json_object *overlay, const char *key)
{
    int ret = 0;
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(overlay, key, &j_value)) {
        printf("ConfigOverlay get %s failed\n", key);
        return -1;
    }
    if(JsonUtils::json_object_get_type(j_value) == json_type_string) {
    ret = update_by_name(base, overlay, JsonUtils::json_object_get_string(j_value));
        if(ret < 0) return ret;
    }else if(JsonUtils::json_object_get_type(j_value) == json_type_array) {
    for(int i = 0; i < JsonUtils::json_object_array_length(j_value); i++){
        ret = update_by_name(base, overlay, JsonUtils::json_object_get_string(JsonUtils::json_object_array_get_idx(j_value, i)));
            if(ret < 0) return ret;
        }
    }
    return 0;
}

struct json_object * ConfigOverlay::getplatform(struct json_object *base)
{
    struct json_object *j_value;
    if(!JsonUtils::json_object_object_get_ex(base, "platform", &j_value)) {
        printf("ConfigOverlay get %s failed\n", "platform");
        return nullptr;
    }
    const char *platform = JsonUtils::json_object_get_string(j_value);

    if(strcmp(platform, "auto") == 0) {
    int len = 100;
    char *line = (char*)malloc(len);
    char *auto_platform = NULL;
    FileUtils fileutils;
    fileutils.create("/etc/openwrt_release", "rb");
    while(fileutils.gets(line, len) > 0) {
        const char *p1 = "DISTRIB_TARGET='";
        int p1_len = strlen(p1);
        char *start = NULL;
        start = strstr(line, p1);
        if(!start) continue;
        auto_platform = start + p1_len;
        start = strstr(start, "/");
        if(!start) continue;
        start[0] = '\0';
        break;
    }

    printf("auto_platform: %s\n", auto_platform);
    JsonUtils::json_object_object_get_ex(base, auto_platform, &j_value);
    fileutils.release();
    free(line);
    return j_value;
    }else{
    JsonUtils::json_object_object_get_ex(base, platform, &j_value);
    return j_value;
    }
}

int ConfigOverlay::update_by_name(struct json_object *base, struct json_object *overlay, const char *name)
{
    struct json_object *j_value;

    if(!JsonUtils::json_object_object_get_ex(overlay, name, &j_value)) {
        if(!JsonUtils::json_object_object_get_ex(base, name, &j_value)) {
            printf("ConfigOverlay overlay get %s failed\n", name);
            return -1;
        }
        JsonUtils::json_object_object_add(j_value, "base", JsonUtils::json_object_new_string(name));
        JsonUtils::json_object_object_add(overlay, name, JsonUtils::json_object_get(j_value));
        return 0;
    }
    struct json_object *j_target_base, *j_base_name;
    if(!JsonUtils::json_object_object_get_ex(j_value, "base", &j_base_name)) {
        printf("ConfigOverlay overlay get %s base failed\n", name);
        return -1;
    }
    if(!JsonUtils::json_object_object_get_ex(base, JsonUtils::json_object_get_string(j_base_name), &j_target_base)) {
        printf("ConfigOverlay base get %s failed\n", name);
        return -1;
    }

    ConfigOverlay::overlay(j_target_base, j_value);

    return 0;
}

}
