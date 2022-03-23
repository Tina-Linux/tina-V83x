#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <getopt.h>

#include <libubox/list.h>
#include "mjson.h"
#include "fileops.h"
#include "task.h"

int main(int argc, char *argv[])
{
#if DEBUG_LEVEL
    init_log();
#endif
    int opt, flags = 0, cnt = 0;
    char *reboot_keypath = NULL;
    while ((opt = getopt(argc, argv, ":r:p")) != -1) {
        switch (opt) {
            case 'p':
                flags = 1;
                DEBUG(BASE, "run in print mode\n");
                break;
            case 'r':
                flags = 2;
                reboot_keypath = optarg;
                DEBUG(BASE, "run in reboot mode with task %s\n", optarg);
                break;
            case ':':
                ERROR("option -%c need a argument\n", optopt);
                exit(1);
            default:
                ERROR("Unknown option : %c\n", (char)optopt);
                exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    // clean the last '/'
    for (cnt = 0; cnt < argc; cnt++) {
        char *p = argv[cnt];
        int len = strlen(p);
        if (len != 1 && p[len - 1] == '/')
            p[len - 1] = '\0';
    }

    // init mjson
    if (mjson_load(DEFAULT_JSON_PATH) != 0) {
        ERROR("load %s failed\n", DEFAULT_JSON_PATH);
        goto err;
    }

    /* -p */
    if (flags & (1 << 0)) {
        if (argc == 0) {
            DEBUG(BASE, "draw all\n");
            mjson_draw_tree("/");
        }
        else {
            for (cnt = 0; cnt < argc; cnt++) {
                DEBUG(BASE, "draw %s\n", argv[cnt]);
                mjson_draw_tree(argv[cnt]);
            }
        }
        exit(0);
    }

    // init task
    if ((cnt = task_init(argc, argv)) < 0) {
        ERROR("init task failed\n");
        goto err;
    } else if (cnt == 0) {
        ERROR("NO ACTIVE TESTCASE\n");
        exit(0);
    }

    /* -r */
    if (flags & (1 << 1)) { //reboot mode
        DEBUG(BASE, "%s has rebooted, recovering ...\n", reboot_keypath);
        if(task_recover_after_reboot(reboot_keypath) < 0) {
            ERROR("recover after reboot failed\n");
            goto err;
        }
    }

    return task_begin();
err:
    return 1;
}
