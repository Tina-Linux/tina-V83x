#include "utils/JsonUtils.h"

#include <typeinfo>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>

#define GET_FUNC_BY_NAME(var, name) \
do { \
    char *error; \
    name = (typeof JsonUtils::##name)dlsym(m_handler, name); \
    if ((error = dlerror()) != NULL)  { \
        printf("%s\n", error); \
        return -1; \
    } \
}while(0)

#define GET_FUNC_BY_TYPE_NAME(var, type, name) \
do { \
    char *error; \
    var = (type)dlsym(m_handler, name); \
    if ((error = dlerror()) != NULL)  { \
        printf("%s\n", error); \
        return -1; \
    } \
}while(0)

namespace AW {

void * JsonUtils::m_handler = nullptr;

struct json_object*(*JsonUtils::json_object_new_int)(int32_t) = nullptr;
struct json_object* (*JsonUtils::json_object_new_string)(const char *s) = nullptr;
struct json_object* (*JsonUtils::json_object_new_object)(void)  = nullptr;
int (*JsonUtils::json_object_put)(struct json_object *)  = nullptr;
const char* (*JsonUtils::json_object_to_json_string_ext)(struct json_object *, int )  = nullptr;
void (*JsonUtils::json_object_object_add)(struct json_object* , const char *, struct json_object *) = nullptr;
int (*JsonUtils::json_object_array_length)(struct json_object *) = nullptr;
int (*JsonUtils::json_object_to_file_ext)(const char *, struct json_object *, int ) = nullptr;
enum json_type (*JsonUtils::json_object_get_type)(struct json_object *) = nullptr;
struct json_object* (*JsonUtils::json_object_array_get_idx)(struct json_object *, int ) = nullptr;
const char* (*JsonUtils::json_object_get_string)(struct json_object *)  = nullptr;
void (*JsonUtils::json_object_object_del)(struct json_object* , const char *) = nullptr;
struct lh_table* (*JsonUtils::json_object_get_object)(struct json_object *) = nullptr;
struct json_object* (*JsonUtils::json_tokener_parse)(const char *) = nullptr;
int32_t (*JsonUtils::json_object_get_int)(struct json_object *)  = nullptr;
struct json_object* (*JsonUtils::json_object_get)(struct json_object *)  = nullptr;
json_bool (*JsonUtils::json_object_object_get_ex)(struct json_object* , const char *, struct json_object **) = nullptr;
json_bool (*JsonUtils::json_object_get_boolean)(struct json_object *obj) = nullptr;
double (*JsonUtils::json_object_get_double)(struct json_object *obj) = nullptr;

int JsonUtils::init(const char* path)
{
    m_handler = dlopen(path, RTLD_LAZY);
    if (!m_handler) {
        printf("dlopen %s error(%s)\n", path, strerror(errno));
        return -1;
    }

    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_new_int, struct json_object*(*)(int32_t), "json_object_new_int");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_new_string, struct json_object* (*)(const char *s), "json_object_new_string");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_new_object, struct json_object* (*)(void), "json_object_new_object") ;
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_put, int (*)(struct json_object *),"json_object_put");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_to_json_string_ext, const char* (*)(struct json_object *, int ),"json_object_to_json_string_ext");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_object_add, void (*)(struct json_object* , const char *, struct json_object *),"json_object_object_add");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_array_length, int (*)(struct json_object *),"json_object_array_length");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_to_file_ext, int (*)(const char *, struct json_object *, int ),"json_object_to_file_ext");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_type, enum json_type (*)(struct json_object *),"json_object_get_type");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_array_get_idx, struct json_object* (*)(struct json_object *, int) ,"json_object_array_get_idx");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_string, const char* (*)(struct json_object *),"json_object_get_string");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_object_del, void (*)(struct json_object* , const char *),"json_object_object_del");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_object, struct lh_table* (*)(struct json_object *),"json_object_get_object");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_tokener_parse, struct json_object* (*)(const char *),"json_tokener_parse");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_int, int32_t (*)(struct json_object *),"json_object_get_int");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get, struct json_object* (*)(struct json_object *),"json_object_get");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_object_get_ex, json_bool (*)(struct json_object* , const char *, struct json_object **),"json_object_object_get_ex");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_boolean, json_bool (*)(struct json_object *),"json_object_get_boolean");
    GET_FUNC_BY_TYPE_NAME(JsonUtils::json_object_get_double, double (*)(struct json_object *),"json_object_get_double");
}

int JsonUtils::release()
{

}

}
