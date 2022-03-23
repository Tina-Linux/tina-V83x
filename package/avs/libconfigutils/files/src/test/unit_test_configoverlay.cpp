#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "configs/ConfigOverlay.h"
#include "utils/JsonUtils.h"

int main(int argc, char **argv)
{

    /*
     * Check that replacing an existing object keeps the key valid,
     * and that it keeps the order the same.
     */
    AW::JsonUtils::init("libjson-c.so.2");

    struct json_object *base = AW::JsonUtils::json_object_new_object();
    AW::JsonUtils::json_object_object_add(base, "foo1", AW::JsonUtils::json_object_new_string("bar1"));
    AW::JsonUtils::json_object_object_add(base, "foo2", AW::JsonUtils::json_object_new_string("bar2"));
    AW::JsonUtils::json_object_object_add(base, "foo3", AW::JsonUtils::json_object_new_int(1));
    AW::JsonUtils::json_object_object_add(base, "foo4", AW::JsonUtils::json_object_new_int(2));
    printf("%s\n", AW::JsonUtils::json_object_to_json_string_ext(base, JSON_C_TO_STRING_PRETTY));

    struct json_object *overlay = AW::JsonUtils::json_object_new_object();
    AW::JsonUtils::json_object_object_add(overlay, "foo1", AW::JsonUtils::json_object_new_string("henrisk"));
    AW::JsonUtils::json_object_object_add(overlay, "foo3", AW::JsonUtils::json_object_new_int(100));
    AW::JsonUtils::json_object_object_add(overlay, "foo5", AW::JsonUtils::json_object_new_int(100));
    AW::ConfigOverlay::overlay(base, overlay);
    AW::JsonUtils::json_object_put(overlay);
    printf("%s\n", AW::JsonUtils::json_object_to_json_string_ext(base, JSON_C_TO_STRING_PRETTY));
    AW::JsonUtils::json_object_put(base);




    return 0;
}
