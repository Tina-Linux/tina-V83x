#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bsa_api.h"

extern BD_NAME local_device_conf_name;
extern BD_ADDR local_device_set_addr;

#define CONF_COMMENT '#'
#define CONF_DELIMITERS " =\n\r\t"
#define CONF_VALUES_DELIMITERS "\"=\n\r\t"
#define CONF_MAX_LINE_LEN 255
#define APP_REM_DEVICES_CONNECTED_FILE_PATH "./bt_connected"

typedef int (conf_action_t)(char *p_conf_name, char *p_conf_value);

typedef struct {
    const char *conf_entry;
    conf_action_t *p_action;
} conf_entry_t;

int device_name_cfg(char *p_conf_name, char *p_conf_value);
int device_addr_set(char *p_conf_name, char *p_conf_value);

/*
 * Current supported entries and corresponding action functions
 */
static const conf_entry_t conf_table[] = {
    {"Name", device_name_cfg},
    {"BtAddr", device_addr_set},
    {(const char *) NULL, NULL}
};

/*****************************************************************************
**   FUNCTIONS
*****************************************************************************/
int str2bd(char *str, BD_ADDR addr)
{
    int i = 0;
    for (i = 0; i < 6; i++) {
       addr[i] = (UINT8) strtoul(str, (char **)&str, 16);
       str++;
    }
    return 0;
}

int device_name_cfg(char *p_conf_name, char *p_conf_value)
{
    strcpy((char *)local_device_conf_name, p_conf_value);
    return 0;
}

int device_addr_set(char *p_conf_name, char *p_conf_value)
{
    int i = 0;
    char *str = p_conf_value;

    if (!p_conf_value){
        for(i=0; i<6; i++){
            local_device_set_addr[0] = 0;
        }
        return 0;
    }

    for (i = 0; i < 6; i++) {
       local_device_set_addr[i] = (UINT8) strtoul(str, (char **)&str, 16);
       str++;
    }

    return 0;
}

void bta_load_addr(const char *p_path)
{
    FILE    *p_file;
    char    line[CONF_MAX_LINE_LEN+1]; /* add 1 for \0 char */

    printf("Attempt to read mac addr from %s\n", p_path);

    if ((p_file = fopen(p_path, "r")) != NULL){
        printf("open %s success!\n", p_path);
        if (fgets(line, CONF_MAX_LINE_LEN+1, p_file) != NULL){
		printf("line:%s\n", line);
            device_addr_set(NULL, line);
        } else {
            printf("read file error!\n");
            device_addr_set(NULL, NULL);
        }
        fclose(p_file);
    }else{
         printf("open file error!\n");
         device_addr_set(NULL, NULL);
    }
}

void store_connected_dev(BD_ADDR bt_mac_addr)
{
    FILE    *p_file;

    printf("Attempt to store connected dev mac addr to %s\n", APP_REM_DEVICES_CONNECTED_FILE_PATH);

    if ((p_file = fopen(APP_REM_DEVICES_CONNECTED_FILE_PATH, "w+")) != NULL){
        fwrite(bt_mac_addr, 1, BD_ADDR_LEN, p_file);
        fclose(p_file);
    } else {
        printf("Error: open %s failed!\n", APP_REM_DEVICES_CONNECTED_FILE_PATH);
        printf("errno %d:%s\n", errno, strerror(errno));
    }
}

void read_connected_dev(BD_ADDR bt_mac_addr)
{
    FILE    *p_file;

    if ((p_file = fopen(APP_REM_DEVICES_CONNECTED_FILE_PATH, "r+")) != NULL){
        fread(bt_mac_addr, 1, BD_ADDR_LEN, p_file);
        fclose(p_file);
    }
}
