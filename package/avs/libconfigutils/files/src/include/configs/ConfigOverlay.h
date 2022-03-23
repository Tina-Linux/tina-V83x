#ifndef __CONFIG_OVERLAY_H__
#define __CONFIG_OVERLAY_H__

#include <json-c/json.h>

namespace AW {

class ConfigOverlay
{
public:
    static int overlay(struct json_object *base, struct json_object *overlay);
    static int update(struct json_object *base, struct json_object *overlay, const char *key);
    static struct json_object *getplatform(struct json_object *base);
private:
    static int update_by_name(struct json_object *base, struct json_object *overlay, const char *name);
};
}
#endif
