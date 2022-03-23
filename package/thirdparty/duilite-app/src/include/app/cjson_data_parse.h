#ifndef __CJSON_DATA_PARSE_H__
#define __CJSON_DATA_PARSE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "cJSON.h"

const char *json_string_1 = "{\"name\": \"Tom\",\"age\":25,\"marry\":true,\"child\":null}";

const char *json_string_2 = "{                                    \
        \"name\": \"中国\",                                 \
        \"province\": [{                                    \
            \"name\": \"黑龙江\",                           \
            \"cities\": {                                   \
                \"city\": [\"哈尔滨\", \"大庆\"]            \
            }                                               \
          }, {                                              \
            \"name\": \"广东\",                             \
            \"cities\": {                                   \
                \"city\": [\"广州\", \"深圳\", \"珠海\"]    \
            }                                               \
            }, {                                            \
            \"name\": \"台湾\",                             \
            \"cities\": {                                   \
                \"city\": [\"台北\", \"高雄\"]              \
                }                                           \
          }, {                                              \
            \"name\": \"新疆\",                             \
            \"cities\": {                                   \
                \"city\": [\"乌鲁木齐\"]                    \
                }                                           \
          }]                                                \
}";

#ifdef __cplusplus
}
#endif

#endif //__CJSON_DATA_PARSE_H__
