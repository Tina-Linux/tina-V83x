/*****************************************************************************
**
**  Name:           app_ble_client_otafwdl.c
**
**  Description:    Bluetooth LE OTA fw download application
**
**  Copyright (c) 2010-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#include "app_ble.h"
#include "app_utils.h"
#include "app_ble_client_otafwdl.h"
/*Local function*/
static void    app_ble_client_uuid128_convert(char uuid[],char *buf);
static void    app_ble_clinet_fw_removechar(char *str,char remove_char);
static void    app_ble_clinet_fw_data_append(char *str,UINT32 from,UINT32 end);
static UINT32  app_ble_client_fw_hex2int(char t);
static void    app_ble_clinet_fw_hex2binary(app_ble_client_fwdl_fw_upgrade_cb *fw);
static UINT32  app_ble_clinet_fw_cal_chunk_size(char a, char b);
static UINT32  app_ble_clinet_fw_hexfile2bin(char *str);
static int     app_ble_client_fw_register_notification(void);
static void    app_ble_clinet_fw_upgrade_prepare(UINT32 client_num,UINT32 len, unsigned char *data);
static void    app_ble_clinet_fw_upgrade_start();
static void    app_ble_clinet_fw_write_data(UINT32 client_num,UINT32 len, unsigned char *data);
static void    app_ble_clinet_fw_crc_calculate(void);
static UINT32  app_ble_clinet_fw_crc_reflect(UINT32 data,UINT32 nbits);
static UINT32  app_ble_clinet_fw_crc_slow(UINT32 crc32 , unsigned char *data, UINT32 nbytes);
static UINT32  app_ble_clinet_fw_crc_complete(UINT32 crc32);;
static UINT32  app_ble_clinet_fw_crc_reflect_data(UINT32 data);
static UINT32  app_ble_clinet_fw_crc_reflect_remainder(UINT32 data);
static UINT32  app_ble_client_fw_get_file_size(char *str);
static void    app_ble_client_fw_upgrade_init(void);
static void    app_ble_client_fw_upgrade_done(void);

/*Global variable*/
app_ble_client_fwdl_fw_upgrade_cb *app_ble_client_fwdl_fw_upgrade_info = NULL;
const char APP_BLE_CLIENT_FWDL_OTA_SERVICE_ID[]       =  "9E5D1E475C1343A0863582AD38A1386F";
const char APP_BLE_CLIENT_FWDL_OTA_CHAR_NOTIFY_ID[]   =  "E3DD50BFF7A74E99838E570A086C666B";
const char APP_BLE_CLIENT_FWDL_OTA_CHAR_HEX_DATA_ID[] =  "92E86C7AD9614091B74F2409E72EFE36";

/*******************************************************************************
**
 ** Function        app_ble_client_fw_get_file_size
 **
 ** Description     return the file size
 **
 ** Parameters      the file path
 **
 ** Returns          the file size
 **
 *********************************************************************************/
static UINT32 app_ble_client_fw_get_file_size(char *str)
{
    FILE *fp;
    UINT32 size;

    fp = fopen(str,"r");
    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    fclose(fp);
    return size;

}
/*******************************************************************************
**
** Function        app_ble_clinet_fw_removechar
**
** Description     remove char form a string
**
** Parameters       str poinet to a string, remove_char: the char has to be removed
**
**  Returns         none
**
********************************************************************************/
static void app_ble_clinet_fw_removechar(char *str, char remove_char)
{
    UINT32 i,front=0;
    UINT32 len =strlen(str);
    for (i=1;i<len;i++)
    {
        if (str[i]!=remove_char)
            str[front++]=str[i];
    }
    str[front]='\0';
}

/*******************************************************************************
**
** Function        app_ble_clinet_fw_data_append
**
** Description     copy data
**
**
**  Returns         none
**
**
*********************************************************************************/
static void app_ble_clinet_fw_data_append(char *str, UINT32 from, UINT32 end)
{
    memcpy(&app_ble_client_fwdl_fw_upgrade_info->otabinary[app_ble_client_fwdl_fw_upgrade_info->ota_data_len],&str[from],end-from);
    app_ble_client_fwdl_fw_upgrade_info->ota_data_len = app_ble_client_fwdl_fw_upgrade_info->ota_data_len+end-from;
}

/*******************************************************************************
**
** Function        app_ble_client_fw_hex2int
**
** Description     hex number to interger
**
**
**  Returns         the mapped number
**
*********************************************************************************/
static UINT32 app_ble_client_fw_hex2int(char t)
{
    t = toupper(t);
    switch(t)
    {
        case '0':return 0;
        case '1':return 1;
        case '2':return 2;
        case '3':return 3;
        case '4':return 4;
        case '5':return 5;
        case '6':return 6;
        case '7':return 7;
        case '8':return 8;
        case '9':return 9;
        case 'A':return 10;
        case 'B':return 11;
        case 'C':return 12;
        case 'D':return 13;
        case 'E':return 14;
        case 'F':return 15;
        default :return 0;   /* actually should not go to defalut hanlde, unless the hexfile is wrong */
    }
}

/*******************************************************************************
**
** Function        app_ble_clinet_fw_hex2binary
**
** Description     conver hex file to binary format
**
**
**  Returns         None
**
 ***************************************************************************/

static void app_ble_clinet_fw_hex2binary(app_ble_client_fwdl_fw_upgrade_cb *fw)
{
    UINT32 i,left,right;
    for(i=0;i< fw->ota_data_len;i+=2)
    {
        left = app_ble_client_fw_hex2int(fw->otabinary[i])<<4;
        right = app_ble_client_fw_hex2int(fw->otabinary[i+1]);
        fw->otabinary[i/2]=left+right;
    }
}

/*******************************************************************************
**
** Function        app_ble_clinet_fw_cal_chunk_size
**
** Description     calculate each hex chunk size
**
**
**  Returns         the chunk size
**
 ***************************************************************************/
static UINT32 app_ble_clinet_fw_cal_chunk_size(char a, char b)
{
     return app_ble_client_fw_hex2int(a)*16+app_ble_client_fw_hex2int(b);
}
/*******************************************************************************
**
** Function        app_ble_clinet_fw_hexfile2bin
**
** Description     parse Intel hex file and conver to binary format
**
**
**  Returns        status: 0 if success / -1 otherwise
**
 *************************************************************************/
static UINT32 app_ble_clinet_fw_hexfile2bin(char *str)
{
    FILE *fp;
    unsigned char *line;
    int cnt=0;
    UINT32 file_size;
    UINT32 hex_idx=0;
    UINT32 chunk_size=0,actual_data_begin=0,actual_data_end=0;

    fp = fopen(str,"rb");
    if(fp==NULL)
    {
        APP_DEBUG0("file not exist");
        return -1;
    }
    file_size = app_ble_client_fw_get_file_size(str);
    if ((line = malloc(sizeof(unsigned char)*file_size))==NULL)
    {
        APP_DEBUG0("malloc memory fail");
		fclose(fp);
		fp = NULL;
		return -1;
    }
    if ((app_ble_client_fwdl_fw_upgrade_info->otabinary = malloc(sizeof(unsigned char)*(file_size))) ==NULL)
    {
        APP_DEBUG0("malloc memory fail");
		free(line);
		line = NULL;
		fclose(fp);
		fp = NULL;
        return -1;
    }
    fgets(line,20,fp);      //skip 1st line
    fread(line,1,file_size,fp);
    app_ble_clinet_fw_removechar(line,':');

    while(1)
    {
        chunk_size = app_ble_clinet_fw_cal_chunk_size(line[hex_idx],line[hex_idx+1])*2;
        if(line[hex_idx+6]=='0' && line[hex_idx+7]=='1')
        {
            APP_INFO0("hex file handle done");
            break;
        }
        actual_data_begin = APP_BLE_CLIENT_OTAFWDL_SKIP_BYTE + hex_idx;
        actual_data_end = actual_data_begin+chunk_size;
        if (line[hex_idx+6]=='0' && line[hex_idx+7]=='4')
        {
            hex_idx = actual_data_end+APP_BLE_CLIENT_OTAFWDL_STRING_CHECKSUM_LENGTH+APP_BLE_CLIENT_OTAFWDL_STRING_SKIP_CRLF;
            continue;
        }
        else
        {
            app_ble_clinet_fw_data_append(line,actual_data_begin,actual_data_end);
            hex_idx = actual_data_end+APP_BLE_CLIENT_OTAFWDL_STRING_CHECKSUM_LENGTH+APP_BLE_CLIENT_OTAFWDL_STRING_SKIP_CRLF;
        }
    }
    fclose(fp);
    app_ble_clinet_fw_hex2binary(app_ble_client_fwdl_fw_upgrade_info);
    free(line);
    app_ble_client_fwdl_fw_upgrade_info->ota_binary_len = app_ble_client_fwdl_fw_upgrade_info->ota_data_len /2;
    app_ble_client_fwdl_fw_upgrade_info->ota_need_send_cnt = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len / APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN)+1;
    app_ble_clinet_fw_crc_calculate();
    return 0;
}

/*******************************************************************************
**
** Function        app_ble_client_uuid128_convert
**
** Description     convert UUID128 to hex number
**
**
**  Returns         None
**
***************************************************************************/
static void app_ble_client_uuid128_convert(char uuid[],char *buf)
{
   int len =strlen(uuid);
   int i;
   int left,right;
   for(i=0;i<len;i+=2)
   {
       left = app_ble_client_fw_hex2int(uuid[i])<<4 ;
       right = app_ble_client_fw_hex2int(uuid[i+1]);
       buf[(len-i)/2-1]=left +right;
   }


}
/*******************************************************************************
**
** Function        app_ble_client_fw_register_notification
**
** Description     This is the register function to receive a notification
**
** Parameters      None
**
** Returns         status: 0 if success / -1 otherwise
**
*******************************************************************************/
static int app_ble_client_fw_register_notification(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_NOTIFREG ble_notireg_param;
    UINT32  client_num = app_ble_client_fwdl_fw_upgrade_info->client_num;
    status = BSA_BleClNotifRegisterInit(&ble_notireg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClNotifRegisterInit failed status = %d", status);
        return -1;
    }
    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_SERVICE_ID,&ble_notireg_param.notification_id.srvc_id.id.uuid.uu.uuid128);

    ble_notireg_param.notification_id.srvc_id.id.uuid.len = 16;
    ble_notireg_param.notification_id.srvc_id.is_primary = 1;
    ble_notireg_param.notification_id.srvc_id.id.inst_id =0;

    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_CHAR_NOTIFY_ID,&ble_notireg_param.notification_id.char_id.uuid.uu.uuid128);

    ble_notireg_param.notification_id.char_id.uuid.len = 16;
    ble_notireg_param.notification_id.srvc_id.id.inst_id = 0;
    ble_notireg_param.notification_id.srvc_id.is_primary = 1;
    ble_notireg_param.notification_id.char_id.inst_id = 0;
    bdcpy(ble_notireg_param.bd_addr, app_ble_cb.ble_client[client_num].server_addr);
    ble_notireg_param.client_if = app_ble_cb.ble_client[client_num].client_if;


    APP_DEBUG1("size of ble_notireg_param:%d", sizeof(ble_notireg_param));
    status = BSA_BleClNotifRegister(&ble_notireg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClNotifRegister failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_clinet_fw_upgrade_start
 **
 ** Description    indicate the remote the fw upgrade procedure start
 **
 **
 **  Returns         None
 **
 *****************************************************************************/
static void app_ble_clinet_fw_upgrade_start()
{
    tBSA_STATUS status;
    tBSA_BLE_CL_WRITE ble_write_param;
    UINT32 client_num = app_ble_client_fwdl_fw_upgrade_info->client_num;
    status = BSA_BleClWriteInit(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWriteInit failed status = %d", status);
        return -1;
    }
    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_SERVICE_ID,&ble_write_param.char_id.srvc_id.id.uuid.uu.uuid128);
    ble_write_param.char_id.srvc_id.id.uuid.len = 16;
    ble_write_param.char_id.srvc_id.is_primary = 1;
    ble_write_param.char_id.srvc_id.id.inst_id =0;

    ble_write_param.char_id.char_id.uuid.uu.uuid16=0x2902;
    ble_write_param.char_id.char_id.inst_id = 0;
    ble_write_param.char_id.char_id.uuid.len = 2;
    ble_write_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_write_param.auth_req = BTA_GATT_AUTH_REQ_NONE;

    ble_write_param.len = 2;
    ble_write_param.value[0]=0;
    ble_write_param.value[1]=2;
    ble_write_param.descr = FALSE;
    ble_write_param.char_id.srvc_id.id.inst_id= 0;
    ble_write_param.write_type = BTA_GATTC_TYPE_WRITE;

    status = BSA_BleClWrite(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWrite failed status = %d", status);
        return -1;
    }


}

/*******************************************************************************
**
** Function        app_ble_clinet_fw_upgrade_prepare
**
** Description
**
**
**  Returns         None
**
* ***************************************************************************/
static void app_ble_clinet_fw_upgrade_prepare(UINT32 client_num,UINT32 len, unsigned char *data)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_WRITE ble_write_param;
    status = BSA_BleClWriteInit(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWriteInit failed status = %d", status);
        return -1;
    }
    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_SERVICE_ID,&ble_write_param.char_id.srvc_id.id.uuid.uu.uuid128);
    ble_write_param.char_id.srvc_id.id.uuid.len = 16;
    ble_write_param.char_id.srvc_id.is_primary = 1;
    ble_write_param.char_id.srvc_id.id.inst_id =0;

    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_CHAR_NOTIFY_ID,&ble_write_param.char_id.char_id.uuid.uu.uuid128);
    ble_write_param.char_id.char_id.inst_id = 0;
    ble_write_param.char_id.char_id.uuid.len = 16;
    ble_write_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_write_param.auth_req = BTA_GATT_AUTH_REQ_NONE;

    ble_write_param.len = len;
    memcpy(&ble_write_param.value[0],data,len);
    ble_write_param.descr = FALSE;
    ble_write_param.char_id.srvc_id.id.inst_id= 0;
    ble_write_param.write_type = BTA_GATTC_TYPE_WRITE;

    status = BSA_BleClWrite(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWrite failed status = %d", status);
        return -1;
    }

}
/*******************************************************************************
**
** Function        app_ble_clinet_fw_write_data
**
** Description     send fw data to remote
**
**
**  Returns         None
**
 * ***************************************************************************/
static void app_ble_clinet_fw_write_data(UINT32 client_num,UINT32 len, unsigned char *data)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_WRITE ble_write_param;
    status = BSA_BleClWriteInit(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWriteInit failed status = %d", status);
        return -1;
    }
    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_SERVICE_ID,&ble_write_param.char_id.srvc_id.id.uuid.uu.uuid128);
    ble_write_param.char_id.srvc_id.id.uuid.len = 16;
    ble_write_param.char_id.srvc_id.is_primary = 1;
    ble_write_param.char_id.srvc_id.id.inst_id =0;

    app_ble_client_uuid128_convert(APP_BLE_CLIENT_FWDL_OTA_CHAR_HEX_DATA_ID,&ble_write_param.char_id.char_id.uuid.uu.uuid128);
    ble_write_param.char_id.char_id.inst_id = 0;
    ble_write_param.char_id.char_id.uuid.len = 16;
    ble_write_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_write_param.auth_req = BTA_GATT_AUTH_REQ_NONE;

    ble_write_param.len = len;
    memcpy(&ble_write_param.value[0],data,len);
    ble_write_param.descr = FALSE;
    ble_write_param.char_id.srvc_id.id.inst_id= 0;
    ble_write_param.write_type = BTA_GATTC_TYPE_WRITE;

    status = BSA_BleClWrite(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWrite failed status = %d", status);
        return -1;
    }
}
/*******************************************************************************
**
** Function        app_ble_client_fw_handle_upgrade_process
**
** Description     handle fw upgrade state machine
**                 I use app_ble_profile_cback to return the final confirmation
**
**
**  Returns         None
**
 ***************************************************************************/
void app_ble_client_fw_handle_upgrade_process(tBSA_BLE_EVT event ,tBSA_BLE_MSG *p_data)
{
    unsigned char response[5];
    UINT32 data_len;
    UINT32 progresscnt;
    if (app_ble_client_fwdl_fw_upgrade_info == NULL)
        return;

    if (event == BSA_BLE_CL_WRITE_EVT)
    {
        switch(app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state)
        {
            case OTAFU_STATE_PRE_INIT:
                response[0]= APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD;
                app_ble_clinet_fw_upgrade_prepare(0,1,&response);
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state =OTAFU_STATE_WAIT_DOWNLOAD_NOTIFY ;
                break;

            case OTAFU_STATE_REC_DOWNLOAD_NOTIFY:
                app_ble_client_fwdl_fw_upgrade_info->ota_binary_len = app_ble_client_fwdl_fw_upgrade_info->ota_data_len /2;
                if(app_ble_client_fwdl_fw_upgrade_info->ota_binary_len < 65535)
                {
                    response[0] = APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_DOWNLOAD;
                    response[1] = app_ble_client_fwdl_fw_upgrade_info->ota_binary_len & 0xffff;
                    response[2] = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len >> 8) & 0xffff;
                    app_ble_clinet_fw_upgrade_prepare(0,3,&response);
                }
                else
                {
                    response[0] = APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_DOWNLOAD;
                    response[1] = app_ble_client_fwdl_fw_upgrade_info->ota_binary_len & 0xffff;
                    response[2] = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len >> 8) & 0xffff;
                    response[3] = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len >> 16) & 0xffff;
                    response[4] = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len >> 24) & 0xffff;
                    app_ble_clinet_fw_upgrade_prepare(0,5,&response);
                }
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state =OTAFU_STATE_WAIT_DOWNLOAD_LENGTH_NOTIFY ;
                break;

            case OTAFU_STATE_REC_DOWNLOAD_LENGTH_NOTIFY:
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state =OTAFU_STATE_WRITE_DATA ;

            case OTAFU_STATE_WRITE_DATA:
                data_len = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len - app_ble_client_fwdl_fw_upgrade_info->ota_binary_idx )> APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN ?APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN: (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len - app_ble_client_fwdl_fw_upgrade_info->ota_binary_idx );
                app_ble_clinet_fw_write_data(0,data_len,&app_ble_client_fwdl_fw_upgrade_info->otabinary[app_ble_client_fwdl_fw_upgrade_info->ota_binary_idx]);
                app_ble_client_fwdl_fw_upgrade_info->ota_binary_idx += data_len;
                app_ble_client_fwdl_fw_upgrade_info->ota_send_cnt++;
                progresscnt = app_ble_client_fwdl_fw_upgrade_info->ota_send_cnt*100/app_ble_client_fwdl_fw_upgrade_info->ota_need_send_cnt;
                printf("%3d%% []\n\033[F\033[J",progresscnt);
                if(app_ble_client_fwdl_fw_upgrade_info->ota_binary_idx == app_ble_client_fwdl_fw_upgrade_info->ota_binary_len)
                    app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_DATA_VERIFY;
                break;

            case OTAFU_STATE_DATA_VERIFY:
                response[0] = APP_BLE_CLIENT_OTAFWDL_WS_UPGRADE_COMMAND_VERIFY;
                response[1] = app_ble_client_fwdl_fw_upgrade_info->mcrc & 0xffff;
                response[2] = (app_ble_client_fwdl_fw_upgrade_info->mcrc >> 8) & 0xffff;
                response[3] = (app_ble_client_fwdl_fw_upgrade_info->mcrc >> 16) & 0xffff;
                response[4] = (app_ble_client_fwdl_fw_upgrade_info->mcrc >> 24) & 0xffff;
                app_ble_clinet_fw_upgrade_prepare(0,5,&response);
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_DATA_VERIFY_CHECK;
                break;

            case OTAFU_STATE_DATA_VERIFY_CHECK:
                /*nothing to do */
                break;

            default:
                APP_DEBUG0("Error when upgrading firmware");
                app_ble_client_fw_upgrade_done();
                break;

        }
    }
    else if(event == BSA_BLE_CL_NOTIF_EVT )
    {
        switch(app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state)
        {
            case OTAFU_STATE_WAIT_DOWNLOAD_NOTIFY:
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_REC_DOWNLOAD_NOTIFY;
                break;

            case OTAFU_STATE_WAIT_DOWNLOAD_LENGTH_NOTIFY:
                app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_REC_DOWNLOAD_LENGTH_NOTIFY;
                break;

            case OTAFU_STATE_DATA_VERIFY_CHECK:
                if(p_data->cli_notif.value[p_data->cli_notif.len-1] ==0 )
                    app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_UPGRADE_SUCCESS;
                else
                    app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_UPGRADE_FAIL;
                app_ble_client_fw_upgrade_done();
                break;

            default:
                APP_DEBUG0("Error when upgrading firmware");
                app_ble_client_fw_upgrade_done();
                break;
        }
    }
}
/*******************************************************************************
**
** Function        app_ble_client_fw_crc_calculate
**
** Description     start fw CRC calculate
**
**
**  Returns         None
**
****************************************************************************/
static void app_ble_clinet_fw_crc_calculate(void)
{
    UINT32 data_len=0;
    UINT32 binary_idx=0;
    while(1)
    {
        data_len = (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len - binary_idx )> APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN ?APP_BLE_CLIENT_OTAFWDL_MAX_TX_WRITE_PACKET_LEN: (app_ble_client_fwdl_fw_upgrade_info->ota_binary_len - binary_idx );
        app_ble_client_fwdl_fw_upgrade_info->mcrc= app_ble_clinet_fw_crc_slow(app_ble_client_fwdl_fw_upgrade_info->mcrc,&app_ble_client_fwdl_fw_upgrade_info->otabinary[binary_idx],data_len);
        binary_idx += data_len;
        if (binary_idx == app_ble_client_fwdl_fw_upgrade_info->ota_binary_len)
            break;
    }
    app_ble_client_fwdl_fw_upgrade_info->mcrc = app_ble_clinet_fw_crc_complete(app_ble_client_fwdl_fw_upgrade_info->mcrc);
}

/*******************************************************************************
**
** Function        app_ble_clinet_fw_crc_reflect
**
** Description     handle fw upgrade state machine
**
**
**  Returns         None
**
****************************************************************************/
static UINT32 app_ble_clinet_fw_crc_reflect(UINT32 data,UINT32 nbits)
{
    UINT32 app_ble_clinet_fw_crc_reflection = 0x00000000;
    int bit;

    for(bit=0;bit < nbits;++bit)
    {
        if((data & 0x01)!=0)
        {
            app_ble_clinet_fw_crc_reflection |=(1 <<( (nbits-1) -bit ) );
        }
        data = (data >>1);
    }
    return app_ble_clinet_fw_crc_reflection;
}
/*******************************************************************************
**
** Function        app_ble_client_fw_crc_slow
**
** Description     calculate crc slow
**
**
**  Returns         None
**
****************************************************************************/
static UINT32 app_ble_clinet_fw_crc_slow(UINT32 crc32 , unsigned char *data, UINT32 nbytes)
{
    int b,bit;
    for(b=0;b<nbytes;b++)
    {
       crc32 ^= ((app_ble_clinet_fw_crc_reflect_data(data[b])) << (APP_BLE_CLIENT_OTAFWDL_WIDTH-8));
       for(bit = 8;bit > 0;--bit)
       {
          if((crc32 & APP_BLE_CLIENT_OTAFWDL_TOPBIT)!=0)
          {
              crc32 = (crc32 <<1) ^ APP_BLE_CLIENT_OTAFWDL_POLYNOMIAL;
          }
          else
          {
              crc32 =(crc32<<1);
          }
       }
    }
    return crc32;
}
/*******************************************************************************
**
** Function        app_ble_client_fw_crc_complect
**
** Description     calculate the final crc value
**
**
**  Returns         None
**
***************************************************************************/
static UINT32 app_ble_clinet_fw_crc_complete(UINT32 crc32)
{
   return (app_ble_clinet_fw_crc_reflect_remainder(crc32) ^ APP_BLE_CLIENT_OTAFWDL_FINAL_XOR_VALUE);
}
/*******************************************************************************
**
** Function        app_ble_client_fw_crc_reflect_data
**
** Description     crc operation
**
**
**  Returns         None
**
****************************************************************************/
static UINT32 app_ble_clinet_fw_crc_reflect_data(UINT32 data)
{
    return app_ble_clinet_fw_crc_reflect(data,8);
}
/*******************************************************************************
**
** Function        app_ble_client_fw_crc_reflect_remainder
**
** Description
**
**
**  Returns         None
**
***************************************************************************/
static UINT32 app_ble_clinet_fw_crc_reflect_remainder(UINT32 data)
{
    return app_ble_clinet_fw_crc_reflect(data,APP_BLE_CLIENT_OTAFWDL_WIDTH);
}
/*******************************************************************************
**
**   Function        app_ble_client_upgrade_fw
**
** Description     initial fw control block
**
**
**  Returns         none
**
***************************************************************************/
static void app_ble_client_fw_upgrade_init(void)
{
    app_ble_client_fwdl_fw_upgrade_info = (app_ble_client_fwdl_fw_upgrade_cb*)malloc(sizeof(app_ble_client_fwdl_fw_upgrade_cb));
    memset(app_ble_client_fwdl_fw_upgrade_info,0,sizeof(app_ble_client_fwdl_fw_upgrade_cb));
    app_ble_client_fwdl_fw_upgrade_info->mcrc = APP_BLE_CLIENT_OTAFWDL_INITIAL_REMAINDER;
    app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_PRE_INIT;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_fw_upgrade
 **
 ** Description     do preparation for fw upgrade
 **
 **
 ** Returns         int
 **
 ***************************************************************************/
int app_ble_client_fw_upgrade()
{
    int len;
    char filename[100];
    len = app_get_string("Enter fw file name:",filename,sizeof(filename));
    if (len < 0)
    {
        APP_ERROR0("app_get_string failed");
        return -1;
    }
    app_ble_client_fw_upgrade_init();
    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    app_ble_client_fwdl_fw_upgrade_info->client_num = app_get_choice("Select");

    if (app_ble_clinet_fw_hexfile2bin(filename)==0)
    {
        app_ble_client_fw_register_notification();
        app_ble_clinet_fw_upgrade_start();
    }
    else
    {
        app_ble_client_fw_upgrade_done();
    }
    return 0;
}
/*******************************************************************************
**
** Function        app_ble_client_fw_upgrade_done
**
** Description
**
**
**  Returns         None
**
***************************************************************************/
static void  app_ble_client_fw_upgrade_done(void)
{
    if( app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state == OTAFU_STATE_UPGRADE_SUCCESS)
        APP_INFO0("FW upgrade success");
    else if( app_ble_client_fwdl_fw_upgrade_info->ota_upgrade_state = OTAFU_STATE_UPGRADE_FAIL)
        APP_INFO0("FW upgrade fail");
    else
        APP_INFO0("FW upgrade abort");

    if (app_ble_client_fwdl_fw_upgrade_info->otabinary )
        free(app_ble_client_fwdl_fw_upgrade_info->otabinary);
    if (app_ble_client_fwdl_fw_upgrade_info)
    {
        free(app_ble_client_fwdl_fw_upgrade_info);
        app_ble_client_fwdl_fw_upgrade_info = NULL;
    }
}
