/*****************************************************************************
**
**  Name:           app_ble_server.c
**
**  Description:    Bluetooth BLE Server general application
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

#include "app_ble_server.h"

/*
 * Global Variables
 */


/*
 * Local functions
 */
static void (*s_pCallback)(tBSA_BLE_EVT,  tBSA_BLE_MSG *);

/*
 * BLE common functions
 */


/*******************************************************************************
 **
 ** Function         app_ble_server_find_free_server
 **
 ** Description      find free server for BLE server application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_free_server(void)
{
    int index;

    for (index = 0; index < BSA_BLE_SERVER_MAX; index++)
    {
        if (!app_ble_cb.ble_server[index].enabled)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_server_find_free_attr
 **
 ** Description      find free attr for BLE server application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_free_attr(UINT16 server)
{
    int index;

    for (index = 0; index < BSA_BLE_ATTRIBUTE_MAX; index++)
    {
        if (!app_ble_cb.ble_server[server].attr[index].attr_UUID.uu.uuid16)
        {
            return index;
        }
    }
    return -1;
}

//uuid 128
int app_ble_server_find_free_attr_uuid128(UINT16 server)
{
    int index;

    for (index = 0; index < BSA_BLE_ATTRIBUTE_MAX; index++)
    {
        if (!app_ble_cb.ble_server[server].attr[index].attr_UUID.len)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_server_display
 **
 ** Description      display BLE server
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_server_display(void)
{
    int index, attr_num;

    APP_INFO0("*************** BLE SERVER LIST *****************");
    for (index = 0; index < BSA_BLE_SERVER_MAX; index++)
    {
        if (app_ble_cb.ble_server[index].enabled)
        {
            APP_INFO1("%d:BLE Server server_if:%d", index,
                       app_ble_cb.ble_server[index].server_if);
            for (attr_num = 0; attr_num < BSA_BLE_ATTRIBUTE_MAX ; attr_num++)
            {
                if (app_ble_cb.ble_server[index].attr[attr_num].attr_UUID.uu.uuid16)
                {
                    if ((app_ble_cb.ble_server[index].attr[attr_num].attr_type == BSA_GATTC_ATTR_TYPE_SRVC) ||
                       (app_ble_cb.ble_server[index].attr[attr_num].attr_type == BSA_GATTC_ATTR_TYPE_INCL_SRVC))
                    {
                        APP_INFO1("\t attr_num:%d:uuid:0x%04x, is_pri:%d, service_id:%d attr_id:%d",
                            attr_num,
                            app_ble_cb.ble_server[index].attr[attr_num].attr_UUID.uu.uuid16,
                            app_ble_cb.ble_server[index].attr[attr_num].is_pri,
                            app_ble_cb.ble_server[index].attr[attr_num].service_id,
                            app_ble_cb.ble_server[index].attr[attr_num].attr_id);
                    }
                    else
                    {
                        APP_INFO1("\t\t attr_num:%d:uuid:0x%04x, is_pri:%d, service_id:%d attr_id:%d",
                            attr_num,
                            app_ble_cb.ble_server[index].attr[attr_num].attr_UUID.uu.uuid16,
                            app_ble_cb.ble_server[index].attr[attr_num].is_pri,
                            app_ble_cb.ble_server[index].attr[attr_num].service_id,
                            app_ble_cb.ble_server[index].attr[attr_num].attr_id);
                    }
                }
            }
        }
    }
    APP_INFO0("*************** BLE SERVER LIST END *************");
}

/*******************************************************************************
 **
 ** Function         app_ble_server_find_reg_pending_index
 **
 ** Description      find registration pending index
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_reg_pending_index(void)
{
    int index;

    for (index = 0; index < BSA_BLE_SERVER_MAX; index++)
    {
        if ((app_ble_cb.ble_server[index].enabled) &&
         (app_ble_cb.ble_server[index].server_if == BSA_BLE_INVALID_IF))
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_server_find_index_by_interface
 **
 ** Description      find BLE server index by interface
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_index_by_interface(tBSA_BLE_IF if_num)
{
    int index;

    for (index = 0; index < BSA_BLE_SERVER_MAX; index++)
    {
        if (app_ble_cb.ble_server[index].server_if == if_num)
        {
            return index;
        }
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_ble_server_find_index_by_conn_id
 **
 ** Description      find BLE server index by connection ID
 **
 ** Parameters       conn_id: Connection ID
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_index_by_conn_id(UINT16 conn_id)
{
    int index;

    for(index = 0; index < BSA_BLE_CLIENT_MAX ; index++)
    {
        if(app_ble_cb.ble_server[index].conn_id == conn_id)
        {
            return index;
        }
    }
    return -1;
}

int app_ble_server_find_servc_num_by_handle(int server_num, UINT16 handle)
{
    int char_attr_num = -1;
    int servc_attr_num = -1;
    unsigned short service_id = 0;

    for (char_attr_num = 0; char_attr_num < BSA_BLE_ATTRIBUTE_MAX; char_attr_num++)
    {
        if (app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_id == handle)
        {
            service_id = app_ble_cb.ble_server[server_num].attr[char_attr_num].service_id;
            break;
        }
    }

    for (servc_attr_num = 0; servc_attr_num < BSA_BLE_ATTRIBUTE_MAX; servc_attr_num++)
    {
	  if (((app_ble_cb.ble_server[server_num].attr[servc_attr_num].attr_type == BSA_GATTC_ATTR_TYPE_SRVC) ||
	      (app_ble_cb.ble_server[server_num].attr[servc_attr_num].attr_type == BSA_GATTC_ATTR_TYPE_INCL_SRVC)) &&
	      (app_ble_cb.ble_server[server_num].attr[servc_attr_num].service_id == service_id))
	  {
		  break;
	  }
    }

    return servc_attr_num;
}

int app_ble_server_find_char_attr_num_by_handle(int server_num, UINT16 handle)
{
    int char_attr_num = -1;
    unsigned short service_id = 0;

#if 0
    APP_INFO1("app_ble_server_find_char_attr_num_by_handle server num %d, handle %d", server_num, handle);
    app_ble_server_display();
#endif

    for (char_attr_num = 0; char_attr_num < BSA_BLE_ATTRIBUTE_MAX; char_attr_num++)
    {
        if (app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_id == handle)
        {
            service_id = app_ble_cb.ble_server[server_num].attr[char_attr_num].service_id;
            break;
        }
    }

    return char_attr_num;
}

/******************************************************************************
*
******************************************************************************/
int app_ble_server_find_char_uuid_by_handle(int server_num, UINT16 handle, UINT8 attr_uuid128[])
{
    int i = 0, attr_num = -1;

    for (attr_num = 0; attr_num < BSA_BLE_ATTRIBUTE_MAX; attr_num++)
    {
        if (app_ble_cb.ble_server[server_num].attr[attr_num].attr_id == handle)
        {
            break;
        }
    }

    if (attr_num != BSA_BLE_ATTRIBUTE_MAX)
    {
	  for (i = 0; i < LEN_UUID_128; i++) {
	      attr_uuid128[i] = app_ble_cb.ble_server[server_num].attr[attr_num].attr_UUID.uu.uuid128[i];
	  }
	  return 0;
    }
    else
    {
        for (i = 0; i < LEN_UUID_128; i++) {
            attr_uuid128[i] = 0;
        }
        return -1;
    }
}


/*
 * BLE Server functions
 */
/*******************************************************************************
 **
 ** Function        app_ble_server_register
 **
 ** Description     Register server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_register(UINT16 uuid, tBSA_BLE_CBACK *p_cback)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_REGISTER ble_register_param;
    UINT16 choice;
    int server_num;

    s_pCallback = NULL;
    server_num = app_ble_server_find_free_server();
    if (server_num < 0)
    {
        APP_ERROR0("No more spaces!!");
        return -1;
    }

    if (!uuid)
    {
        APP_INFO0("Bluetooth BLE register menu:");
        choice = app_get_choice("Register app UUID(eg. x9999)");
    }
    else
    {
        choice = uuid;
    }

    status = BSA_BleSeAppRegisterInit(&ble_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppRegisterInit failed status = %d", status);
        return -1;
    }

    ble_register_param.uuid.len = 2;
    ble_register_param.uuid.uu.uuid16 = choice;
    if (p_cback == NULL)
    {
        ble_register_param.p_cback = app_ble_server_profile_cback;
    }
    else
    {
        ble_register_param.p_cback = p_cback;
    }

    //BTM_BLE_SEC_NONE: No authentication and no encryption
    //BTM_BLE_SEC_ENCRYPT: encrypt the link with current key
    //BTM_BLE_SEC_ENCRYPT_NO_MITM: unauthenticated encryption
    //BTM_BLE_SEC_ENCRYPT_MITM: authenticated encryption
    ble_register_param.sec_act = BTM_BLE_SEC_NONE;

    status = BSA_BleSeAppRegister(&ble_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppRegister failed status = %d", status);
        return -1;
    }
    app_ble_cb.ble_server[server_num].enabled = TRUE;
    app_ble_cb.ble_server[server_num].server_if = ble_register_param.server_if;
    APP_INFO1("enabled:%d, server_if:%d", app_ble_cb.ble_server[server_num].enabled,
                    app_ble_cb.ble_server[server_num].server_if);
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_ble_server_deregister
 **
 ** Description     Deregister server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_deregister(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_DEREGISTER ble_deregister_param;
    int num;

    APP_INFO0("Bluetooth BLE deregister menu:");
    APP_INFO0("Select Server:");
    app_ble_server_display();
    num = app_get_choice("Select");
    if ((num < 0) || (num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", num);
        return -1;
    }
    if (app_ble_cb.ble_server[num].enabled != TRUE)
    {
        APP_ERROR1("Server was not registered! = %d", num);
        return -1;
    }

    status = BSA_BleSeAppDeregisterInit(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregisterInit failed status = %d", status);
        return -1;
    }

    ble_deregister_param.server_if = app_ble_cb.ble_server[num].server_if;

    status = BSA_BleSeAppDeregister(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregister failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_create_service
 **
 ** Description     create service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_create_service(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_CREATE ble_create_param;
    UINT16 service;
    UINT16  num_handle;
    BOOLEAN  is_primary;
    int server_num, attr_num;

    APP_INFO0("Select Server:");
    app_ble_server_display();
    server_num = app_get_choice("Select");
    if ((server_num < 0) || (server_num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    if (app_ble_cb.ble_server[server_num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", server_num);
        return -1;
    }

    service = app_get_choice("Enter Service UUID to create(eg. x180A)");
    if (!service)
    {
        APP_ERROR1("wrong value = %d", service);
        return -1;
    }

    APP_INFO0("Enter num of handle(x2) to create.");
    APP_INFO0("\tFor Example, if you will add 5 characteristics in this service");
    APP_INFO0("\tUse 12 (1 service x 2 + 5 characteristics x 2)");
    num_handle = app_get_choice("Enter num of handle");
    if (!num_handle)
    {
        APP_ERROR1("wrong value = %d", num_handle);
        return -1;
    }
    is_primary = app_get_choice("Select primary(1) or not(0)");
    if (!(is_primary == 0) && !(is_primary == 1))
    {
        APP_ERROR1("wrong value = %d", is_primary);
        return -1;
    }
    attr_num = app_ble_server_find_free_attr(server_num);
    if (attr_num < 0)
    {
        APP_ERROR1("Wrong attr number! = %d", attr_num);
        return -1;
    }
    status = BSA_BleSeCreateServiceInit(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateServiceInit failed status = %d", status);
        return -1;
    }

    ble_create_param.service_uuid.uu.uuid16 = service;
    ble_create_param.service_uuid.len = 2;
    ble_create_param.server_if = app_ble_cb.ble_server[server_num].server_if;
    ble_create_param.num_handle = num_handle;
    if (is_primary != 0)
    {
        ble_create_param.is_primary = TRUE;
    }
    else
    {
        ble_create_param.is_primary = FALSE;
    }

    app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag = TRUE;

    status = BSA_BleSeCreateService(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateService failed status = %d", status);
        app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag = FALSE;
        return -1;
    }

    /* store information on control block */
    app_ble_cb.ble_server[server_num].attr[attr_num].attr_UUID.len = 2;
    app_ble_cb.ble_server[server_num].attr[attr_num].attr_UUID.uu.uuid16 = service;
    app_ble_cb.ble_server[server_num].attr[attr_num].is_pri = ble_create_param.is_primary;
    app_ble_cb.ble_server[server_num].attr[attr_num].attr_type = BSA_GATTC_ATTR_TYPE_SRVC;

    while(app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag == TRUE)
		{
        GKI_delay(5);
		}

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_add_char
 **
 ** Description     Add character to service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_add_char(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_ADDCHAR ble_addchar_param;
    UINT16      char_uuid;
    int server_num, srvc_attr_num, char_attr_num;
    int attribute_permission;
    int characteristic_property = 0;
    int is_descript;

    APP_INFO0("Select Server:");
    app_ble_server_display();
    server_num = app_get_choice("Select");
    if ((server_num < 0) || (server_num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    if (app_ble_cb.ble_server[server_num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", server_num);
        return -1;
    }

    APP_INFO0("Select service's attribute number");
    srvc_attr_num = app_get_choice("Select");

    if(srvc_attr_num < 0)
    {
        APP_ERROR0("app_ble_server_add_char : Invalid srvc_attr_num entered");
        return -1;
    }

    char_attr_num = app_ble_server_find_free_attr(server_num);

    char_uuid = app_get_choice("Add char UUID(eg. x9999)");

    status = BSA_BleSeAddCharInit(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddCharInit failed status = %d", status);
        return -1;
    }
    ble_addchar_param.service_id = app_ble_cb.ble_server[server_num].attr[srvc_attr_num].service_id;
    ble_addchar_param.char_uuid.len = 2;
    ble_addchar_param.char_uuid.uu.uuid16 = char_uuid;

    is_descript = app_get_choice("Select descriptor? (yes=1 or no=0)");

    if(is_descript)
    {
        ble_addchar_param.is_descr = TRUE;

        APP_INFO0("Enter Attribute Permissions[Eg: Read-0x1, Write-0x10, Read|Write-0x11]");
        attribute_permission = app_get_choice("Enter");
        ble_addchar_param.perm = attribute_permission;
    }
    else
    {
        ble_addchar_param.is_descr = FALSE;

        APP_INFO0("Enter Attribute Permissions[Eg: Read-0x1, Write-0x10, Read|Write-0x11]");
        attribute_permission = app_get_choice("Enter");
        ble_addchar_param.perm = attribute_permission;

        APP_INFO0("Enter Characterisic Properties Eg: WRITE-0x08, READ-0x02, Notify-0x10, Indicate-0x20");
        APP_INFO0("Eg: For READ|WRITE|NOTIFY|INDICATE enter 0x3A");
        characteristic_property = app_get_choice("Enter");
        ble_addchar_param.property = characteristic_property;
    }

    app_ble_cb.ble_server[server_num].attr[char_attr_num].wait_flag = TRUE;
    status = BSA_BleSeAddChar(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddChar failed status = %d", status);
        return -1;
    }

    /* save all information */
    app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_UUID.len = 2;
    app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_UUID.uu.uuid16 = char_uuid;
    app_ble_cb.ble_server[server_num].attr[char_attr_num].prop = characteristic_property;
    app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_type = BSA_GATTC_ATTR_TYPE_CHAR;

    while(app_ble_cb.ble_server[server_num].attr[char_attr_num].wait_flag == TRUE)
		{
        GKI_delay(5);
		}

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_start_service
 **
 ** Description     Start Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_start_service(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_START ble_start_param;
    int num, attr_num;

    APP_INFO0("Select Server:");
    app_ble_server_display();
    num = app_get_choice("Select");
    if ((num < 0) || (num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", num);
        return -1;
    }
    if (app_ble_cb.ble_server[num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", num);
        return -1;
    }

    APP_INFO0("Select Service's attribute number :");
    attr_num = app_get_choice("Select");

    if(attr_num < 0)
    {
        APP_ERROR0("app_ble_server_start_service : Invalid attr_num entered");
        return -1;
    }

    status = BSA_BleSeStartServiceInit(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartServiceInit failed status = %d", status);
        return -1;
    }

    ble_start_param.service_id = app_ble_cb.ble_server[num].attr[attr_num].service_id;
    ble_start_param.sup_transport = BSA_BLE_GATT_TRANSPORT_LE_BR_EDR;

    APP_INFO1("service_id:%d, num:%d", ble_start_param.service_id, num);

    status = BSA_BleSeStartService(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartService failed status = %d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_send_indication
 **
 ** Description     Send indication to client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_send_indication(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_SENDIND ble_sendind_param;
    int num, length_of_data, index, attr_num;

    APP_INFO0("Select Server:");
    app_ble_server_display();
    num = app_get_choice("Select");
    if ((num < 0) || (num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", num);
        return -1;
    }
    if (app_ble_cb.ble_server[num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", num);
        return -1;
    }
    if(app_ble_cb.ble_server[num].congested)
    {
        APP_ERROR1("fail : congested(%d)!", app_ble_cb.ble_server[num].congested);
        return -1;
    }

    APP_INFO0("Select Service's attribute number :");
    if (-1 == (attr_num = app_get_choice("Select")))
        return -1;

    status = BSA_BleSeSendIndInit(&ble_sendind_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeSendIndInit failed status = %d", status);
        return -1;
    }

    ble_sendind_param.conn_id = app_ble_cb.ble_server[num].conn_id;
    ble_sendind_param.attr_id = app_ble_cb.ble_server[num].attr[attr_num].attr_id;

    length_of_data = app_get_choice("Enter length of data");
    ble_sendind_param.data_len = length_of_data;

    for (index = 0; index < length_of_data ; index++)
    {
        ble_sendind_param.value[index] = app_get_choice("Enter data in byte");
    }

    ble_sendind_param.need_confirm = FALSE;

    status = BSA_BleSeSendInd(&ble_sendind_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeSendInd failed status = %d", status);
        return -1;
    }

    return 0;
}

int app_ble_server_send_indication_ext(int server_num, int attr_num, UINT8 data[], int data_len)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_SENDIND ble_sendind_param;
    int index;

    APP_INFO1("Server number: %d", server_num);
    if ((server_num < 0) || (server_num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    if (app_ble_cb.ble_server[server_num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", server_num);
        return -1;
    }
    if(app_ble_cb.ble_server[server_num].congested)
    {
        APP_ERROR1("fail : congested(%d)!", app_ble_cb.ble_server[server_num].congested);
        return -1;
    }

    APP_INFO1("Service's attribute number: %d", attr_num);
    if (-1 == attr_num)
        return -1;

    status = BSA_BleSeSendIndInit(&ble_sendind_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeSendIndInit failed status = %d", status);
        return -1;
    }

    ble_sendind_param.conn_id = app_ble_cb.ble_server[server_num].conn_id;
    ble_sendind_param.attr_id = app_ble_cb.ble_server[server_num].attr[attr_num].attr_id;

    ble_sendind_param.data_len = data_len;

    for (index = 0; index < data_len ; index++)
    {
        ble_sendind_param.value[index] = data[index];
    }

    ble_sendind_param.need_confirm = FALSE;

    status = BSA_BleSeSendInd(&ble_sendind_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeSendInd failed status = %d", status);
        return -1;
    }

    return 0;
}


/*******************************************************************************
**
** Function         app_ble_server_profile_cback
**
** Description      BLE Server Profile callback.
**
** Returns          void
**
*******************************************************************************/
void app_ble_server_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data)
{
    int num, attr_index;
    int status;
    tBSA_BLE_SE_SENDRSP send_server_resp;
    UINT8 attribute_value[BSA_BLE_MAX_ATTR_LEN]={0x11,0x22,0x33,0x44};

    APP_DEBUG1("app_ble_server_profile_cback event = %d ", event);

    switch (event)
    {
    case BSA_BLE_SE_DEREGISTER_EVT:
        APP_INFO1("BSA_BLE_SE_DEREGISTER_EVT server_if:%d status:%d",
            p_data->ser_deregister.server_if, p_data->ser_deregister.status);
        num = app_ble_server_find_index_by_interface(p_data->ser_deregister.server_if);
        if(num < 0)
        {
            APP_ERROR0("no deregister pending!!");
            break;
        }

        app_ble_cb.ble_server[num].server_if = BSA_BLE_INVALID_IF;
        app_ble_cb.ble_server[num].enabled = FALSE;
        for (attr_index = 0 ; attr_index < BSA_BLE_ATTRIBUTE_MAX ; attr_index++)
        {
            memset(&app_ble_cb.ble_server[num].attr[attr_index], 0, sizeof(tAPP_BLE_ATTRIBUTE));
        }

        break;

    case BSA_BLE_SE_CREATE_EVT:
        APP_INFO1("BSA_BLE_SE_CREATE_EVT server_if:%d status:%d service_id:%d",
            p_data->ser_create.server_if, p_data->ser_create.status, p_data->ser_create.service_id);

        num = app_ble_server_find_index_by_interface(p_data->ser_create.server_if);

        /* search interface number */
        if(num < 0)
        {
            APP_ERROR0("no interface!!");
            break;
        }

        /* search attribute number */
        for (attr_index = 0 ; attr_index < BSA_BLE_ATTRIBUTE_MAX ; attr_index++)
        {
            if (app_ble_cb.ble_server[num].attr[attr_index].wait_flag == TRUE)
            {
                APP_INFO1("BSA_BLE_SE_CREATE_EVT if_num:%d, attr_num:%d", num, attr_index);
                if (p_data->ser_create.status == BSA_SUCCESS)
                {
                    app_ble_cb.ble_server[num].attr[attr_index].service_id = p_data->ser_create.service_id;
                    app_ble_cb.ble_server[num].attr[attr_index].wait_flag = FALSE;
                    break;
                }
                else  /* if CREATE fail */
                {
                    memset(&app_ble_cb.ble_server[num].attr[attr_index], 0, sizeof(tAPP_BLE_ATTRIBUTE));
                    break;
                }
            }
        }
        if (attr_index >= BSA_BLE_ATTRIBUTE_MAX)
        {
            APP_ERROR0("BSA_BLE_SE_CREATE_EVT no waiting!!");
            break;
        }

        break;

    case BSA_BLE_SE_ADDCHAR_EVT:
        APP_INFO1("BSA_BLE_SE_ADDCHAR_EVT status:%d", p_data->ser_addchar.status);
        APP_INFO1("attr_id:0x%x", p_data->ser_addchar.attr_id);
        num = app_ble_server_find_index_by_interface(p_data->ser_addchar.server_if);

        /* search interface number */
        if(num < 0)
        {
            APP_ERROR0("no interface!!");
            break;
        }

        for (attr_index = 0 ; attr_index < BSA_BLE_ATTRIBUTE_MAX ; attr_index++)
        {
            if (app_ble_cb.ble_server[num].attr[attr_index].wait_flag == TRUE)
            {
                APP_INFO1("if_num:%d, attr_num:%d", num, attr_index);
                if (p_data->ser_addchar.status == BSA_SUCCESS)
                {
                    app_ble_cb.ble_server[num].attr[attr_index].service_id = p_data->ser_addchar.service_id;
                    app_ble_cb.ble_server[num].attr[attr_index].attr_id = p_data->ser_addchar.attr_id;
                    app_ble_cb.ble_server[num].attr[attr_index].wait_flag = FALSE;
                    break;
                }
                else  /* if CREATE fail */
                {
                    memset(&app_ble_cb.ble_server[num].attr[attr_index], 0, sizeof(tAPP_BLE_ATTRIBUTE));
                    break;
                }
            }
        }
        if (attr_index >= BSA_BLE_ATTRIBUTE_MAX)
        {
            APP_ERROR0("BSA_BLE_SE_ADDCHAR_EVT no waiting!!");
            break;
        }

        break;

    case BSA_BLE_SE_START_EVT:
        APP_INFO1("BSA_BLE_SE_START_EVT status:%d", p_data->ser_start.status);
        break;

    case BSA_BLE_SE_READ_EVT:

        APP_INFO1("BSA_BLE_SE_READ_EVT status:%d", p_data->ser_read.status);
/*
        BSA_BleSeSendRspInit(&send_server_resp);
        send_server_resp.conn_id = p_data->ser_read.conn_id;
        send_server_resp.trans_id = p_data->ser_read.trans_id;
        send_server_resp.status = p_data->ser_read.status;
        send_server_resp.handle = p_data->ser_read.handle;
        send_server_resp.offset = p_data->ser_read.offset;
        send_server_resp.len = 4;
        send_server_resp.auth_req = GATT_AUTH_REQ_NONE;
        memcpy(send_server_resp.value, attribute_value, BSA_BLE_MAX_ATTR_LEN);
        APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.conn_id:%d, send_server_resp.trans_id:%d", send_server_resp.conn_id, send_server_resp.trans_id, send_server_resp.status);
        APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.status:%d,send_server_resp.auth_req:%d", send_server_resp.status,send_server_resp.auth_req);
        APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.handle:%d, send_server_resp.offset:%d, send_server_resp.len:%d", send_server_resp.handle,send_server_resp.offset,send_server_resp.len );
        BSA_BleSeSendRsp(&send_server_resp);
*/
        break;

    case BSA_BLE_SE_WRITE_EVT:
        APP_INFO1("BSA_BLE_SE_WRITE_EVT status:%d", p_data->ser_write.status);
        APP_DUMP("Write value", p_data->ser_write.value, p_data->ser_write.len);
        APP_INFO1("BSA_BLE_SE_WRITE_EVT trans_id:%d, conn_id:%d, handle:%d, is_prep:%d, offset:%d",
            p_data->ser_write.trans_id, p_data->ser_write.conn_id, p_data->ser_write.handle,
            p_data->ser_write.is_prep, p_data->ser_write.offset);

        if (p_data->ser_write.need_rsp)
        {
            BSA_BleSeSendRspInit(&send_server_resp);
            send_server_resp.conn_id = p_data->ser_write.conn_id;
            send_server_resp.trans_id = p_data->ser_write.trans_id;
            send_server_resp.status = p_data->ser_write.status;
            send_server_resp.handle = p_data->ser_write.handle;
            if(p_data->ser_write.is_prep)
            {
                send_server_resp.offset = p_data->ser_write.offset;
                send_server_resp.len = p_data->ser_write.len;
                memcpy(send_server_resp.value, p_data->ser_write.value, send_server_resp.len);
            }
            else
                send_server_resp.len = 0;
            APP_INFO1("BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:%d, send_server_resp.trans_id:%d", send_server_resp.conn_id, send_server_resp.trans_id, send_server_resp.status);
            APP_INFO1("BSA_BLE_SE_WRITE_EVT: send_server_resp.status:%d,send_server_resp.auth_req:%d", send_server_resp.status,send_server_resp.auth_req);
            APP_INFO1("BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:%d, send_server_resp.offset:%d, send_server_resp.len:%d", send_server_resp.handle,send_server_resp.offset,send_server_resp.len );
            BSA_BleSeSendRsp(&send_server_resp);
        }
        break;

    case BSA_BLE_SE_EXEC_WRITE_EVT:
        APP_INFO1("BSA_BLE_SE_EXEC_WRITE_EVT status:%d", p_data->ser_exec_write.status);
        APP_INFO1("BSA_BLE_SE_WRITE_EVT trans_id:%d, conn_id:%d, exec_write:%d",
            p_data->ser_exec_write.trans_id, p_data->ser_exec_write.conn_id, p_data->ser_exec_write.exec_write);

        BSA_BleSeSendRspInit(&send_server_resp);
        send_server_resp.conn_id = p_data->ser_exec_write.conn_id;
        send_server_resp.trans_id = p_data->ser_exec_write.trans_id;
        send_server_resp.status = p_data->ser_exec_write.status;
        send_server_resp.handle = 0;
        send_server_resp.len = 0;
        APP_INFO1("BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:%d, send_server_resp.trans_id:%d", send_server_resp.conn_id, send_server_resp.trans_id, send_server_resp.status);
        APP_INFO1("BSA_BLE_SE_WRITE_EVT: send_server_resp.status:%d", send_server_resp.status);
        BSA_BleSeSendRsp(&send_server_resp);

        break;

    case BSA_BLE_SE_OPEN_EVT:
        APP_INFO1("BSA_BLE_SE_OPEN_EVT status:%d", p_data->ser_open.reason);
        if (p_data->ser_open.reason == BSA_SUCCESS)
        {
            APP_INFO1("conn_id:0x%x", p_data->ser_open.conn_id);
            num = app_ble_server_find_index_by_interface(p_data->ser_open.server_if);
            /* search interface number */
            if(num < 0)
            {
                APP_ERROR0("no interface!!");
                break;
            }
            app_ble_cb.ble_server[num].conn_id = p_data->ser_open.conn_id;

#if 1
            /* XML Database update */
            app_read_xml_remote_devices();
            /* Add BLE service for this devices in XML database */
            app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->ser_open.remote_bda,
                    BSA_BLE_SERVICE_MASK);

            status = app_write_xml_remote_devices();
            if (status < 0)
            {
                APP_ERROR1("app_ble_write_remote_devices failed: %d", status);
            }
#endif
        }
        break;

    case BSA_BLE_SE_CLOSE_EVT:
        APP_INFO1("BSA_BLE_SE_CLOSE_EVT status:%d", p_data->ser_close.reason);
        APP_INFO1("conn_id:0x%x", p_data->ser_close.conn_id);
        num = app_ble_server_find_index_by_interface(p_data->ser_close.server_if);
        /* search interface number */
        if(num < 0)
        {
            APP_ERROR0("no interface!!");
            break;
        }
        app_ble_cb.ble_server[num].conn_id = BSA_BLE_INVALID_CONN;
        break;

    case BSA_BLE_SE_CONF_EVT:
        APP_INFO1("BSA_BLE_SE_CONF_EVT status:%d", p_data->ser_conf.status);
        APP_INFO1("conn_id:0x%x", p_data->ser_conf.conn_id);
        break;

    case BSA_BLE_SE_CONGEST_EVT:
        APP_INFO1("BSA_BLE_SE_CONGEST_EVT  :conn_id:0x%x, congested:%d",
                    p_data->ser_congest.conn_id, p_data->ser_congest.congested);
        num = app_ble_server_find_index_by_conn_id(p_data->ser_congest.conn_id);
        if (num >= 0)
        {
            app_ble_cb.ble_server[num].congested = p_data->ser_congest.congested;
        }
        break;

    case BSA_BLE_SE_MTU_EVT:
        APP_INFO1("BSA_BLE_SE_MTU_EVT conn_id:0x%x, mtu:%d",
                    p_data->ser_mtu.conn_id, p_data->ser_mtu.mtu);
        break;

    default:
        break;
    }

    /* forward the callback to registered applications */
    if(s_pCallback)
        s_pCallback(event, p_data);
}


/*******************************************************************************
 **
 ** Function        app_ble_server_open
 **
 ** Description     This is the ble open connection to ble client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_open(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_OPEN ble_open_param;
    int device_index;
    BD_ADDR bd_addr;
    int server_num;
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

    APP_INFO0("Select Server:");
    app_ble_server_display();
    server_num = app_get_choice("Select");

    if((server_num < 0) ||
       (server_num >= BSA_BLE_SERVER_MAX) ||
       (app_ble_cb.ble_server[server_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }

    if (app_ble_cb.ble_server[server_num].conn_id != BSA_BLE_INVALID_CONN)
    {
        APP_ERROR1("Connection already exist, conn_id = %d",
                app_ble_cb.ble_server[server_num].conn_id );
        return -1;
    }

    status = BSA_BleSeConnectInit(&ble_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeConnectInit failed status = %d", status);
        return -1;
    }

    ble_open_param.server_if = app_ble_cb.ble_server[server_num].server_if;
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
    bdcpy(ble_open_param.bd_addr, bd_addr);

    status = BSA_BleSeConnect(&ble_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeConnect failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_close(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_CLOSE ble_close_param;
    int server_num;

    APP_INFO0("Select Server:");
    app_ble_server_display();
    server_num = app_get_choice("Select");

    if((server_num < 0) ||
       (server_num >= BSA_BLE_SERVER_MAX) ||
       (app_ble_cb.ble_server[server_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    status = BSA_BleSeCloseInit(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCloseInit failed status = %d", status);
        return -1;
    }
    ble_close_param.conn_id = app_ble_cb.ble_server[server_num].conn_id;
    status = BSA_BleSeClose(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeClose failed status = %d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 **
 ** Function        app_ble_server_ext funcitons
 **
 **
 *******************************************************************************/
int app_ble_server_register_ext(UINT16 uuid, tBSA_BLE_CBACK *p_cback)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_REGISTER ble_register_param;
    UINT16 choice;
    int server_num;

    s_pCallback = p_cback;
    server_num = app_ble_server_find_free_server();
    if (server_num < 0)
    {
        APP_ERROR0("No more spaces!!");
        return -1;
    }

    if (!uuid)
    {
        APP_INFO0("Uuid is 0, use default value");
        choice = 0x9999;
    }
    else
    {
        choice = uuid;
    }

    status = BSA_BleSeAppRegisterInit(&ble_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppRegisterInit failed status = %d", status);
        return -1;
    }

    ble_register_param.uuid.len = 2;
    ble_register_param.uuid.uu.uuid16 = choice;
    ble_register_param.p_cback = app_ble_server_profile_cback;

    //BTM_BLE_SEC_NONE: No authentication and no encryption
    //BTM_BLE_SEC_ENCRYPT: encrypt the link with current key
    //BTM_BLE_SEC_ENCRYPT_NO_MITM: unauthenticated encryption
    //BTM_BLE_SEC_ENCRYPT_MITM: authenticated encryption
    ble_register_param.sec_act = BTM_BLE_SEC_NONE;

    status = BSA_BleSeAppRegister(&ble_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppRegister failed status = %d", status);
        return -1;
    }
    app_ble_cb.ble_server[server_num].enabled = TRUE;
    app_ble_cb.ble_server[server_num].server_if = ble_register_param.server_if;
    APP_INFO1("enabled:%d, server_if:%d", app_ble_cb.ble_server[server_num].enabled,
                    app_ble_cb.ble_server[server_num].server_if);
    return server_num;
}

int app_ble_server_deregister_ext(int server_num)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_DEREGISTER ble_deregister_param;
    int num;

    num = server_num;
    if ((num < 0) || (num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", num);
        return -1;
    }
    if (app_ble_cb.ble_server[num].enabled != TRUE)
    {
        APP_ERROR1("Server was not registered! = %d", num);
        return -1;
    }

    status = BSA_BleSeAppDeregisterInit(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregisterInit failed status = %d", status);
        return -1;
    }

    ble_deregister_param.server_if = app_ble_cb.ble_server[num].server_if;

    status = BSA_BleSeAppDeregister(&ble_deregister_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAppDeregister failed status = %d", status);
        return -1;
    }

    return 0;
}

int app_ble_server_create_service_uuid128(int server_num, UINT8 service_uuid[], int char_num, int is_primary)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_CREATE ble_create_param;
    UINT16  num_handle;
    int attr_num, index;

    if ((server_num < 0) || (server_num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    if (app_ble_cb.ble_server[server_num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", server_num);
        return -1;
    }

    num_handle = 2;
    num_handle += char_num * 2;
    if (!num_handle)
    {
        APP_ERROR1("wrong value = %d", num_handle);
        return -1;
    }

    if (is_primary != 0)
    {
        is_primary = 1;
    }

    attr_num = app_ble_server_find_free_attr_uuid128(server_num);
    if (attr_num < 0)
    {
        APP_ERROR1("Wrong attr number! = %d", attr_num);
        return -1;
    }
    status = BSA_BleSeCreateServiceInit(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateServiceInit failed status = %d", status);
        return -1;
    }

    ble_create_param.service_uuid.len = LEN_UUID_128;
    for (index = 0; index < LEN_UUID_128; index++) {
        ble_create_param.service_uuid.uu.uuid128[index] = service_uuid[index];
    }

    ble_create_param.server_if = app_ble_cb.ble_server[server_num].server_if;
    ble_create_param.num_handle = num_handle;
    if (is_primary != 0)
    {
        ble_create_param.is_primary = TRUE;
    }
    else
    {
        ble_create_param.is_primary = FALSE;
    }

    app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag = TRUE;

    status = BSA_BleSeCreateService(&ble_create_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCreateService failed status = %d", status);
        app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag = FALSE;
        return -1;
    }

    /* store information on control block */
    app_ble_cb.ble_server[server_num].attr[attr_num].attr_UUID.len = LEN_UUID_128;
    for (index = 0; index < LEN_UUID_128; index++) {
        app_ble_cb.ble_server[server_num].attr[attr_num].attr_UUID.uu.uuid128[index] = service_uuid[index];
    }
    app_ble_cb.ble_server[server_num].attr[attr_num].is_pri = ble_create_param.is_primary;
    app_ble_cb.ble_server[server_num].attr[attr_num].attr_type = BSA_GATTC_ATTR_TYPE_SRVC;

    while(app_ble_cb.ble_server[server_num].attr[attr_num].wait_flag == TRUE)
		{
        GKI_delay(5);
		}

    return attr_num;
}

int app_ble_server_add_char_uuid128(int server_num, int srvc_attr_num,
    UINT8 char_uuid128[], int is_descript, UINT16 attribute_permission, UINT16 characteristic_property)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_ADDCHAR ble_addchar_param;
    int char_attr_num, index;

    if ((server_num < 0) || (server_num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
    if (app_ble_cb.ble_server[server_num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", server_num);
        return -1;
    }

    if(srvc_attr_num < 0)
    {
        APP_ERROR0("app_ble_server_add_char : Invalid srvc_attr_num entered");
        return -1;
    }

    char_attr_num = app_ble_server_find_free_attr_uuid128(server_num);

    status = BSA_BleSeAddCharInit(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddCharInit failed status = %d", status);
        return -1;
    }
    ble_addchar_param.service_id = app_ble_cb.ble_server[server_num].attr[srvc_attr_num].service_id;
    ble_addchar_param.char_uuid.len = LEN_UUID_128;
    for (index = 0; index < LEN_UUID_128; index++) {
        ble_addchar_param.char_uuid.uu.uuid128[index] = char_uuid128[index];
    }

    if(is_descript)
    {
        ble_addchar_param.is_descr = TRUE;
        ble_addchar_param.perm = attribute_permission;
    }
    else
    {
        ble_addchar_param.is_descr = FALSE;
        ble_addchar_param.perm = attribute_permission;
        ble_addchar_param.property = characteristic_property;
    }

    app_ble_cb.ble_server[server_num].attr[char_attr_num].wait_flag = TRUE;

    status = BSA_BleSeAddChar(&ble_addchar_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeAddChar failed status = %d", status);
        return -1;
    }

    /* save all information */
    app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_UUID.len = LEN_UUID_128;
    for (index = 0; index < LEN_UUID_128; index++) {
        app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_UUID.uu.uuid128[index] = char_uuid128[index];
    }

    app_ble_cb.ble_server[server_num].attr[char_attr_num].prop = characteristic_property;
    app_ble_cb.ble_server[server_num].attr[char_attr_num].attr_type = BSA_GATTC_ATTR_TYPE_CHAR;

    while(app_ble_cb.ble_server[server_num].attr[char_attr_num].wait_flag == TRUE)
		{
        GKI_delay(5);
		}

    return char_attr_num;
}

int app_ble_server_start_service_ext(int server_num, int srvc_attr_num)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_START ble_start_param;
    int num, attr_num;

    num = server_num;
    if ((num < 0) || (num >= BSA_BLE_SERVER_MAX))
    {
        APP_ERROR1("Wrong server number! = %d", num);
        return -1;
    }
    if (app_ble_cb.ble_server[num].enabled != TRUE)
    {
        APP_ERROR1("Server was not enabled! = %d", num);
        return -1;
    }

    attr_num = srvc_attr_num;
    if(attr_num < 0)
    {
        APP_ERROR0("app_ble_server_start_service : Invalid attr_num entered");
        return -1;
    }

    status = BSA_BleSeStartServiceInit(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartServiceInit failed status = %d", status);
        return -1;
    }

    ble_start_param.service_id = app_ble_cb.ble_server[num].attr[attr_num].service_id;
    ble_start_param.sup_transport = BSA_BLE_GATT_TRANSPORT_LE_BR_EDR;

    APP_INFO1("service_id:%d, num:%d", ble_start_param.service_id, num);

    status = BSA_BleSeStartService(&ble_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeStartService failed status = %d", status);
        return -1;
    }
    return 0;
}

int app_ble_server_config_adv_data(UINT8 srvc_uuid128_array[],
        UINT8 app_ble_adv_value[], int data_len,
        UINT16 appearance_data, int is_scan_rsp)
{
	  int i = 0;
    tBSA_DM_BLE_ADV_CONFIG adv_conf;

    /*Adv.Data should be < 31bytes including Manufacturer data,Device Name, Appearance data, Services Info,etc.. */
    /* We are not receving all fields from user to reduce the complexity */
    memset(&adv_conf, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
    /* start advertising */
    adv_conf.len = data_len;
    adv_conf.flag = BSA_DM_BLE_ADV_FLAG_MASK;
    memcpy(adv_conf.p_val, app_ble_adv_value, data_len);
    /* All the masks/fields that are set will be advertised*/
    adv_conf.adv_data_mask = BSA_DM_BLE_AD_BIT_FLAGS|
        BSA_DM_BLE_AD_BIT_MANU|BSA_DM_BLE_AD_BIT_DEV_NAME|BSA_DM_BLE_AD_BIT_SERVICE;

    adv_conf.appearance_data = appearance_data;
    adv_conf.services_128b.list_cmpl = 0;

    adv_conf.num_service = 1;
    for (i = 0; i < 16; i++)
    {
        adv_conf.services_128b.uuid128[i] = srvc_uuid128_array[i];
    }
    adv_conf.uuid_val[0] = (srvc_uuid128_array[13] << 8) | (srvc_uuid128_array[12]);

    if(is_scan_rsp)
    {
        adv_conf.is_scan_rsp = 1;
    }
    else
    {
        adv_conf.is_scan_rsp = 0;
    }
    app_dm_set_ble_adv_data(&adv_conf);

    return 0;
}

int app_ble_server_send_read_rsp(void *p_cb_data, UINT8 rsp_data[], int len)
{
    tBSA_BLE_SE_SENDRSP send_server_resp;
    tBSA_BLE_MSG *p_data = (tBSA_BLE_MSG *)p_cb_data;

    if (len > BSA_BLE_MAX_ATTR_LEN)
    {
        APP_ERROR0("Ble server send rsp over limited length, failed!");
        return -1;
    }

    BSA_BleSeSendRspInit(&send_server_resp);
    send_server_resp.conn_id = p_data->ser_read.conn_id;
    send_server_resp.trans_id = p_data->ser_read.trans_id;
    send_server_resp.status = p_data->ser_read.status;
    send_server_resp.handle = p_data->ser_read.handle;
    send_server_resp.offset = p_data->ser_read.offset;
    send_server_resp.len = len - send_server_resp.offset;
    send_server_resp.auth_req = GATT_AUTH_REQ_NONE;
    memcpy(send_server_resp.value, (rsp_data + send_server_resp.offset), BSA_BLE_MAX_ATTR_LEN);

    BSA_BleSeSendRsp(&send_server_resp);

    APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.conn_id:%d, send_server_resp.trans_id:%d", send_server_resp.conn_id, send_server_resp.trans_id, send_server_resp.status);
    APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.status:%d,send_server_resp.auth_req:%d", send_server_resp.status,send_server_resp.auth_req);
    APP_INFO1("BSA_BLE_SE_READ_EVT: send_server_resp.handle:%d, send_server_resp.offset:%d, send_server_resp.len:%d", send_server_resp.handle,send_server_resp.offset,send_server_resp.len );

    return 0;
}
