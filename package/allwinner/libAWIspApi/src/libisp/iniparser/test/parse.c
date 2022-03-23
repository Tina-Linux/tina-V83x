#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iniparser.h"

int main(int argc, char * argv[])
{
    dictionary * ini ;
    char       * ini_name ;

    if (argc<2) {
        ini_name = "/system/etc/hawkview/ov4688/isp_test_param.ini";
    } else {
        ini_name = argv[1] ;
    }

    ini = iniparser_load(ini_name);
    iniparser_dump(ini, stdout);
    iniparser_freedict(ini);

    return 0 ;
}
