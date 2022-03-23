#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__

#include <json-c/json.h>


#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && __STDC_VERSION__ >= 199901L

# define JsonUtils_json_object_object_foreach(obj,key,val) \
    char *key = NULL; \
    struct json_object *val = NULL; \
    for(struct lh_entry *entry ## key = JsonUtils::json_object_get_object(obj)->head, *entry_next ## key = NULL; \
        ({ if(entry ## key) { \
            key = (char*)entry ## key->k; \
            val = (struct json_object*)entry ## key->v; \
            entry_next ## key = entry ## key->next; \
        } ; entry ## key; }); \
        entry ## key = entry_next ## key )

#else /* ANSI C or MSC */

# define JsonUtils_json_object_object_foreach(obj,key,val) \
    char *key;\
    struct json_object *val; \
    struct lh_entry *entry ## key; \
    struct lh_entry *entry_next ## key = NULL; \
    for(entry ## key = JsonUtils::json_object_get_object(obj)->head; \
        (entry ## key ? ( \
            key = (char*)entry ## key->k, \
            val = (struct json_object*)entry ## key->v, \
            entry_next ## key = entry ## key->next, \
            entry ## key) : 0); \
        entry ## key = entry_next ## key)

#endif /* defined(__GNUC__) && !defined(__STRICT_ANSI__) && __STDC_VERSION__ >= 199901L */

namespace AW
{

class JsonUtils
{
public:
    static int init(const char* path);
    static int release();
    static struct json_object* (*json_object_new_int)(int32_t i);
    static struct json_object* (*json_object_new_string)(const char *s);
    static struct json_object* (*json_object_new_object)(void);

    static struct json_object* (*json_object_get)(struct json_object *obj);
    static int (*json_object_put)(struct json_object *obj);

    static int (*json_object_array_length)(struct json_object *obj);
    static struct json_object* (*json_object_array_get_idx)(struct json_object *obj, int idx);

    static void (*json_object_object_del)(struct json_object* obj, const char *key);
    static void (*json_object_object_add)(struct json_object* obj, const char *key, struct json_object *val);


    static json_bool (*json_object_object_get_ex)(struct json_object* obj, const char *key, struct json_object **value);
    static struct lh_table* (*json_object_get_object)(struct json_object *obj);
    static enum json_type (*json_object_get_type)(struct json_object *obj);
    static int32_t (*json_object_get_int)(struct json_object *obj);
    static const char* (*json_object_get_string)(struct json_object *obj);
    static json_bool (*json_object_get_boolean)(struct json_object *obj);
    static double (*json_object_get_double)(struct json_object *obj);

    static int (*json_object_to_file_ext)(const char *filename, struct json_object *obj, int flags);
    static struct json_object* (*json_tokener_parse)(const char *str);
    static const char* (*json_object_to_json_string_ext)(struct json_object *obj, int flags);

private:
    static void *m_handler;
};

}
#endif
