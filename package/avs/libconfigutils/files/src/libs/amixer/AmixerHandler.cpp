
#include "utils/JsonUtils.h"
#include "utils/AmixerUtils.h"
#include "amixer/AmixerHandler.h"

#define GET_VAL_BY_KEY_CHECKED(json, key, value) \
do { \
    if(!JsonUtils::json_object_object_get_ex(json, key, &value)) { \
        printf("get %s failed\n", key); \
        return -1; }\
}while(0)

namespace AW {

std::shared_ptr<AmixerHandler> AmixerHandler::create(struct json_object *config)
{
    auto amixer = std::shared_ptr<AmixerHandler>(new AmixerHandler());
    if(amixer->init(config) < 0) return nullptr;

    return amixer;
}

AmixerHandler::~AmixerHandler()
{
    if(m_config != nullptr)
        JsonUtils::json_object_put(m_config);
}
int AmixerHandler::init(struct json_object *config)
{
    struct json_object *j_value;

    GET_VAL_BY_KEY_CHECKED(config, "device", j_value);
    GET_VAL_BY_KEY_CHECKED(config, "iface", j_value);
    GET_VAL_BY_KEY_CHECKED(config, "name", j_value);
    GET_VAL_BY_KEY_CHECKED(config, "active", j_value);
    GET_VAL_BY_KEY_CHECKED(config, "disactive", j_value);

    m_config = JsonUtils::json_object_get(config);

    return 0;
}

int AmixerHandler::active()
{
    return action("active");
}

int AmixerHandler::disactive()
{
    return action("disactive");
}

int AmixerHandler::action(const char* cmd)
{
    struct json_object *j_value;
    const char *card, *iface, *name;

    GET_VAL_BY_KEY_CHECKED(m_config, "device", j_value);
    card = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(m_config, "iface", j_value);
    iface = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(m_config, "name", j_value);
    name = JsonUtils::json_object_get_string(j_value);

    GET_VAL_BY_KEY_CHECKED(m_config, cmd, j_value);
    switch(JsonUtils::json_object_get_type(j_value)) {

        case json_type_int : {
            return AmixerUtils::cset(card, iface, name, JsonUtils::json_object_get_int(j_value));
        }
        case json_type_string : {
            return AmixerUtils::cset(card, iface, name, JsonUtils::json_object_get_string(j_value));
        }
        case json_type_boolean: {
            return AmixerUtils::cset(card, iface, name, JsonUtils::json_object_get_boolean(j_value)? true:false);
        }
        case json_type_null:
        case json_type_double:
        case json_type_object:
        case json_type_array:
        default:
            printf("AmixerHandler active type is not supported!\n");
            return -1;
    }
}

}
