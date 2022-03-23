#include <stdio.h>
#include <stdlib.h>

#include "mjson.h"
#include "sysinfo.h"

int main(int argc, char *argv[])
{
    char *s_res;
    char *p;

    printf("config value:\n");
    // if path error will return NULL
    if ((s_res = mjson_fetch_string("/demo/demo-c/command")) != NULL)
        printf("\t/demo/demo-c/command = %s\n", s_res);
    else
        goto fail;

    // get sys info
    printf("system information:\n");
    p = get_kernel_version();
    if (p == NULL)
        printf("\tget kernel version failed\n");
    else
        printf("\tkernel version: %s", p);
    free(p); //must free in testcase after used

    p = get_target();
    if (p == NULL)
        printf("\tget target failed\n");
    else
        printf("\ttarget: %s", p);
    free(p); //must free in testcase after used

    goto success;

fail:
    //not 0 means failed
    return -1;
success:
    //0 means successfully
    return 0;
}
