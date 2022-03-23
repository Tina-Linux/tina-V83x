/*****************************************************************************
**
**  Name:           app_ble_client.c
**
**  Description:    Bluetooth BLE Client general application
**
**  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include "app_ble.h"
#include "app_xml_utils.h"
#include "app_utils.h"
#include "app_mgt.h"
#include "app_disc.h"
#include "app_dm.h"
#include "app_ble_client_db.h"
#include "app_ble_client.h"
#if (defined APP_BLE_OTA_FW_DL_INCLUDE) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE)
#include "app_ble_client_otafwdl.h"
#endif



/*
 * Global Variables
 */
enum
{
    APP_BLECL_LOAD_ATTR_IDLE,
    APP_BLECL_LOAD_ATTR_START
};

BOOLEAN service_search_pending = FALSE;
/*
 * Local functions
 */


/*
 * BLE common functions
 */

/*******************************************************************************
 **
 ** Function         app_ble_client_find_free_space
 **
 ** Description      find free space for BLE client application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_free_space(void)
{
    int index;

    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(!app_ble_cb.ble_client[index].enabled)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_client_display_attr
 **
 ** Description      Display BLE client's attribute list
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_client_display_attr(tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt)
{
    tAPP_BLE_CLIENT_DB_ATTR *cur_attr;

    cur_attr = p_blecl_db_elmt->p_attr;

    while(cur_attr != NULL)
    {
        if(cur_attr->attr_type == 3)
        {
            APP_INFO1("    %s UUID:0x%04x handle:%d attr_type:%d id:%d prop:0x%x Is_Primary:%d",
                app_ble_display_service_name(cur_attr->attr_UUID.uu.uuid16),
                cur_attr->attr_UUID.uu.uuid16, cur_attr->handle, cur_attr->attr_type,
                cur_attr->id, cur_attr->prop, cur_attr->is_primary);
        }
        else
        {
            APP_INFO1("        UUID:0x%04x handle:%d attr_type:%d id:%d prop:0x%x",
                cur_attr->attr_UUID.uu.uuid16, cur_attr->handle, cur_attr->attr_type,
                cur_attr->id, cur_attr->prop);
        }

        cur_attr = cur_attr->next;
    }

}


/*******************************************************************************
 **
 ** Function         app_ble_client_display
 **
 ** Description      Display BLE client
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_client_display(int detail)
{
    int index;
    tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt;

    APP_INFO0("BLE CLIENT LIST");
    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_client[index].enabled)
        {
            if(app_ble_cb.ble_client[index].conn_id == BSA_BLE_INVALID_CONN)
            {
                APP_INFO1("Index:%d client_if:%d", index, app_ble_cb.ble_client[index].client_if);
            }
            else
            {
                APP_INFO1("Index:%d client_if:%d conn_id:%d BDA:%02X:%02X:%02X:%02X:%02X:%02X",
                  index,
                  app_ble_cb.ble_client[index].client_if,
                  app_ble_cb.ble_client[index].conn_id,
                  app_ble_cb.ble_client[index].server_addr[0],
                  app_ble_cb.ble_client[index].server_addr[1],
                  app_ble_cb.ble_client[index].server_addr[2],
                  app_ble_cb.ble_client[index].server_addr[3],
                  app_ble_cb.ble_client[index].server_addr[4],
                  app_ble_cb.ble_client[index].server_addr[5]);
                if(detail)
                {
                    p_blecl_db_elmt = app_ble_client_db_find_by_bda(app_ble_cb.ble_client[index].server_addr);
                    if(p_blecl_db_elmt)
                    {
                        app_ble_client_display_attr(p_blecl_db_elmt);
                    }
                }
            }
        }
    }
    APP_INFO0("");

}

/*******************************************************************************
 **
 ** Function         app_ble_client_find_reg_pending_index
 **
 ** Description      find registration pending index
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_reg_pending_index(void)
{
    int index;

    for(index = 0 ; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_client[index].enabled == TRUE &&
            (app_ble_cb.ble_client[index].client_if == BSA_BLE_INVALID_IF))
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_client_find_index_by_interface
 **
 ** Description      find BLE client index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_index_by_interface(tBSA_BLE_IF if_num)
{
    int index;

    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_client[index].client_if == if_num)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_client_find_index_by_conn_id
 **
 ** Description      find BLE client index by connection ID
 **
 ** Parameters       conn_id: Connection ID
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_index_by_conn_id(UINT16 conn_id)
{
    int index;

    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_client[index].conn_id == conn_id)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_client_find_conn_id_by_interface
 **
 ** Description      find BLE client conn index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_client_find_conn_id_by_interface(tBSA_BLE_IF if_num)
{
    int index;

    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_client[index].client_if == if_num)
        {
            return app_ble_cb.ble_client[index].conn_id;
        }
    }
    return -1;
}


/*
 * BLE Client functions
 */


/*******************************************************************************
 **
 ** Function        app_ble_client_register
 **
 ** Description     This is the ble client register command
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_register(UINT16 uuid)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_REGISTER ble_appreg_param;
    UINT16 choice;
    int index;

    if(!uuid)
    {
        APP_INFO0("Bluetooth BLE register menu:");
        choice = app_get_choice("Register app UUID(eg. x8000)");
    }
    else
    {
        choice = uuid;
    }
    index = app_ble_client_find_free_space();
    if( index < 0)
    {
        APP_ERROR0("app_ble_client_register maximum number of BLE client already registered");
        return -1;
    }
    status = BSA_BleClAppRegisterInit(&ble_appreg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BLE client registration failed with status %d", status);
        return -1;
    }

    ble_appreg_param.uuid.len = 2;
    ble_appreg_param.uuid.uu.uuid16 = choice;
    ble_appreg_param.p_cback = app_ble_client_profile_cback;

    //BTM_BLE_SEC_NONE: No authentication and no encryption
    //BTM_BLE_SEC_ENCRYPT: encrypt the link with current key
    //BTM_BLE_SEC_ENCRYPT_NO_MITM: unauthenticated encryption
    //BTM_BLE_SEC_ENCRYPT_MITM: authenticated encryption
    ble_appreg_param.sec_act = BTM_BLE_SEC_NONE;

    status = BSA_BleClAppRegister(&ble_appreg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClAppRegister failed status = %d", status);
        return -1;
    }
    app_ble_cb.ble_client[index].enabled = TRUE;
    app_ble_cb.ble_client[index].client_if = ble_appreg_param.client_if;

    return 0;
}



/*******************************************************************************
 **
 ** Function        app_ble_client_service_search
 **
 ** Description     This is the ble search service to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_service_search(void)
{
    tBSA_STATUS status;
    int choice;
    UINT16 service = 0;

    APP_INFO0("Bluetooth BLE service search menu:");
    APP_INFO0("    0 Search all services");
    APP_INFO0("    1 Search a service");
    choice = app_get_choice("Select");

    if (choice == 0)
    {
        service = 0;
    }
    else if (choice == 1)
    {
        service = app_get_choice("Enter Service UUID to search(eg. x1800)");
    }
    else
    {
        APP_ERROR0("Bad choice (0) or (1) only]");
        return -1;
    }

    status = app_ble_client_service_search_execute(service);
    if (status != BSA_SUCCESS)
    {
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_service_search_execute
 **
 ** Description     This is the ble search service to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_service_search_execute(UINT16 service)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_SEARCH ble_search_param;
    int client_num;

    status = BSA_BleClSearchInit(&ble_search_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClServiceSearchReqInit failed status = %d", status);
    }
    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) || (client_num >= BSA_BLE_CLIENT_MAX))
    {
        APP_ERROR1("BSA_BleClServiceSearchReq failed Invalid client_num:%d", client_num);
        return -1;
    }
    ble_search_param.conn_id= app_ble_cb.ble_client[client_num].conn_id;
    if (service != 0)
    {
        ble_search_param.uuid.len = 2;
        ble_search_param.uuid.uu.uuid16 = service;
    }
    status = BSA_BleClSearch(&ble_search_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClServiceSearchReq failed for service:0x%x with status:%d",
                service, status);
        return -1;
    }
    service_search_pending = TRUE;
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_read
 **
 ** Description     This is the read function to read a characteristec or characteristic descriptor from BLE server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id, descr_id;
    int client_num;
    int is_descript;
    int ser_inst_id, char_inst_id, is_primary;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("app_ble_client_read client %d was not enabled yet", client_num);
        return -1;
    }
    service = app_get_choice("Enter Service UUID to read(eg. x1800)");
    is_primary = app_get_choice("Enter Is_primary value(eg:0,1)");
    ser_inst_id = app_get_choice("Enter Instance ID for Service UUID(eg. 0,1,2..)");
    char_id = app_get_choice("Enter Char UUID to read(eg. x2a00)");
    char_inst_id = app_get_choice("Enter Instance ID for Char UUID(eg. 0,1,2..)");

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    is_descript = app_get_choice("Select descriptor? (yes=1 or no=0)");
    if(is_descript == 1)
    {
        ble_read_param.descr_id.char_id.char_id.uuid.len = 2;
        ble_read_param.descr_id.char_id.char_id.uuid.uu.uuid16 = char_id;
        ble_read_param.descr_id.char_id.srvc_id.id.uuid.len = 2;
        ble_read_param.descr_id.char_id.srvc_id.id.uuid.uu.uuid16 = service;
        ble_read_param.descr_id.char_id.srvc_id.is_primary = is_primary;

        descr_id = app_get_choice("Enter Descriptor type UUID to read(eg. x2902)");
        ble_read_param.descr_id.descr_id.uuid.uu.uuid16 = descr_id;
        ble_read_param.descr_id.descr_id.uuid.len = 2;

        ble_read_param.descr = TRUE;
    }
    else
    {
        ble_read_param.char_id.srvc_id.id.inst_id = ser_inst_id;
        ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
        ble_read_param.char_id.srvc_id.id.uuid.len = 2;
        ble_read_param.char_id.srvc_id.is_primary = is_primary;

        ble_read_param.char_id.char_id.inst_id = char_inst_id;
        ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
        ble_read_param.char_id.char_id.uuid.len = 2;
        ble_read_param.descr = FALSE;
    }
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;
    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_write
 **
 ** Description     This is the write function to write a characteristic or characteristic discriptor to BLE server.
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_write(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_WRITE ble_write_param;
    UINT16 len, char_id, index, service, descr_id;
    int client_num, is_descript;
    UINT8 data[100];
    int ser_inst_id, char_inst_id, is_primary;
    UINT8 desc_value;
    UINT8 write_type=0;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");
    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }

    if(app_ble_cb.ble_client[client_num].write_pending ||
        app_ble_cb.ble_client[client_num].congested)
    {
        APP_ERROR1("fail : write pending(%d), congested(%d)!",
            app_ble_cb.ble_client[client_num].write_pending,
            app_ble_cb.ble_client[client_num].congested);
        return -1;
    }

    status = BSA_BleClWriteInit(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWriteInit failed status = %d", status);
        return -1;
    }

    service = app_get_choice("Enter Service UUID to write(eg. x180d)");
    is_primary = app_get_choice("Enter Is_primary value(eg:0,1)");
    ser_inst_id = app_get_choice("Enter Instance ID for Service UUID(eg. 0,1,2..)");
    char_id = app_get_choice("Enter Char UUID to write(eg. x2902)");
    char_inst_id = app_get_choice("Enter Instance ID for Char UUID(eg. 0,1,2..)");

    is_descript = app_get_choice("select descriptor? (yes=1 or no=0)");
    if(is_descript == 1)
    {
        ble_write_param.descr_id.char_id.char_id.uuid.len=2;
        ble_write_param.descr_id.char_id.char_id.uuid.uu.uuid16 = char_id;
        ble_write_param.descr_id.char_id.srvc_id.id.uuid.len=2;
        ble_write_param.descr_id.char_id.srvc_id.id.uuid.uu.uuid16 = service;
        ble_write_param.descr_id.char_id.srvc_id.is_primary = is_primary;

        descr_id = app_get_choice("Enter Descriptor type UUID to write(eg. x2902)");
        desc_value = app_get_choice("Enter Descriptor value to write(eg. x01)");
        ble_write_param.descr_id.descr_id.uuid.uu.uuid16 = descr_id;
        ble_write_param.descr_id.descr_id.uuid.len = 2;
        ble_write_param.len = 1;
        ble_write_param.value[0] = desc_value;
        ble_write_param.descr = TRUE;
    }
    else
    {
        len = app_get_choice("Enter number of bytes to write (eg. 1 or 2)");
        for (index = 0; index < len ; index++)
        {
            data[index]= app_get_choice("Enter data in byte (eg. 1 or 2)");
            ble_write_param.value[index] = data[index];
        }
        ble_write_param.len = len;
        ble_write_param.descr = FALSE;
        ble_write_param.char_id.srvc_id.id.inst_id= ser_inst_id;
        ble_write_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
        ble_write_param.char_id.srvc_id.id.uuid.len = 2;
        ble_write_param.char_id.srvc_id.is_primary = is_primary;

        ble_write_param.char_id.char_id.inst_id = char_inst_id;
        ble_write_param.char_id.char_id.uuid.uu.uuid16 = char_id;
        ble_write_param.char_id.char_id.uuid.len = 2;
    }

    ble_write_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_write_param.auth_req = BTA_GATT_AUTH_REQ_NONE;
    write_type= app_get_choice("Enter WRITE TYPE [i.e 1-GATT_WRITE_NO_RSP 2-GATT_WRITE]");
    APP_INFO1("BSA_BleClWrite write_type:%d", write_type);
    if (write_type == BTA_GATTC_TYPE_WRITE_NO_RSP)
    {
        ble_write_param.write_type = BTA_GATTC_TYPE_WRITE_NO_RSP;
    }
    else if (write_type == BTA_GATTC_TYPE_WRITE)
    {
        ble_write_param.write_type = BTA_GATTC_TYPE_WRITE;
    }
    else
    {
        APP_ERROR1("BSA_BleClWrite failed wrong write_type:%d", write_type);
        return -1;
    }

    status = BSA_BleClWrite(&ble_write_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClWrite failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_register_notification
 **
 ** Description     This is the register function to receive a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_register_notification(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_NOTIFREG ble_notireg_param;
    UINT16 service_id, character_id;

    int client_num, uuid;
    int ser_inst_id, char_inst_id, is_primary;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");
    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }
    status = BSA_BleClNotifRegisterInit(&ble_notireg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClNotifRegisterInit failed status = %d", status);
        return -1;
    }
    if((uuid = app_get_choice("Enter Service UUID to register(eg. x1800)"))<0)
    {
        APP_ERROR1("Invalid Service UUID = %d", uuid);
        return -1;
    }
    else
    {
        service_id = uuid;
    }

    if((uuid = app_get_choice("Enter Char UUID to register(eg. x2a00)"))<0)
    {
        APP_ERROR1("Invalid Char UUID = %d", uuid);
        return -1;
    }
    else
    {
        character_id = uuid;
    }
    is_primary = app_get_choice("Enter Is_primary value(eg:0,1)");
    ser_inst_id = app_get_choice("Enter Instance ID for Service UUID(eg. 0,1,2..)");
    char_inst_id = app_get_choice("Enter Instance ID for Char UUID(eg. 0,1,2..)");
    ble_notireg_param.notification_id.srvc_id.id.uuid.len = 2;
    ble_notireg_param.notification_id.srvc_id.id.uuid.uu.uuid16 = service_id;
    ble_notireg_param.notification_id.char_id.uuid.len = 2;
    ble_notireg_param.notification_id.char_id.uuid.uu.uuid16 = character_id;
    ble_notireg_param.notification_id.srvc_id.id.inst_id = ser_inst_id;
    ble_notireg_param.notification_id.srvc_id.is_primary = is_primary;
    ble_notireg_param.notification_id.char_id.inst_id = char_inst_id;
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
 ** Function        app_ble_client_open
 **
 ** Description     This is the ble open connection to ble server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_open(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_OPEN ble_open_param;
    int device_index;
    BD_ADDR bd_addr;
    int client_num;
    int direct;

    APP_INFO0("Bluetooth BLE connect menu:");
    APP_INFO0("    0 Device from XML database (already paired)");
    APP_INFO0("    1 Device found in last discovery");
    device_index = app_get_choice("Select source");
    /* Devices from XML database */
    if (device_index == 0)
    {
        /* Read the XML file which contains all the bonded devices */
        app_read_xml_remote_devices();

        app_xml_display_devices(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db));
        device_index = app_get_choice("Select device");
        if ((device_index >= 0) &&
            (device_index < APP_NUM_ELEMENTS(app_xml_remote_devices_db)) &&
            (app_xml_remote_devices_db[device_index].in_use != FALSE))
        {
            bdcpy(bd_addr, app_xml_remote_devices_db[device_index].bd_addr);
        }
        else
        {
            APP_ERROR1("Bad Device Index:%d\n", device_index);
            return -1;
        }

    }
    /* Devices from Discovery */
    else if (device_index == 1)
    {
        app_disc_display_devices();
        device_index = app_get_choice("Select device");
        if ((device_index >= 0) &&
            (device_index < APP_DISC_NB_DEVICES) &&
            (app_discovery_cb.devs[device_index].in_use != FALSE))
        {
            bdcpy(bd_addr, app_discovery_cb.devs[device_index].device.bd_addr);
        }
    }
    else
    {
        APP_ERROR0("Bad choice [XML(0) or Disc(1) only]");
        return -1;
    }

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }

    if (app_ble_cb.ble_client[client_num].conn_id != BSA_BLE_INVALID_CONN)
    {
        APP_ERROR1("Connection already exist, conn_id = %d", app_ble_cb.ble_client[client_num].conn_id );
        return -1;
    }

    status = BSA_BleClConnectInit(&ble_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClConnectInit failed status = %d", status);
        return -1;
    }

    ble_open_param.client_if = app_ble_cb.ble_client[client_num].client_if;
    direct = app_get_choice("Direct connection:1, Background connection:0");
    if(direct == 1)
    {
        ble_open_param.is_direct = TRUE;
    }
    else if(direct == 0)
    {
        ble_open_param.is_direct = FALSE;
    }
    else
    {
        APP_ERROR1("Wrong selection! = %d", direct);
        return -1;
    }
    bdcpy(app_ble_cb.ble_client[client_num].server_addr, bd_addr);
    bdcpy(ble_open_param.bd_addr, bd_addr);

    status = BSA_BleClConnect(&ble_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClConnect failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_close(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_CLOSE ble_close_param;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }
    status = BSA_BleClCloseInit(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClCLoseInit failed status = %d", status);
        return -1;
    }
    ble_close_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    status = BSA_BleClClose(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClClose failed status = %d", status);
        return -1;
    }

    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_unpair
 **
 ** Description     Unpair an LE device
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_unpair(void)
{
    tBSA_STATUS status;
    tBSA_SEC_REMOVE_DEV sec_remove_dev;
    tBSA_BLE_CL_CLOSE ble_close_param;
    int client_num;
    tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt;

    APP_INFO0("app_ble_client_unpair");

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }

    /*First close any active connection if exist*/
    if(app_ble_cb.ble_client[client_num].conn_id != BSA_BLE_INVALID_CONN)
    {
        status = BSA_BleClCloseInit(&ble_close_param);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_BleClCLoseInit failed status = %d", status);
            return -1;
        }
        ble_close_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
        status = BSA_BleClClose(&ble_close_param);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_BleClClose failed status = %d", status);
            return -1;
        }
    }

    /* Remove the device from Security database (BSA Server) */
    BSA_SecRemoveDeviceInit(&sec_remove_dev);

    /* Read the XML file which contains all the bonded devices */
    app_read_xml_remote_devices();

    if ((client_num >= 0) &&
        (client_num < APP_MAX_NB_REMOTE_STORED_DEVICES) &&
        (app_xml_remote_devices_db[client_num].in_use != FALSE))
    {
        bdcpy(sec_remove_dev.bd_addr, app_xml_remote_devices_db[client_num].bd_addr);
        status = BSA_SecRemoveDevice(&sec_remove_dev);
    }
    else
    {
        APP_ERROR1("Wrong input [%d]",client_num);
        return -1;
    }

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_SecRemoveDevice Operation Failed with status [%d]",status);
        return -1;
    }
    else
    {
        /* delete the device from database */
        app_xml_remote_devices_db[client_num].in_use = FALSE;
        app_write_xml_remote_devices();
    }

    APP_INFO0("Remove device from BLE DB");
    APP_DEBUG1("BDA:%02X:%02X:%02X:%02X:%02X:%02X",
              app_xml_remote_devices_db[client_num].bd_addr[0], app_xml_remote_devices_db[client_num].bd_addr[1],
              app_xml_remote_devices_db[client_num].bd_addr[2], app_xml_remote_devices_db[client_num].bd_addr[3],
              app_xml_remote_devices_db[client_num].bd_addr[4], app_xml_remote_devices_db[client_num].bd_addr[5]);

    /* search BLE DB */
    p_blecl_db_elmt = app_ble_client_db_find_by_bda(app_xml_remote_devices_db[client_num].bd_addr);
    if(p_blecl_db_elmt != NULL)
    {
        APP_DEBUG0("Device present in DB, So clear it!!");
        app_ble_client_db_clear_by_bda(app_xml_remote_devices_db[client_num].bd_addr);
        app_ble_client_db_save();
    }
    else
    {
        APP_DEBUG0("No device present");
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_deregister
 **
 ** Description     This is the ble deregister app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_DEREGISTER ble_appdereg_param;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }
    status = BSA_BleClAppDeregisterInit(&ble_appdereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClAppDeregisterInit failed status = %d", status);
        return -1;
    }
    ble_appdereg_param.client_if = app_ble_cb.ble_client[client_num].client_if;
    status = BSA_BleClAppDeregister(&ble_appdereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleAppDeregister failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_deregister_all
 **
 ** Description     Deregister all apps
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister_all(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_DEREGISTER ble_appdereg_param;
    int index;

    for(index = 0;index < BSA_BLE_CLIENT_MAX;index++)
    {
        if(app_ble_cb.ble_client[index].enabled)
        {
            APP_DEBUG1("deregister app:%d",index);
            status = BSA_BleClAppDeregisterInit(&ble_appdereg_param);
            if (status != BSA_SUCCESS)
            {
                APP_ERROR1("BSA_BleClAppDeregisterInit failed status = %d", status);
                return -1;
            }
            ble_appdereg_param.client_if = app_ble_cb.ble_client[index].client_if;
            status = BSA_BleClAppDeregister(&ble_appdereg_param);
            if (status != BSA_SUCCESS)
            {
                APP_ERROR1("BSA_BleAppDeregister failed status = %d", status);
                return -1;
            }
        }
    }

    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_deregister_notification
 **
 ** Description     This is the deregister function for a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_deregister_notification(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_NOTIFDEREG ble_notidereg_param;
    UINT16 service_id, character_id, all;
    int client_num, uuid;
    int ser_inst_id, char_inst_id, is_primary;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if((client_num < 0) ||
       (client_num >= BSA_BLE_CLIENT_MAX) ||
       (app_ble_cb.ble_client[client_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong client number! = %d", client_num);
        return -1;
    }
    status = BSA_BleClNotifDeregisterInit(&ble_notidereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClNotifDeregisterInit failed status = %d", status);
        return -1;
    }
    bdcpy(ble_notidereg_param.bd_addr, app_ble_cb.ble_client[client_num].server_addr);
    ble_notidereg_param.client_if = app_ble_cb.ble_client[client_num].client_if;

    all = app_get_choice("0 = deregister all notifications, 1 = deregister a notification");
    if(all == 0)
    {
        APP_INFO0("Deregister all notifications");
    }
    else if(all == 1)
    {
        if((uuid = app_get_choice("Enter Service UUID to deregister(eg. x1800)"))<0)
        {
            APP_ERROR1("Invalid Service UUID = %d", uuid);
            return -1;
        }
        else
        {
            service_id = uuid;
        }

        if((uuid = app_get_choice("Enter Char UUID to deregister(eg. x2a00)"))<0)
        {
            APP_ERROR1("Invalid Char UUID = %d", uuid);
            return -1;
        }
        else
        {
            character_id = uuid;
        }

        is_primary = app_get_choice("Enter Is_primary value(eg:0,1)");
        ser_inst_id = app_get_choice("Enter Instance ID for Service UUID(eg. 0,1,2..)");
        char_inst_id = app_get_choice("Enter Instance ID for Char UUID(eg. 0,1,2..)");

        ble_notidereg_param.notification_id.srvc_id.id.uuid.len = 2;
        ble_notidereg_param.notification_id.srvc_id.id.uuid.uu.uuid16 = service_id;
        ble_notidereg_param.notification_id.char_id.uuid.len = 2;
        ble_notidereg_param.notification_id.char_id.uuid.uu.uuid16 = character_id;
        ble_notidereg_param.notification_id.srvc_id.id.inst_id = ser_inst_id;
        ble_notidereg_param.notification_id.srvc_id.is_primary = is_primary;
        ble_notidereg_param.notification_id.char_id.inst_id = char_inst_id;
        bdcpy(ble_notidereg_param.bd_addr, app_ble_cb.ble_client[client_num].server_addr);
        ble_notidereg_param.client_if = app_ble_cb.ble_client[client_num].client_if;

        APP_DEBUG1("size of ble_notidereg_param:%d", sizeof(ble_notidereg_param));
    }
    else
    {
        APP_ERROR1("wrong select = %d", all);
        return -1;
    }
    status = BSA_BleClNotifDeregister(&ble_notidereg_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleClNotifDeregister failed status = %d", status);
        return -1;
    }
    return 0;
}

static int number_of_load_attr = 0;
static int state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
/*******************************************************************************
 **
 ** Function        app_ble_client_load_attr
 **
 ** Description     This is the cache load function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_load_attr(tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt,
                             BD_ADDR bd_addr, UINT16 conn_id)
{
    int index = 0;
    int skip_count = 0;
    int sent = 0;
    tBSA_STATUS status;

    tBSA_BLE_CL_CACHE_LOAD cache_load;
    tAPP_BLE_CLIENT_DB_ATTR *cur_attr;

    BSA_BleClCacheLoadInit(&cache_load);

    /* in case of new device */
    if(p_blecl_db_elmt == NULL)
    {
        bdcpy(cache_load.bd_addr, bd_addr);
        cache_load.conn_id = conn_id;
        cache_load.num_attr = 0;
        cache_load.more = 0;
        status = BSA_BleClCacheLoad(&cache_load);
        if(status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
            return -1;
        }
        state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
        number_of_load_attr = 0;
        return 1;
    }

    cur_attr = p_blecl_db_elmt->p_attr;

    if((number_of_load_attr == 0) && (state_of_load_attr == APP_BLECL_LOAD_ATTR_IDLE)) /* first load attribute */
    {
        state_of_load_attr = APP_BLECL_LOAD_ATTR_START;
        if(p_blecl_db_elmt->p_attr == NULL) /* no attribute in cache */
        {
            bdcpy(cache_load.bd_addr, bd_addr);
            cache_load.conn_id = conn_id;
            cache_load.num_attr = 0;
            cache_load.more = 0;
            status = BSA_BleClCacheLoad(&cache_load);
            if(status != BSA_SUCCESS)
            {
                APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
                return -1;
            }
            state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
            number_of_load_attr = 0;
            sent = 1;
        }
        else /* Cache has attribute list */
        {
            while((cur_attr != NULL) && !(sent))
            {
                if(index == BSA_BLE_CL_NV_LOAD_MAX)
                {
                    bdcpy(cache_load.bd_addr, bd_addr);
                    cache_load.conn_id = conn_id;
                    cache_load.num_attr = index;
                    if(cur_attr->next)
                    {
                        cache_load.more = 1;
                        state_of_load_attr = APP_BLECL_LOAD_ATTR_START;
                        number_of_load_attr = index;
                    }
                    else
                    {
                        cache_load.more = 0;
                        state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
                        number_of_load_attr = 0;
                    }
                    status = BSA_BleClCacheLoad(&cache_load);
                    if(status != BSA_SUCCESS)
                    {
                        APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
                        return -1;
                    }
                    sent = 1;
                }
                else
                {
                    cache_load.attr[index].uuid = cur_attr->attr_UUID;
                    cache_load.attr[index].s_handle = cur_attr->handle;
                    cache_load.attr[index].attr_type = cur_attr->attr_type;
                    cache_load.attr[index].id = cur_attr->id;
                    cache_load.attr[index].prop = cur_attr->prop;
                    cache_load.attr[index].is_primary = cur_attr->is_primary;
                    APP_DEBUG1("\t Attr[0x%04x] handle[%d] uuid[0x%04x] inst[%d] type[%d] prop[0x%1x] is_primary[%d]",
                              index, cache_load.attr[index].s_handle,
                              cache_load.attr[index].uuid.uu.uuid16,
                              cache_load.attr[index].id,
                              cache_load.attr[index].attr_type,
                              cache_load.attr[index].prop,
                              cache_load.attr[index].is_primary);
                    index++;
                    cur_attr = cur_attr->next;
                }
            }
            if(index && !(sent))
            {
                bdcpy(cache_load.bd_addr, bd_addr);
                cache_load.conn_id = conn_id;
                cache_load.num_attr = index+1;
                cache_load.more = 0;
                status = BSA_BleClCacheLoad(&cache_load);
                if(status != BSA_SUCCESS)
                {
                    APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
                    return -1;
                }
                state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
                number_of_load_attr = 0;
                sent = 1;
            }
        }
    }
    /* Need to send pending attributes to BSA server */
    else if((number_of_load_attr != 0) && (state_of_load_attr == APP_BLECL_LOAD_ATTR_START))
    {
        while(skip_count < (number_of_load_attr))/* skip attributes which was sent already */
        {
            cur_attr = cur_attr->next;
            skip_count++;
        }
        while((cur_attr != NULL) && (!sent))
        {
            if(index == BSA_BLE_CL_NV_LOAD_MAX)
            {
                bdcpy(cache_load.bd_addr, bd_addr);
                cache_load.conn_id = conn_id;
                cache_load.num_attr = index;
                if(cur_attr->next)
                {
                    cache_load.more = 1;
                    state_of_load_attr = APP_BLECL_LOAD_ATTR_START;
                    number_of_load_attr = number_of_load_attr + index;
                }
                else
                {
                    cache_load.more = 0;
                    state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
                    number_of_load_attr = 0;
                }
                status = BSA_BleClCacheLoad(&cache_load);
                if(status != BSA_SUCCESS)
                {
                    APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
                    return -1;
                }
                sent = 1;
            }
            else
            {
                cache_load.attr[index].uuid = cur_attr->attr_UUID;
                cache_load.attr[index].s_handle = cur_attr->handle;
                cache_load.attr[index].attr_type = cur_attr->attr_type;
                cache_load.attr[index].id = cur_attr->id;
                cache_load.attr[index].prop = cur_attr->prop;
                cache_load.attr[index].is_primary = cur_attr->is_primary;
                APP_DEBUG1("\t Attr[0x%04x] handle[%d] uuid[0x%04x] inst[%d] type[%d] prop[0x%1x] is_primary[%d]",
                              index, cache_load.attr[index].s_handle,
                              cache_load.attr[index].uuid.uu.uuid16,
                              cache_load.attr[index].id,
                              cache_load.attr[index].attr_type,
                              cache_load.attr[index].prop,
                              cache_load.attr[index].is_primary);
                index++;
                cur_attr = cur_attr->next;
            }
        }
        if(index && !(sent))
        {
            bdcpy(cache_load.bd_addr, bd_addr);
            cache_load.conn_id = conn_id;
            cache_load.num_attr = index;
            cache_load.more = 0;
            status = BSA_BleClCacheLoad(&cache_load);
            if(status != BSA_SUCCESS)
            {
                APP_ERROR1("BSA_BleClCacheLoad failed status = %d", status);
                return -1;
            }
            state_of_load_attr = APP_BLECL_LOAD_ATTR_IDLE;
            number_of_load_attr = 0;
            sent = 1;
        }

    }
    else
    {
        APP_ERROR1("Abnormal situation number_of_load_attr:%d, state_of_load_attr:%d",
                    number_of_load_attr, state_of_load_attr);
    }
    return 1;
}

/*******************************************************************************
**
** Function         app_ble_client_profile_cback
**
** Description      BLE Client Profile callback.
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data)
{
    int index;
    int client_num;
    int status;
    tAPP_BLE_CLIENT_DB_ELEMENT *p_blecl_db_elmt;
    tAPP_BLE_CLIENT_DB_ATTR *p_blecl_db_attr;
    tBSA_BLE_CL_INDCONF ind_conf;
#if (defined APP_BLE_OTA_FW_DL_INCLUDED) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE)
    app_ble_client_fw_handle_upgrade_process(event,p_data);
#endif
    switch (event)
    {
        case BSA_BLE_CL_DEREGISTER_EVT:
            APP_INFO1("BSA_BLE_CL_DEREG_EVT cif = %d status=%d", p_data->cli_deregister.client_if,
                              p_data->cli_deregister.status );
            client_num = app_ble_client_find_index_by_interface(p_data->cli_deregister.client_if);
            if(client_num < 0)
            {
                APP_ERROR0("No client!!");
                break;
            }
            app_ble_cb.ble_client[client_num].enabled = FALSE;
            app_ble_cb.ble_client[client_num].client_if = BSA_BLE_INVALID_IF;
            app_ble_cb.ble_client[client_num].conn_id = BSA_BLE_INVALID_CONN;
            break;

        case BSA_BLE_CL_OPEN_EVT:
            APP_INFO1("BSA_BLE_CL_OPEN_EVT client_if= %d status=%d",
                              p_data->cli_open.client_if, p_data->cli_open.status );
            if (p_data->cli_open.status == BSA_SUCCESS)
            {
                client_num = app_ble_client_find_index_by_interface(p_data->cli_open.client_if);
                if(client_num < 0)
                {
                    APP_ERROR0("No client!!");
                    break;
                }
                app_ble_cb.ble_client[client_num].conn_id = p_data->cli_open.conn_id;
                /* XML Database update */
                app_read_xml_remote_devices();
                /* Add BLE service for this devices in XML database */
                app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                        APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->cli_open.bd_addr,
                        BSA_BLE_SERVICE_MASK);
                /* Check if the name in the inquiry responses database */
                for (index = 0; index < APP_NUM_ELEMENTS(app_discovery_cb.devs); index++)
                {
                    if (!bdcmp(app_discovery_cb.devs[index].device.bd_addr, p_data->cli_open.bd_addr))
                    {
                        app_xml_update_name_db(app_xml_remote_devices_db,
                                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->cli_open.bd_addr,
                                app_discovery_cb.devs[index].device.name);
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE) /* BLE Connection from DB */
                        app_xml_update_ble_addr_type_db(app_xml_remote_devices_db,
                                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->cli_open.bd_addr,
                                app_discovery_cb.devs[index].device.ble_addr_type);
                        app_xml_update_device_type_db(app_xml_remote_devices_db,
                                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->cli_open.bd_addr,
                                app_discovery_cb.devs[index].device.device_type);
                        app_xml_update_inq_result_type_db(app_xml_remote_devices_db,
                                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->cli_open.bd_addr,
                                app_discovery_cb.devs[index].device.inq_result_type);
#endif
                        break;
                    }
                }
                status = app_write_xml_remote_devices();
                if (status < 0)
                {
                    APP_ERROR1("app_ble_write_remote_devices failed: %d", status);
                }
                /* check DB first */
                p_blecl_db_elmt = app_ble_client_db_find_by_bda(p_data->cli_open.bd_addr);
                if(p_blecl_db_elmt == NULL) /* this is new device */
                {
                    APP_DEBUG0("New device. Update BLE DB!!");
                    /* update BLE DB */
                    p_blecl_db_elmt = app_ble_client_db_alloc_element();
                    if(p_blecl_db_elmt != NULL)
                    {
                        bdcpy(p_blecl_db_elmt->bd_addr, p_data->cli_open.bd_addr);
                        p_blecl_db_elmt->app_handle = p_data->cli_open.client_if;
                        app_ble_client_db_add_element(p_blecl_db_elmt);
                        APP_DEBUG1("save BDA:%02X:%02X:%02X:%02X:%02X:%02X, client_if= %d ",
                              p_blecl_db_elmt->bd_addr[0], p_blecl_db_elmt->bd_addr[1],
                              p_blecl_db_elmt->bd_addr[2], p_blecl_db_elmt->bd_addr[3],
                              p_blecl_db_elmt->bd_addr[4], p_blecl_db_elmt->bd_addr[5],
                              p_data->cli_open.client_if);
                        app_ble_client_db_save();
                    }
                }
                else
                {
                    APP_DEBUG0("already exist in BLE DB!!");
                }
            }
            break;

        case BSA_BLE_CL_SEARCH_RES_EVT:
            APP_INFO1("BSA_BLE_CL_SEARCH_RES_EVT: conn_id = %d",
                    p_data->cli_search_res.conn_id);
            /* always assume 16 bits UUID */
            APP_INFO1(" Searched Service name:%s(uuid:0x%04x)",
                    app_ble_display_service_name(p_data->cli_search_res.service_uuid.id.uuid.uu.uuid16),
                    p_data->cli_search_res.service_uuid.id.uuid.uu.uuid16);
            service_search_pending = FALSE;
            break;

        case BSA_BLE_CL_SEARCH_CMPL_EVT:
            APP_INFO1("BSA_BLE_CL_SEARCH_CMPL_EVT = %d",
                    p_data->cli_search_cmpl.status);
            if (service_search_pending == TRUE)
            {
                APP_INFO0("Searched service not found");
                service_search_pending = FALSE;
            }
            break;

        case BSA_BLE_CL_READ_EVT:
            APP_INFO1("BSA_BLE_CL_READ_EVT: conn_id:%d len:%d",
                    p_data->cli_read.conn_id, p_data->cli_read.len);
            client_num = app_ble_client_find_index_by_conn_id(p_data->cli_read.conn_id);
            app_ble_cb.ble_client[client_num].read_pending = FALSE;
            if(p_data->cli_read.status == BSA_SUCCESS)
            {
                APP_INFO1("srvc_id:%s (uuid:0x%04x)",
                        app_ble_display_service_name(p_data->cli_read.srvc_id.id.uuid.uu.uuid16),
                        p_data->cli_read.srvc_id.id.uuid.uu.uuid16);

                app_ble_client_handle_read_evt(&p_data->cli_read);
            }
            else
            {
                APP_ERROR1("BSA_BLE_CL_READ_EVT: status:%d", p_data->cli_read.status);
            }
            break;

        case BSA_BLE_CL_CLOSE_EVT:
            APP_INFO1("BSA_BLE_CL_CLOSE_EVT, BDA:%02X:%02X:%02X:%02X:%02X:%02X  ",
                              p_data->cli_close.remote_bda[0], p_data->cli_close.remote_bda[1],
                              p_data->cli_close.remote_bda[2], p_data->cli_close.remote_bda[3],
                              p_data->cli_close.remote_bda[4], p_data->cli_close.remote_bda[5]);
            client_num = app_ble_client_find_index_by_interface(p_data->cli_open.client_if);
            if(client_num < 0)
            {
                APP_ERROR0("No client!!");
                break;
            }
            app_ble_cb.ble_client[client_num].conn_id = BSA_BLE_INVALID_CONN;
            break;


        case BSA_BLE_CL_WRITE_EVT:
            APP_INFO1("BSA_BLE_CL_WRITE_EVT, status:%d",
                      p_data->cli_write.status);
            client_num = app_ble_client_find_index_by_conn_id(p_data->cli_write.conn_id);
            if (client_num >= 0)
            {
                app_ble_cb.ble_client[client_num].write_pending = FALSE;
            }
            break;

        case BSA_BLE_CL_CONGEST_EVT:
            APP_INFO1("BSA_BLE_CL_CONGEST_EVT  :conn_id:0x%x, congested:%d",
                        p_data->cli_congest.conn_id, p_data->cli_congest.congested);
            client_num = app_ble_client_find_index_by_conn_id(p_data->cli_congest.conn_id);
            if (client_num >= 0)
            {
                app_ble_cb.ble_client[client_num].congested = p_data->cli_congest.congested;
            }
            break;

        case BSA_BLE_CL_NOTIF_EVT:
            APP_INFO1("BSA_BLE_CL_NOTIF_EVT BDA :%02X:%02X:%02X:%02X:%02X:%02X",
                        p_data->cli_notif.bda[0], p_data->cli_notif.bda[1],
                        p_data->cli_notif.bda[2], p_data->cli_notif.bda[3],
                        p_data->cli_notif.bda[4], p_data->cli_notif.bda[5]);
            APP_INFO1("conn_id:0x%x, svrc_id:0x%04x, char_id:0x%04x, descr_type:0x%04x, len:%d, is_notify:%d",
                        p_data->cli_notif.conn_id,
                        p_data->cli_notif.char_id.srvc_id.id.uuid.uu.uuid16,
                        p_data->cli_notif.char_id.char_id.uuid.uu.uuid16,
                        p_data->cli_notif.descr_type.uuid.uu.uuid16, p_data->cli_notif.len,
                        p_data->cli_notif.is_notify);
            APP_DUMP("data", p_data->cli_notif.value, p_data->cli_notif.len);
            if(p_data->cli_notif.is_notify != TRUE)
            {
                /* this is indication, and need to send confirm */
                APP_INFO0("Receive Indication! send Indication Confirmation!");
                status = BSA_BleClIndConfInit(&ind_conf);
                if (status < 0)
                {
                    APP_ERROR1("BSA_BleClIndConfInit failed: %d", status);
                    break;
                }
                ind_conf.conn_id = p_data->cli_notif.conn_id;
                memcpy(&(ind_conf.char_id), &(p_data->cli_notif.char_id), sizeof(tBTA_GATTC_CHAR_ID));
                status = BSA_BleClIndConf(&ind_conf);
                if (status < 0)
                {
                    APP_ERROR1("BSA_BleClIndConf failed: %d", status);
                    break;
                }
                app_ble_client_handle_indication(&p_data->cli_notif);
            }
            else  /* notification */
            {
                app_ble_client_handle_notification(&p_data->cli_notif);
            }
            break;

        case BSA_BLE_CL_CACHE_SAVE_EVT:
            APP_INFO0("BSA_BLE_CL_CACHE_SAVE_EVT");
            APP_DEBUG1("evt:0x%x, attr:0x%x, num_attr:%d, conn_id:0x%x",
                      p_data->cli_cache_save.evt ,p_data->cli_cache_save.attr,
                      p_data->cli_cache_save.num_attr ,p_data->cli_cache_save.conn_id);
            APP_DEBUG1("BDA:%02X:%02X:%02X:%02X:%02X:%02X  ",
                      p_data->cli_cache_save.bd_addr[0], p_data->cli_cache_save.bd_addr[1],
                      p_data->cli_cache_save.bd_addr[2], p_data->cli_cache_save.bd_addr[3],
                      p_data->cli_cache_save.bd_addr[4], p_data->cli_cache_save.bd_addr[5]);
            for(index=0; index < p_data->cli_cache_save.num_attr;index++)
            {
                APP_DEBUG1("\t Attr[0x%04x] handle[%d] uuid[0x%04x] inst[%d] type[%d] prop[0x%1x] is_primary[%d]",
                              index, p_data->cli_cache_save.attr[index].s_handle,
                              p_data->cli_cache_save.attr[index].uuid.uu.uuid16,
                              p_data->cli_cache_save.attr[index].id,
                              p_data->cli_cache_save.attr[index].attr_type,
                              p_data->cli_cache_save.attr[index].prop,
                              p_data->cli_cache_save.attr[index].is_primary);
            }
            /* save BLE DB */
            /* step 1. find element by bsa */
            /* step 2. add attribute */
            p_blecl_db_elmt = app_ble_client_db_find_by_bda(p_data->cli_cache_save.bd_addr);
            if (p_blecl_db_elmt == NULL)
            {
                APP_DEBUG0("No element!!");
                APP_DEBUG0("New device. Update BLE DB!!");
                /* update BLE DB */
                p_blecl_db_elmt = app_ble_client_db_alloc_element();
                if(p_blecl_db_elmt != NULL)
                {
                    bdcpy(p_blecl_db_elmt->bd_addr, p_data->cli_cache_save.bd_addr);
                    app_ble_client_db_add_element(p_blecl_db_elmt);
                    APP_DEBUG1("save BDA:%02X:%02X:%02X:%02X:%02X:%02X, client_if= %d ",
                          p_blecl_db_elmt->bd_addr[0], p_blecl_db_elmt->bd_addr[1],
                          p_blecl_db_elmt->bd_addr[2], p_blecl_db_elmt->bd_addr[3],
                          p_blecl_db_elmt->bd_addr[4], p_blecl_db_elmt->bd_addr[5],
                          p_data->cli_open.client_if);
                    app_ble_client_db_save();
                }
            }
            if(p_blecl_db_elmt != NULL)
            {
                for(index=0; index < p_data->cli_cache_save.num_attr;index++)
                {
                    if(!app_ble_client_db_find_by_handle(p_blecl_db_elmt, p_data->cli_cache_save.attr[index].s_handle))
                    {
                        p_blecl_db_attr = app_ble_client_db_alloc_attr(p_blecl_db_elmt);
                        p_blecl_db_attr->attr_type = p_data->cli_cache_save.attr[index].attr_type;
                        p_blecl_db_attr->attr_UUID = p_data->cli_cache_save.attr[index].uuid;
                        p_blecl_db_attr->handle = p_data->cli_cache_save.attr[index].s_handle;
                        p_blecl_db_attr->id = p_data->cli_cache_save.attr[index].id;
                        p_blecl_db_attr->prop = p_data->cli_cache_save.attr[index].prop;
                        p_blecl_db_attr->is_primary = p_data->cli_cache_save.attr[index].is_primary;
                    }
                }
                /* step 3. save attribute */
                app_ble_client_db_save();
            }
            break;

        case BSA_BLE_CL_CACHE_LOAD_EVT:
            APP_INFO0("BSA_BLE_CL_CACHE_LOAD_EVT");
            APP_DEBUG1("BDA:%02X:%02X:%02X:%02X:%02X:%02X  id:0x%x",
                      p_data->cli_cache_load.bd_addr[0], p_data->cli_cache_load.bd_addr[1],
                      p_data->cli_cache_load.bd_addr[2], p_data->cli_cache_load.bd_addr[3],
                      p_data->cli_cache_load.bd_addr[4], p_data->cli_cache_load.bd_addr[5],
                      p_data->cli_cache_load.conn_id);
            /* search BLE DB */
            p_blecl_db_elmt = app_ble_client_db_find_by_bda(p_data->cli_cache_load.bd_addr);
            if(p_blecl_db_elmt != NULL)
            {
                APP_DEBUG1("YES! We have information of BDA:%02X:%02X:%02X:%02X:%02X:%02X  ",
                      p_data->cli_cache_load.bd_addr[0], p_data->cli_cache_load.bd_addr[1],
                      p_data->cli_cache_load.bd_addr[2], p_data->cli_cache_load.bd_addr[3],
                      p_data->cli_cache_load.bd_addr[4], p_data->cli_cache_load.bd_addr[5]);
                app_ble_client_load_attr(p_blecl_db_elmt, p_data->cli_cache_load.bd_addr, p_data->cli_cache_load.conn_id);
            }
            else
            {
                APP_DEBUG1("No information of BDA:%02X:%02X:%02X:%02X:%02X:%02X  ",
                      p_data->cli_cache_load.bd_addr[0], p_data->cli_cache_load.bd_addr[1],
                      p_data->cli_cache_load.bd_addr[2], p_data->cli_cache_load.bd_addr[3],
                      p_data->cli_cache_load.bd_addr[4], p_data->cli_cache_load.bd_addr[5]);
                app_ble_client_load_attr(p_blecl_db_elmt, p_data->cli_cache_load.bd_addr, p_data->cli_cache_load.conn_id);
            }
            break;

        default:
            break;
    }

}

/*******************************************************************************
**
** Function         app_ble_client_handle_indication
**
** Description      Function to handle callback for indications
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_indication(tBSA_BLE_CL_NOTIF_MSG *cli_notif)
{
    UINT8 flag = 0;
    UINT8 *pp;
    BOOLEAN blp_bp_measurement_mmHg= FALSE;
    BOOLEAN blp_bp_measurement_kpa= FALSE;
    BOOLEAN blp_bp_measurement_timestamp= FALSE;
    BOOLEAN blp_bp_measurement_pulserate= FALSE;
    BOOLEAN blp_bp_measurement_userid= FALSE;
    BOOLEAN blp_bp_measurement_status= FALSE;
    UINT16 blp_comp_val_syst = 0;
    UINT16 blp_comp_val_Dias = 0;
    UINT16 blp_mean_arte_pressure = 0;
    UINT8 blp_measurement_timestamp = 0;
    UINT16 blp_measurement_pulserate = 0;
    UINT8 blp_measurement_user_id = 0;
    UINT16 blp_measurement_status = 0;
    BOOLEAN htp_tmp_measurement_fah = FALSE;
    BOOLEAN htp_tmp_measurement_cels = FALSE;
    BOOLEAN htp_tmp_measurement_timestamp = FALSE;
    BOOLEAN htp_tmp_type_flag = FALSE;
    UINT32 tmp_measurement_value_cels = 0;
    UINT32 tmp_measurement_value_fah = 0;
    UINT16 year = 0;
    UINT8 month = 0;
    UINT8 day = 0;
    UINT8 hours = 0;
    UINT8 minutes = 0;
    UINT8 seconds = 0;
    UINT8 value = 0;
    UINT8 float_exponent_val = 0;
    float f1oat_temp_conversion = 0;

    switch(cli_notif->char_id.char_id.uuid.uu.uuid16)
    {
    case BSA_BLE_GATT_UUID_BLOOD_PRESSURE_MEASUREMENT:
        APP_INFO1("BSA_BLE_GATT_UUID_BLOOD_PRESSURE_MEASUREMENT, cli_notif->len %d", cli_notif->len );
        if(cli_notif->len <= BSA_BLE_GATT_LENGTH_OF_BLOOD_PRESSURE_MEASUREMENT)
        {
            pp = cli_notif->value;
            STREAM_TO_UINT8(flag, pp);
            APP_INFO1("Flags:0x%x", flag);
            if(flag & 0x1)
            {
                blp_bp_measurement_kpa = TRUE;
                APP_INFO0("Blood pressure for Systolic, Diastolic and MAP in units of kPa");
            }
            else
            {
                blp_bp_measurement_mmHg = TRUE;
                APP_INFO0("Blood pressure for Systolic, Diastolic and MAP in units of mmHg");
            }
            if((flag >> 1) & 0x1)
            {
                blp_bp_measurement_timestamp = TRUE;
                APP_INFO0("Time Stamp present");
            }
            else
            {
                APP_INFO0("Time Stamp not present");
            }
            if((flag >> 2) & 0x1)
            {
                blp_bp_measurement_pulserate = TRUE;
                APP_INFO0("Pulse Rate present");
            }
            else
            {
                APP_INFO0("Pulse Rate not present");
            }
            if((flag >> 3) & 0x1)
            {
                blp_bp_measurement_userid = TRUE;
                APP_INFO0("User ID present");
            }
            else
            {
                APP_INFO0("User ID not present");
            }
            if((flag >> 4) & 0x1)
            {
                blp_bp_measurement_status = TRUE;
                APP_INFO0("Measurement Status present");
            }
            else
            {
                APP_INFO0("Measurement Status not present");
            }

            if(blp_bp_measurement_mmHg)
            {
                STREAM_TO_UINT16(blp_comp_val_syst, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Systolic (mmHg):%d", blp_comp_val_syst);
                STREAM_TO_UINT16(blp_comp_val_Dias, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Diastolic (mmHg):%d", blp_comp_val_Dias);
                STREAM_TO_UINT16(blp_mean_arte_pressure, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Mean Arterial Pressure (mmHg):%d", blp_mean_arte_pressure);
            }

            if(blp_bp_measurement_kpa)
            {
                STREAM_TO_UINT16(blp_comp_val_syst, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Systolic (kPa):%d", blp_comp_val_syst);
                STREAM_TO_UINT16(blp_comp_val_Dias, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Diastolic (kPa):%d", blp_comp_val_Dias);
                STREAM_TO_UINT16(blp_mean_arte_pressure, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Mean Arterial Pressure (kPa):%d", blp_mean_arte_pressure);
            }

            if(blp_bp_measurement_timestamp)
            {
                STREAM_TO_UINT8(blp_measurement_timestamp, pp);
                APP_INFO1("Time Stamp:%d",blp_measurement_timestamp);
            }

            if(blp_bp_measurement_pulserate)
            {
                STREAM_TO_UINT16(blp_measurement_pulserate, pp);
                APP_INFO1("Pulse Rate:%d",blp_measurement_pulserate);
            }

            if(blp_bp_measurement_userid)
            {
                STREAM_TO_UINT8(blp_measurement_user_id, pp);
                APP_INFO1("User Id:%d",blp_measurement_user_id);
            }

            if(blp_bp_measurement_status)
            {
                STREAM_TO_UINT16(blp_measurement_status, pp);
                APP_INFO1("Measurement Status:%d",blp_measurement_status);
            }
        }
        else
        {
            APP_ERROR1("Wrong length:%d", cli_notif->len);
        }
        break;

    case BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT:
        APP_INFO1("BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT, cli_notif->len %d", cli_notif->len );
        pp = cli_notif->value;
        STREAM_TO_UINT8(flag, pp);
        APP_INFO1("Flags:0x%x", flag);
        if(flag & 0x1)
        {
            htp_tmp_measurement_fah = TRUE;
            APP_INFO0("Temperature Measurement Value in units of Fahrenheit");
        }
        else
        {
            htp_tmp_measurement_cels = TRUE;
            APP_INFO0("Temperature Measurement Value in units of Celsius");
        }

        if((flag >> 1) & 0x1)
        {
            htp_tmp_measurement_timestamp = TRUE;
            APP_INFO0("Time Stamp field present");
        }
        else
        {
            APP_INFO0("Time Stamp field not present");
        }

        if((flag >> 2) & 0x1)
        {
            htp_tmp_type_flag = TRUE;
            APP_INFO0("Temperature Type field present");
        }
        else
        {
            APP_INFO0("Temperature Type field not present");
        }

        if(htp_tmp_measurement_cels)
        {
            STREAM_TO_UINT24(tmp_measurement_value_cels, pp);
            STREAM_TO_UINT8(float_exponent_val,pp);

            if (float_exponent_val==0xFF)
            {
                 f1oat_temp_conversion = (float)tmp_measurement_value_cels/10.0;
                 APP_INFO1("Temperature Measurement Value (Celsius):%.1f", f1oat_temp_conversion);
            }
            else
            {
                 APP_INFO1("Temperature Measurement Value (Celsius):%d", tmp_measurement_value_cels);
            }
        }

        if(htp_tmp_measurement_fah)
        {
            STREAM_TO_UINT24(tmp_measurement_value_fah, pp);
            STREAM_TO_UINT8(float_exponent_val,pp);

            if (float_exponent_val==0xFF)
            {
                 f1oat_temp_conversion = (float)tmp_measurement_value_fah/10.0;
                 APP_INFO1("Temperature Measurement Value (Fahrenheit):%.1f", f1oat_temp_conversion);
            }
            else
            {
                APP_INFO1("Temperature Measurement Value (Fahrenheit):%d", tmp_measurement_value_fah);
            }
        }

        if(htp_tmp_measurement_timestamp)
        {
            STREAM_TO_UINT16(year, pp);
            STREAM_TO_UINT8(month, pp);
            STREAM_TO_UINT8(day, pp);
            STREAM_TO_UINT8(hours, pp);
            STREAM_TO_UINT8(minutes, pp);
            STREAM_TO_UINT8(seconds, pp);
            APP_INFO1("Temperature Measurement Time stamp Date: Year-%d, Month-%d, Day-%d", year, month, day);
            APP_INFO1("Time: Hours-%d, Minutes-%d, Seconds-%d", hours, minutes, seconds);
        }
        if(htp_tmp_type_flag)
        {
            STREAM_TO_UINT8(value, pp);
            if(value == 1)
                APP_INFO0("Location : Armpit");
            else if(value == 2)
                APP_INFO0("Location : Body (general)");
            else if(value == 3)
                APP_INFO0("Location : Ear (usually ear lobe)");
            else if(value == 4)
                APP_INFO0("Location : Finger");
            else if(value == 5)
                APP_INFO0("Location : Gastro-intestinal Tract");
            else if(value == 6)
                APP_INFO0("Location : Mouth");
            else if(value == 7)
                APP_INFO0("Location : Rectum");
            else if(value == 8)
                APP_INFO0("Location : Toe");
            else if(value == 9)
                APP_INFO0("Location : Tympanum (ear drum)");
            else
                APP_INFO0("Location : Reserved");
        }
            break;

        default:
                break;
    }
}

/*******************************************************************************
**
** Function         app_ble_client_handle_notification
**
** Description      Function to handle callback for notification events
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_notification(tBSA_BLE_CL_NOTIF_MSG *cli_notif)
{
    UINT8 flag = 0;
    UINT8 *pp;
    UINT32 cwr = 0;
    UINT16 lwet = 0;
    UINT16 ccr = 0;
    UINT16 lcet = 0;
    BOOLEAN blp_bp_measurement_mmHg= FALSE;
    BOOLEAN blp_bp_measurement_kpa= FALSE;
    BOOLEAN blp_bp_measurement_timestamp= FALSE;
    BOOLEAN blp_bp_measurement_pulserate= FALSE;
    BOOLEAN blp_bp_measurement_userid= FALSE;
    BOOLEAN blp_bp_measurement_status= FALSE;
    UINT16 blp_comp_val_syst = 0;
    UINT16 blp_comp_val_Dias = 0;
    UINT16 blp_mean_arte_pressure = 0;
    UINT8 blp_measurement_timestamp = 0;
    UINT16 blp_measurement_pulserate = 0;
    UINT8 blp_measurement_user_id = 0;
    UINT16 blp_measurement_status = 0;
    UINT16 rscs_is = 0;
    UINT8 rscs_ic = 0;
    UINT16 rscs_isl = 0;
    UINT32 rscs_td = 0;
    BOOLEAN htp_tmp_measurement_fah = FALSE;
    BOOLEAN htp_tmp_measurement_cels = FALSE;
    BOOLEAN htp_tmp_measurement_timestamp = FALSE;
    BOOLEAN htp_tmp_type_flag = FALSE;
    UINT32 tmp_measurement_value_cels = 0;
    UINT32 tmp_measurement_value_fah = 0;
    UINT16 year = 0;
    UINT8 month = 0;
    UINT8 day = 0;
    UINT8 hours = 0;
    UINT8 minutes = 0;
    UINT8 seconds = 0;
    UINT8 value = 0;
    UINT8 float_exponent_val = 0;
    float f1oat_temp_conversion = 0;



    switch(cli_notif->char_id.char_id.uuid.uu.uuid16)
    {
    case BSA_BLE_GATT_UUID_CSC_MEASUREMENT:
        if(cli_notif->len == BSA_BLE_GATT_LENGTH_OF_CSC_MEASUREMENT)
        {
            pp = cli_notif->value;
            STREAM_TO_UINT8(flag, pp);
            APP_INFO1("Flags:0x%x", flag);
            STREAM_TO_UINT32(cwr, pp);
            APP_INFO1("Cumulative Wheel Revolution:%d", cwr);
            STREAM_TO_UINT16(lwet, pp);
            APP_INFO1("Last Wheel Event Time:%d", lwet);
            STREAM_TO_UINT16(ccr, pp);
            APP_INFO1("Cumulative Crank Revolution:%d", ccr);
            STREAM_TO_UINT16(lcet, pp);
            APP_INFO1("Last Crank Event Time: %d", lcet);
        }
        else
        {
            APP_ERROR1("Wrong length:%d", cli_notif->len);
        }
        break;

    case BSA_BLE_GATT_UUID_INTERMEDIATE_CUFF_PRESSURE:
        APP_INFO1("BSA_BLE_GATT_UUID_INTERMEDIATE_CUFF_PRESSURE, cli_notif->len %d", cli_notif->len );
        if(cli_notif->len <= BSA_BLE_GATT_LENGTH_OF_BLOOD_PRESSURE_MEASUREMENT)
        {
            pp = cli_notif->value;
            STREAM_TO_UINT8(flag, pp);
            APP_INFO1("Flags:0x%x", flag);
            if(flag & 0x1)
            {
                blp_bp_measurement_kpa = TRUE;
                APP_INFO0("Blood pressure for Systolic, Diastolic and MAP in units of kPa");
            }
            else
            {
                blp_bp_measurement_mmHg = TRUE;
                APP_INFO0("Blood pressure for Systolic, Diastolic and MAP in units of mmHg");
            }
            if((flag >> 1) & 0x1)
            {
                blp_bp_measurement_timestamp = TRUE;
                APP_INFO0("Time Stamp present");
            }
            else
            {
                APP_INFO0("Time Stamp not present");
            }
            if((flag >> 2) & 0x1)
            {
                blp_bp_measurement_pulserate = TRUE;
                APP_INFO0("Pulse Rate present");
            }
            else
            {
                APP_INFO0("Pulse Rate not present");
            }
            if((flag >> 3) & 0x1)
            {
                blp_bp_measurement_userid = TRUE;
                APP_INFO0("User ID present");
            }
            else
            {
                APP_INFO0("User ID not present");
            }
            if((flag >> 4) & 0x1)
            {
                blp_bp_measurement_status = TRUE;
                APP_INFO0("Measurement Status present");
            }
            else
            {
                APP_INFO0("Measurement Status not present");
            }

            if(blp_bp_measurement_mmHg)
            {
                STREAM_TO_UINT16(blp_comp_val_syst, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Systolic (mmHg):%d", blp_comp_val_syst);
                STREAM_TO_UINT16(blp_comp_val_Dias, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Diastolic (mmHg):%d", blp_comp_val_Dias);
                STREAM_TO_UINT16(blp_mean_arte_pressure, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Mean Arterial Pressure (mmHg):%d", blp_mean_arte_pressure);
            }

            if(blp_bp_measurement_kpa)
            {
                STREAM_TO_UINT16(blp_comp_val_syst, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Systolic (kPa):%d", blp_comp_val_syst);
                STREAM_TO_UINT16(blp_comp_val_Dias, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Diastolic (kPa):%d", blp_comp_val_Dias);
                STREAM_TO_UINT16(blp_mean_arte_pressure, pp);
                APP_INFO1("Blood Pressure Measurement Compound Value - Mean Arterial Pressure (kPa):%d", blp_mean_arte_pressure);
            }

            if(blp_bp_measurement_timestamp)
            {
                STREAM_TO_UINT8(blp_measurement_timestamp, pp);
                APP_INFO1("Time Stamp:%d",blp_measurement_timestamp);
            }

            if(blp_bp_measurement_pulserate)
            {
                STREAM_TO_UINT16(blp_measurement_pulserate, pp);
                APP_INFO1("Pulse Rate:%d",blp_measurement_pulserate);
            }

            if(blp_bp_measurement_userid)
            {
                STREAM_TO_UINT8(blp_measurement_user_id, pp);
                APP_INFO1("User Id:%d",blp_measurement_user_id);
            }

            if(blp_bp_measurement_status)
            {
                STREAM_TO_UINT16(blp_measurement_status, pp);
                APP_INFO1("Measurement Status:%d",blp_measurement_status);
            }
        }
        else
        {
            APP_ERROR1("Wrong length:%d", cli_notif->len);
        }
    break;

    case BSA_BLE_GATT_UUID_RSC_MEASUREMENT:
        if(cli_notif->len == BSA_BLE_GATT_LENGTH_OF_RSC_MEASUREMENT)
        {
            pp = cli_notif->value;
            STREAM_TO_UINT8(flag, pp);
            APP_INFO1("Flags:0x%x", flag);
            STREAM_TO_UINT16(rscs_is, pp);
            APP_INFO1("Instantaneous Speed:%d", rscs_is);
            STREAM_TO_UINT8(rscs_ic, pp);
            APP_INFO1("Instantaneous Cadence:%d", rscs_ic);
            STREAM_TO_UINT16(rscs_isl, pp);
            APP_INFO1("Instantaneous Stride Length:%d", rscs_isl);
            STREAM_TO_UINT32(rscs_td, pp);
            APP_INFO1("Total Distance: %d", rscs_td);
        }
        else
        {
            APP_ERROR1("Wrong length:%d", cli_notif->len);
        }
        break;

     case BSA_BLE_GATT_UUID_INTERMEDIATE_TEMPERATURE:
        APP_INFO1("BSA_BLE_GATT_UUID_INTERMEDIATE_TEMPERATURE, cli_notif->len %d", cli_notif->len );
        pp = cli_notif->value;
        STREAM_TO_UINT8(flag, pp);
        APP_INFO1("Flags:0x%x", flag);
        if(flag & 0x1)
        {
            htp_tmp_measurement_fah = TRUE;
            APP_INFO0("Temperature Measurement Value in units of Fahrenheit");
        }
        else
        {
            htp_tmp_measurement_cels = TRUE;
            APP_INFO0("Temperature Measurement Value in units of Celsius");
        }

        if((flag >> 1) & 0x1)
        {
            htp_tmp_measurement_timestamp = TRUE;
            APP_INFO0("Time Stamp field present");
        }
        else
        {
            APP_INFO0("Time Stamp field not present");
        }

        if((flag >> 2) & 0x1)
        {
            htp_tmp_type_flag = TRUE;
            APP_INFO0("Temperature Type field present");
        }
        else
        {
            APP_INFO0("Temperature Type field not present");
        }

        if(htp_tmp_measurement_cels)
        {
            STREAM_TO_UINT24(tmp_measurement_value_cels, pp);
            STREAM_TO_UINT8(float_exponent_val,pp);
            if (float_exponent_val==0xFF)
            {
                 f1oat_temp_conversion = (float)tmp_measurement_value_cels/10.0;
                 APP_INFO1("Temperature Measurement Value (Celsius):%.1f", f1oat_temp_conversion);
            }
            else
            {
                 APP_INFO1("Temperature Measurement Value (Celsius):%d", tmp_measurement_value_cels);
            }
        }

        if(htp_tmp_measurement_fah)
        {
            STREAM_TO_UINT24(tmp_measurement_value_fah, pp);
            STREAM_TO_UINT8(float_exponent_val,pp);
            if (float_exponent_val==0xFF)
            {
                 f1oat_temp_conversion = (float)tmp_measurement_value_fah/10.0;
                 APP_INFO1("Temperature Measurement Value (Fahrenheit):%.1f", f1oat_temp_conversion);
            }
            else
            {
                APP_INFO1("Temperature Measurement Value (Fahrenheit):%d", tmp_measurement_value_fah);
            }
        }

        if(htp_tmp_measurement_timestamp)
        {
            STREAM_TO_UINT16(year, pp);
            STREAM_TO_UINT8(month, pp);
            STREAM_TO_UINT8(day, pp);
            STREAM_TO_UINT8(hours, pp);
            STREAM_TO_UINT8(minutes, pp);
            STREAM_TO_UINT8(seconds, pp);
            APP_INFO1("Temperature Measurement Time stamp Date: Year-%d, Month-%d, Day-%d", year, month, day);
            APP_INFO1("Time: Hours-%d, Minutes-%d, Seconds-%d", hours, minutes, seconds);
        }
        if(htp_tmp_type_flag)
        {
            STREAM_TO_UINT8(value, pp);
            if(value == 1)
                APP_INFO0("Location : Armpit");
            else if(value == 2)
                APP_INFO0("Location : Body (general)");
            else if(value == 3)
                APP_INFO0("Location : Ear (usually ear lobe)");
            else if(value == 4)
                APP_INFO0("Location : Finger");
            else if(value == 5)
                APP_INFO0("Location : Gastro-intestinal Tract");
            else if(value == 6)
                APP_INFO0("Location : Mouth");
            else if(value == 7)
                APP_INFO0("Location : Rectum");
            else if(value == 8)
                APP_INFO0("Location : Toe");
            else if(value == 9)
                APP_INFO0("Location : Tympanum (ear drum)");
            else
                APP_INFO0("Location : Reserved");
         }
        break;


    default:
        break;
    }
}

/*******************************************************************************
**
** Function         app_ble_client_handle_read_evt
**
** Description      Function to handle callback for read events
**
** Returns          void
**
*******************************************************************************/
void app_ble_client_handle_read_evt(tBSA_BLE_CL_READ_MSG *cli_read)
{
    UINT8 value = 0;
    UINT16 temperature_measurement_interval = 0;
    UINT8 *pp;
    UINT16 lower_range = 0,upper_range = 0;

    switch(cli_read->char_id.uuid.uu.uuid16)
    {
    case BSA_BLE_GATT_UUID_SENSOR_LOCATION:
        APP_INFO0("char_id:BSA_BLE_GATT_UUID_SENSOR_LOCATION");
        value = cli_read->value[0];
        if(value == 0)
            APP_INFO0("Location : Other");
        else if(value == 1)
            APP_INFO0("Location : Top of Shoe");
        else if(value == 2)
            APP_INFO0("Location : In Shoe");
        else if(value == 3)
            APP_INFO0("Location : Hip");
        else if(value == 4)
            APP_INFO0("Location : Front Wheel");
        else if(value == 5)
            APP_INFO0("Location : Left Crank");
        else if(value == 6)
            APP_INFO0("Location : Right Crank");
        else if(value == 7)
            APP_INFO0("Location : Left Pedal");
        else if(value == 8)
            APP_INFO0("Location : Right Pedal");
        else if(value == 9)
            APP_INFO0("Location : Front Hub");
        else if(value == 10)
            APP_INFO0("Location : Real Dropout");
        else if(value == 11)
            APP_INFO0("Location : Chainstay");
        else if(value == 12)
            APP_INFO0("Location : Real Wheel");
        else if(value == 13)
            APP_INFO0("Location : Real Hub");
        else if(value == 14)
            APP_INFO0("Location : Chest");
        else
            APP_INFO0("Location : Reserved");
        break;

    case BSA_BLE_GATT_UUID_CSC_FEATURE:
        APP_INFO0("char_id:BSA_BLE_GATT_UUID_CSC_FEATURE");
        value = cli_read->value[0];
        if(value & 1)
            APP_INFO0("Wheel Revolution Data Supported!");
        if(value & 2)
            APP_INFO0("Crank Revolution Data Supported!");
        if(value & 4)
            APP_INFO0("Multiple Sensor Location Supported!");
        break;

    case BSA_BLE_GATT_UUID_BLOOD_PRESSURE_FEATURE:
         APP_INFO0("char_id: BSA_BLE_GATT_UUID_BLOOD_PRESSURE_FEATURE");
        value = cli_read->value[0];
        if(value & 1)
            APP_INFO0("Body Movement Detection feature supported!");
        if(value & 2)
            APP_INFO0("Cuff Fit Detection feature supported!");
        if(value & 4)
            APP_INFO0("Irregular Pulse Detection feature supported!");
        if(value & 8)
            APP_INFO0("Pulse Rate Range Detection feature supported!");
        if(value & 16)
            APP_INFO0("Measurement Position Detection feature supported!");
        if(value & 32)
            APP_INFO0("Multiple Bonds supported!");
         break;

    case BSA_BLE_GATT_UUID_RSC_FEATURE:
        APP_INFO0("char_id:BSA_BLE_GATT_UUID_RSC_FEATURE");
        value = cli_read->value[0];
        if(value & 1)
            APP_INFO0("Instantaneous Stride Length Measurement Supported!");
        if(value & 2)
            APP_INFO0("Walking or Running Status Supported!");
        if(value & 4)
            APP_INFO0("Calibration Procedure Supported!");
        if(value & 8)
            APP_INFO0("Multiple Sensor Locations Supported!");
        break;

    case BSA_BLE_GATT_UUID_TEMPERATURE_TYPE:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_TEMPERATURE_TYPE");
        value = cli_read->value[0];
        if(value == 1)
            APP_INFO0("Location : Armpit");
        else if(value == 2)
            APP_INFO0("Location : Body (general)");
        else if(value == 3)
            APP_INFO0("Location : Ear (usually ear lobe)");
        else if(value == 4)
            APP_INFO0("Location : Finger");
        else if(value == 5)
            APP_INFO0("Location : Gastro-intestinal Tract");
        else if(value == 6)
            APP_INFO0("Location : Mouth");
        else if(value == 7)
            APP_INFO0("Location : Rectum");
        else if(value == 8)
            APP_INFO0("Location : Toe");
        else if(value == 9)
            APP_INFO0("Location : Tympanum (ear drum)");
        else
            APP_INFO0("Location : Reserved");
        break;

    case BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT_INTERVAL:
        if(cli_read->descr_type.uuid.uu.uuid16 == BSA_BLE_GATT_UUID_CHAR_VALID_RANGE)
        {
            APP_INFO0("char_id: BSA_BLE_GATT_UUID_CHAR_VALID_RANGE");
            pp = cli_read->value;
            STREAM_TO_UINT16(lower_range,pp);
            STREAM_TO_UINT16(upper_range,pp);
            APP_INFO1("Lower Range:%d, Upper Range:%d",lower_range,upper_range);
        }
        else
        {
            APP_INFO0("char_id: BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT_INTERVAL");
            pp = cli_read->value;
            STREAM_TO_UINT16(temperature_measurement_interval,pp);
            APP_INFO1("Measurement Interval Seconds:%d, Minutes:%d, Hours:%d",temperature_measurement_interval,(temperature_measurement_interval/60),(temperature_measurement_interval/(60*60)));
        }
        break;

    case BSA_BLE_GATT_UUID_BATTERY_LEVEL:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_BATTERY_LEVEL");
        value = cli_read->value[0];
        APP_INFO1("Battery level %d percentage left", value);
        break;

    case BSA_BLE_GATT_UUID_MANU_NAME:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_MANU_NAME");
        APP_INFO1("Manufacturer name : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_MODEL_NUMBER_STR :
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_MODEL_NUMBER_STR ");
        APP_INFO1("Model Number : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_SERIAL_NUMBER_STR :
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_SERIAL_NUMBER_STR ");
        APP_INFO1("Model Number : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_HW_VERSION_STR :
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_HW_VERSION_STR ");
        APP_INFO1("Model Number : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_FW_VERSION_STR:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_FW_VERSION_STR ");
        APP_INFO1("Model Number : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_SW_VERSION_STR:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_SW_VERSION_STR ");
        APP_INFO1("Model Number : %s", cli_read->value);
        break;

    case BSA_BLE_GATT_UUID_SYSTEM_ID:
        APP_INFO0("char_id: BSA_BLE_GATT_UUID_SYSTEM_ID ");
        APP_INFO1("Model Number : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
            cli_read->value[0], cli_read->value[1], cli_read->value[2], cli_read->value[3],
            cli_read->value[4], cli_read->value[5], cli_read->value[6], cli_read->value[7]);
        break;

    default:
        APP_DEBUG1("char_uuid:0x%04x", cli_read->char_id.uuid.uu.uuid16);
        APP_DEBUG1("descr_uuid:0x%04x", cli_read->descr_type.uuid.uu.uuid16);
        APP_DUMP("Read value", cli_read->value, cli_read->len);

        break;
    }
}


/*******************************************************************************
 **
 ** Function        app_ble_client_read_battery_level
 **
 ** Description     Read BLE Device battery level
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_battery_level(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_battery_level : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_BATTERY_SERVICE;
    char_id = BSA_BLE_GATT_UUID_BATTERY_LEVEL;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_read_mfr_name
 **
 ** Description     Read BLE Device manufacturer name
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_mfr_name(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_mfr_name : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_MANU_NAME;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_read_model_number
 **
 ** Description     Read BLE Device model number
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_model_number(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_model_number : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_MODEL_NUMBER_STR;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_read_serial_number
 **
 ** Description     Read BLE Device serial number
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_serial_number(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_serial_number : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_SERIAL_NUMBER_STR;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_read_hardware_revision
 **
 ** Description     Read BLE Device hardware revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_hardware_revision(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_hardware_revision : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_HW_VERSION_STR;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_read_firmware_revision
 **
 ** Description     Read BLE Device firmware revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_firmware_revision(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_firmware_revision : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_FW_VERSION_STR;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_client_read_software_revision
 **
 ** Description     Read BLE Device software revision
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_software_revision(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_software_revision : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_SW_VERSION_STR;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_client_read_system_id
 **
 ** Description     Read BLE Device system ID
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_client_read_system_id(void)
{
    tBSA_STATUS status;
    tBSA_BLE_CL_READ ble_read_param;
    UINT16 service, char_id;
    int client_num;

    APP_INFO0("Select Client:");
    app_ble_client_display(0);
    client_num = app_get_choice("Select");

    if(client_num < 0)
    {
        APP_ERROR0("app_ble_client_read_system_id : Invalid client number entered");
        return -1;
    }

    service = BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION;
    char_id = BSA_BLE_GATT_UUID_SYSTEM_ID;

    status = BSA_BleClReadInit(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
    }

    ble_read_param.descr = FALSE;
    ble_read_param.char_id.srvc_id.id.inst_id= 0x00;
    ble_read_param.char_id.srvc_id.id.uuid.uu.uuid16 = service;
    ble_read_param.char_id.srvc_id.id.uuid.len = 2;
    ble_read_param.char_id.srvc_id.is_primary = 1;

    ble_read_param.char_id.char_id.inst_id = 0x00;
    ble_read_param.char_id.char_id.uuid.uu.uuid16 = char_id;
    ble_read_param.char_id.char_id.uuid.len = 2;
    ble_read_param.conn_id = app_ble_cb.ble_client[client_num].conn_id;
    ble_read_param.auth_req = 0x00;

    status = BSA_BleClRead(&ble_read_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("app_ble_client_read failed status = %d", status);
        return -1;
    }
    return 0;
}
