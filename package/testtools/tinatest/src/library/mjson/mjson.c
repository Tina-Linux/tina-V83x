#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "mjson.h"

static json_object *JSON_ROOT;

static int mjson_get_array_from_obj(json_object *obj, struct mjson_value *val)
{
    struct json_object *objelem;
    enum mjson_type type;
    int cnt = 0, max = json_object_array_length(obj);

    while (cnt < max) {
        objelem = json_object_array_get_idx(obj, cnt);
        type = (enum mjson_type)json_object_get_type(objelem);
        if (type != mjson_type_string) {
            ERROR("get array failed as index %d isn't string\n", cnt);
            while(--cnt >= 0)
                val->val.m_array[cnt] = NULL;
            return -1;
        }
        val->val.m_array[cnt++] = (char *)json_object_get_string(objelem);
    }
    val->val.m_array_cnt = cnt;
    return 0;
}

void mjson_get_value_from_obj(json_object *obj, struct mjson_value *val)
{
    if (obj == NULL || val == NULL)
        return;
    val->type = (enum mjson_type)json_object_get_type(obj);
    switch (val->type) {
        case mjson_type_null:
            val->val = (union mdata)0;
            break;
        case mjson_type_boolean:
            val->val.m_boolean = json_object_get_boolean(obj);
            break;
        case mjson_type_double:
            val->val.m_double = json_object_get_double(obj);
            break;
        case mjson_type_int:
            val->val.m_int = (int)json_object_get_int(obj);
            break;
        case mjson_type_array:
            if (mjson_get_array_from_obj(obj, val))
                val->type = mjson_type_error;
            break;
        case mjson_type_string:
            val->val.m_string = (char *)json_object_get_string(obj);
            break;
        default: //mjson_type_object && mjson_type_error
            val->val = (union mdata)0;
            break;
     }
}

int mjson_load(const char *fpath)
{
    FILE *fp;
    char *buf, *p;
    unsigned long len;
    struct stat status;

    //read file
    if (stat(fpath, &status) < 0) {
        ERROR("stat %s failed - %s\n", fpath, strerror(errno));
        return -1;
    }
    len = status.st_size;
    buf = p = (char *)malloc(len + 4);
    memset(buf, 0, len + 4);
    *p++ = '{';
    fp = fopen(fpath, "r");
    if (fp == NULL) {
        ERROR("open %s failed : %s\n", fpath, strerror(errno));
        return -1;
    }
    if (fread(p, 1, len, fp) != len) {
        ERROR("read %s failed : %s\n", fpath, strerror(errno));
        fclose(fp);
        return -1;
    }
    fclose(fp);
    if (strncmp(p, "\"/\"", 3)) {
        ERROR("%s should begin with \"/\"\n", fpath);
        return -1;
    }
    p += len;
    *p = '}';

    //parse file
    json_object *obj = NULL;
    struct json_tokener *tok = json_tokener_new();
    enum json_tokener_error jerr;
    obj = json_tokener_parse_ex(tok, buf, len + 2);
    if ((jerr = json_tokener_get_error(tok)) != json_tokener_success) {
        ERROR("parse %s failed(%s)\n", fpath, json_tokener_error_desc(jerr));
        return -1;
    }

    if (json_object_object_get_ex(obj, (const char *)"/", &JSON_ROOT) != TRUE) {
        ERROR("parse %s failed\n", fpath);
        return -1;
    }
    json_tokener_free(tok);
    free(buf);

    return 0;
}

static json_object *mjson_fetch_key(const char *keypath, json_object *obj)
{
    char *pre = (char *)keypath, *next = NULL;
    char key[1024] = {0};
    int len;
    json_object *val = NULL;

    if (pre[0] == '/')
        pre++;
    next = strchr(pre, '/');
    len = next == NULL ? strlen(pre) : (next - pre);
    strncpy(key, pre, len > 1023 ? 1023 : len);

    if (json_object_object_get_ex(obj, key, &val) == TRUE) {
        if (next != NULL)
            return mjson_fetch_key(next, val);
    }
    return val;
}

struct mjson_value mjson_fetch(const char *keypath)
{
    struct mjson_value val;
    val.type = mjson_type_error;
    val.keypath = keypath;

    if (JSON_ROOT == NULL)
#ifdef DEFAULT_JSON_PATH
    {
        if (mjson_load(DEFAULT_JSON_PATH) != 0)
            return val;
    }
#else
    {
        ERROR("havn't loaded json\n");
        return val;
    }
#endif
    if (keypath == NULL || JSON_ROOT == NULL)
        return val;
    if (strcmp(keypath, "/") == 0)
        val.obj = JSON_ROOT;
    else
        val.obj = mjson_fetch_key(keypath, JSON_ROOT);

    if (val.obj != NULL)
        mjson_get_value_from_obj(val.obj, &val);
    if (val.type != mjson_type_error) {
        char *p = strrchr(keypath, '/');
        val.keyname = ++p;
        val.keypath = keypath;
    }
    return val;
}

int mjson_fetch_int(const char *keypath)
{
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_int)
        return val.val.m_int;
    else
        return -1;
}

int mjson_fetch_boolean(const char *keypath)
{
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_boolean)
        return val.val.m_boolean;
    else
        return -1;
}

double mjson_fetch_double(const char *keypath)
{
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_double)
        return val.val.m_double;
    else
        return -1;
}

char *mjson_fetch_string(const char *keypath)
{
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_string)
        return val.val.m_string;
    else
        return NULL;
}

int mjson_fetch_array(const char *keypath, char *array[], int max)
{
    struct mjson_value val = mjson_fetch(keypath);
    if (val.type == mjson_type_array) {
        int cnt = max > val.val.m_array_cnt ? val.val.m_array_cnt : max;
        memcpy(array, val.val.m_array, sizeof(char *) * cnt);
        return cnt;
    } else {
        return 0;
    }
}

enum mjson_type mjson_get_type(const char *keypath)
{
    if (keypath == NULL || JSON_ROOT == NULL)
        return mjson_type_error;
    json_object *match_obj = mjson_fetch_key(keypath, JSON_ROOT);
    if (match_obj == NULL)
        return mjson_type_error;
    return (enum mjson_type)json_object_get_type(match_obj);
}

static void mjson_draw_tree_do(char *str, int depth)
{
    while(depth-- > 0)
        printf("|   ");
    printf("|-- ");
    printf("%s\n", str);
}

void mjson_draw_tree(const char *keypath)
{
    static int depth = -1;
    struct mjson_value val = mjson_fetch(keypath);
    char *buf = malloc(100);
    if (buf == NULL) {
        ERROR("malloc failed\n");
        return;
    }
    memset(buf, 0, 100);

    if (val.type == mjson_type_error) {
        ERROR("no match %s\n", keypath);
        free(buf);
        return;
    }
    if (depth == -1) {
        printf(". (%s)\n", keypath);
        depth++;
    }
    json_object_object_foreach(val.obj, key, value) {
        enum mjson_type type = (enum mjson_type)json_object_get_type(value);
        switch (type) {
            case mjson_type_int:
                snprintf(buf, 100, "%s = %d", key, json_object_get_int(value));
                mjson_draw_tree_do(buf, depth);
                break;
            case mjson_type_null:
                snprintf(buf, 100, "%s = NULL", key);
                mjson_draw_tree_do(buf, depth);
                break;
            case mjson_type_boolean:
                snprintf(buf, 100, "%s = %s", key,
                        json_object_get_boolean(value) ? "TRUE" : "FALSE");
                mjson_draw_tree_do(buf, depth);
                break;
            case mjson_type_string:
                snprintf(buf, 100, "%s = \"%s\"", key, json_object_get_string(value));
                mjson_draw_tree_do(buf, depth);
                break;
            case mjson_type_double:
                snprintf(buf, 100, "%s = %lf", key, json_object_get_double(value));
                mjson_draw_tree_do(buf, depth);
                break;
            case mjson_type_array:
                {
                    snprintf(buf, 100, "%s = [", key);
                    char *p = &buf[strlen(buf)];
                    const char *s;
                    int idx = 0;
                    for (; idx < json_object_array_length(value); idx++) {
                        json_object *obj = json_object_array_get_idx(value, idx);
                        switch ((enum mjson_type)json_object_get_type(obj)) {
                            case mjson_type_string:
                                s = json_object_get_string(obj);
                                if (idx == 0) {
                                    snprintf(p, 100 - strlen(buf), "\"%s\"", s);
                                    p += strlen(s) + 2;
                                }
                                else {
                                    snprintf(p, 100 - strlen(buf), ",\"%s\"", s);
                                    p += strlen(s) + 3;
                                }
                                break;
                            default:
                                if (idx == 0) {
                                    snprintf(p, 100 - strlen(buf), "Invalid");
                                    p += strlen("Invalid");
                                }
                                else {
                                    snprintf(p, 100 - strlen(buf), ",Invalid");
                                    p += strlen(",Invalid");
                                }
                                break;
                        }
                    }
                    snprintf(p, 100 - strlen(buf), "]");
                    mjson_draw_tree_do(buf,depth);
                }
                break;
            case mjson_type_object:
                snprintf(buf, 100, "%s", key);
                mjson_draw_tree_do(buf, depth);
                if (strcmp(keypath, "/") == 0)
                    snprintf(buf, 100, "%s%s", keypath, key);
                else
                    snprintf(buf, 100, "%s/%s", keypath, key);
                depth++;
                mjson_draw_tree(buf);
                break;
            default:
                break;
        }
    }
    depth--;
    free(buf);
}
