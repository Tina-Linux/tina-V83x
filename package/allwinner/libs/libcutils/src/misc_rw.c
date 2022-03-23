/*
 * write boot command in misc partition
 */

#define LOG_TAG "misc_rw"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define LOGE(...) fprintf(stderr, "E:" __VA_ARGS__)

static const char *MISC_DEVICE = "/dev/block/by-name/misc";

/* Bootloader Message
 */
struct bootloader_message {
    char command[32];
    char status[32];
    char recovery[1024];
};


// ------------------------------------
// for misc partitions on block devices
// ------------------------------------

static int get_bootloader_message_block(struct bootloader_message *out,
                                        const char* device) {
    FILE* f = fopen(device, "rb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    struct bootloader_message temp;
    int count = fread(&temp, sizeof(temp), 1, f);
    if (count != 1) {
        LOGE("Failed reading %s\n(%s)\n", device, strerror(errno));
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    memcpy(out, &temp, sizeof(temp));
    return 0;
}

static int set_bootloader_message_block(const struct bootloader_message *in,
                                        const char* device) {
    FILE* f = fopen(device, "wb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    int count = fwrite(in, sizeof(*in), 1, f);
    if (count != 1) {
        LOGE("Failed writing %s\n(%s)\n", device, strerror(errno));
		fclose(f);
        return -1;
    }
    fflush(f);
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", device, strerror(errno));
        return -1;
    }
    return 0;
}

/* force the next boot to recovery/efex */
int write_misc(char *reason) {
    struct bootloader_message boot, temp;

    memset(&boot, 0, sizeof(boot));
    if (!strcmp("recovery", reason)) {
        reason = "boot-recovery";
    }

    strcpy(boot.command, reason);
    if (set_bootloader_message_block(&boot, MISC_DEVICE) )
        return -1;

    //read for compare
    memset(&temp, 0, sizeof(temp));
    if (get_bootloader_message_block(&temp, MISC_DEVICE))
        return -1;

    if( memcmp(&boot, &temp, sizeof(boot)) )
        return -1;

    return 0;
}

/*
 * The recovery tool communicates with the main system through /cache files.
 *   /cache/recovery/command - INPUT - command line for tool, one arg per line
 *   /cache/recovery/log - OUTPUT - combined log file from recovery run(s)
 *   /cache/recovery/intent - OUTPUT - intent that was passed in
 *
 * The arguments which may be supplied in the recovery.command file:
 *   --send_intent=anystring - write the text out to recovery.intent
 *   --update_package=path - verify install an OTA package file
 *   --wipe_data - erase user data (and cache), then reboot
 *   --wipe_cache - wipe cache (but not user data), then reboot
 *   --set_encrypted_filesystem=on|off - enables / diasables encrypted fs
 */
static const char *COMMAND_FILE = "/cache/recovery/command";
