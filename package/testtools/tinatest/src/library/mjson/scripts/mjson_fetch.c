/* It's a API for shell */
#include <stdio.h>
#include <stdlib.h>

#include "mjson.h"

int main(int argc, char *argv[]) {
    for(int num = 1; num < argc; num++) {
        if (argv[num][0] != '/')
            continue;
        if (mjson_load(DEFAULT_JSON_PATH) != 0)
            return -1;

        struct mjson_value val;
        val = mjson_fetch(argv[num]);
        switch(val.type) {
            case mjson_type_null :
                printf("NULL");
                break;
            case mjson_type_boolean :
                printf("%s", val.val.m_boolean == TRUE ? "true" : "false");
                break;
            case mjson_type_double :
                printf("%f", val.val.m_double);
                break;
            case mjson_type_int :
                printf("%d", val.val.m_int);
                break;
            case mjson_type_array :
                for(int cnt = 1, max = atoi(val.val.m_array[0]); cnt <= max; cnt++) {
                    if (cnt == max)
                        printf("%s", val.val.m_array[cnt]);
                    else
                        printf("%s ", val.val.m_array[cnt]);
                }
                break;
            case mjson_type_string :
                printf("%s", val.val.m_string);
                break;
            default :
                break;
        }
        printf("\n");
    }

    return 0;
}
