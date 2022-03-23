/*****************************************************************************
 **
 **  Name:           app_disc.c
 **
 **  Description:    Bluetooth Discovery functions
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include "app_disc.h"

#include "bsa_api.h"
#include "app_xml_param.h"
#include "app_utils.h"
#include "app_services.h"
#include "app_utils.h"
#include "app_xml_utils.h"

#define HCI_EIR_DEVICE_ID_TYPE  0x10    /* Device Id EIR Tag (not yet official) */

typedef struct
{
    int nb_devices;
    tBSA_DISC_CBACK *p_disc_cback;
} tAPP_DISC_CB;

tAPP_DISC_CB app_disc_cb;

tAPP_DISCOVERY_CB app_discovery_cb;

typedef struct {
    UINT16 vendor;
    UINT16 vendor_id_source;
} tAPP_VENDOR;

#define APP_VENDOR_APPLE_COUNT 2
#define VENDOR_SOURCE_BT 1
#define VENDOR_SOURCE_USB 2
static tAPP_VENDOR app_vendor_apple[APP_VENDOR_APPLE_COUNT] =
{
    {76, VENDOR_SOURCE_BT},
    {1452, VENDOR_SOURCE_USB}
};

static UINT8 app_get_dev_platform(UINT16 vendor, UINT16 vendor_id_source);

/*******************************************************************************
 **
 ** Function         app_disc_display_devices
 **
 ** Description      This function is used to list discovered devices
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_disc_display_devices(void)
{
    int index;

    for (index = 0; index < APP_DISC_NB_DEVICES; index++)
    {
        if (app_discovery_cb.devs[index].in_use != FALSE)
        {
            APP_INFO1("Dev:%d", index);
            APP_INFO1("\tBdaddr:%02x:%02x:%02x:%02x:%02x:%02x",
                    app_discovery_cb.devs[index].device.bd_addr[0],
                    app_discovery_cb.devs[index].device.bd_addr[1],
                    app_discovery_cb.devs[index].device.bd_addr[2],
                    app_discovery_cb.devs[index].device.bd_addr[3],
                    app_discovery_cb.devs[index].device.bd_addr[4],
                    app_discovery_cb.devs[index].device.bd_addr[5]);
            APP_INFO1("\tName:%s", app_discovery_cb.devs[index].device.name);
            APP_INFO1("\tClassOfDevice:%02x:%02x:%02x => %s",
                    app_discovery_cb.devs[index].device.class_of_device[0],
                    app_discovery_cb.devs[index].device.class_of_device[1],
                    app_discovery_cb.devs[index].device.class_of_device[2],
                    app_get_cod_string(
                            app_discovery_cb.devs[index].device.class_of_device));
            APP_INFO1("\tRssi:%d", app_discovery_cb.devs[index].device.rssi);
            if (app_discovery_cb.devs[index].device.eir_vid_pid[0].valid)
            {
                APP_INFO1("\tVidSrc:%d Vid:0x%04X Pid:0x%04X Version:0x%04X",
                        app_discovery_cb.devs[index].device.eir_vid_pid[0].vendor_id_source,
                        app_discovery_cb.devs[index].device.eir_vid_pid[0].vendor,
                        app_discovery_cb.devs[index].device.eir_vid_pid[0].product,
                        app_discovery_cb.devs[index].device.eir_vid_pid[0].version);
            }
        }
    }
}

/*******************************************************************************
 **
 ** Function         app_disc_parse_eir
 **
 ** Description      This function is used to parse EIR data
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_disc_parse_eir(UINT8 *p_eir)
{
    UINT8 *p = p_eir;
    UINT8 eir_length;
    UINT8 eir_tag;

    while(1)
    {
        /* Read Tag's length */
        STREAM_TO_UINT8(eir_length, p);
        if (eir_length == 0)
        {
            break;    /* Last Tag */
        }
        eir_length--;

        /* Read Tag Id */
        STREAM_TO_UINT8(eir_tag, p);

        switch(eir_tag)
        {
        case HCI_EIR_FLAGS_TYPE:
            APP_DUMP("FLAGS_TYPE", p, eir_length);
            break;
        case HCI_EIR_MORE_16BITS_UUID_TYPE:
            APP_DUMP("MORE_16BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_COMPLETE_16BITS_UUID_TYPE:
            APP_DUMP("COMPLETE_16BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_MORE_32BITS_UUID_TYPE:
            APP_DUMP("MORE_32BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_COMPLETE_32BITS_UUID_TYPE:
            APP_DUMP("COMPLETE_32BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_MORE_128BITS_UUID_TYPE:
            APP_DUMP("MORE_128BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_COMPLETE_128BITS_UUID_TYPE:
            APP_DUMP("COMPLETE_128BITS_UUID_TYPE", p, eir_length);
            break;
        case HCI_EIR_SHORTENED_LOCAL_NAME_TYPE:
            APP_DUMP("SHORTENED_LOCAL_NAME_TYPE", p, eir_length);
            break;
        case HCI_EIR_COMPLETE_LOCAL_NAME_TYPE:
            APP_DUMP("COMPLETE_LOCAL_NAME_TYPE", p, eir_length);
            break;
        case HCI_EIR_TX_POWER_LEVEL_TYPE:
            APP_DUMP("TX_POWER_LEVEL_TYPE", p, eir_length);
            break;
        case HCI_EIR_3D_SYNC_TYPE:
            APP_DUMP("3D_SYNC_TYPE", p, eir_length);
            break;
        case HCI_EIR_MANUFACTURER_SPECIFIC_TYPE:
            APP_DUMP("MANUFACTURER_SPECIFIC_TYPE", p, eir_length);
            break;
        case HCI_EIR_DEVICE_ID_TYPE:
            APP_DUMP("DEVICE_ID_TYPE", p, eir_length);
            break;
        case BTM_BLE_AD_TYPE_SERVICE_DATA:
            APP_DUMP("SERVICE_DATA_TYPE", p, eir_length);
            break;
        default:
            APP_DUMP("Unknown EIR (Tag and data)", p - 1 , eir_length + 1);
            break;
        }
        p += eir_length;
    }
}

/*******************************************************************************
 **
 ** Function         app_generic_disc_cback
 **
 ** Description      Discovery callback
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_generic_disc_cback(tBSA_DISC_EVT event, tBSA_DISC_MSG *p_data)
{
    int index;

    /* If User provided a callback, let's call it */
    if (app_disc_cb.p_disc_cback)
    {
        app_disc_cb.p_disc_cback(event, p_data);
    }

    switch (event)
    {
    /* a New Device has been discovered */
    case BSA_DISC_NEW_EVT:
        /* check if this device has already been received (update) */
        for (index = 0; index < APP_DISC_NB_DEVICES; index++)
        {
            if ((app_discovery_cb.devs[index].in_use == TRUE) &&
                (!bdcmp(app_discovery_cb.devs[index].device.bd_addr, p_data->disc_new.bd_addr)))
            {
                /* Update device */
                APP_INFO1("Update device:%d", index);
                app_discovery_cb.devs[index].device = p_data->disc_new;
                break;
            }
        }
        /* If this is a new device */
        if (index >= APP_DISC_NB_DEVICES)
        {
            /* Look for a free place to store dev info */
            for (index = 0; index < APP_DISC_NB_DEVICES; index++)
            {
                if (app_discovery_cb.devs[index].in_use == FALSE)
                {
                    APP_INFO1("New Discovered device:%d", index);
                    app_discovery_cb.devs[index].in_use = TRUE;
                    memcpy(&app_discovery_cb.devs[index].device, &p_data->disc_new,
                            sizeof(tBSA_DISC_REMOTE_DEV));
                    break;
                }
            }
        }
        /* If this is a new device */
        if (index >= APP_DISC_NB_DEVICES)
        {
            APP_INFO0("No room to save new discovered");
        }

        APP_INFO1("\tBdaddr:%02x:%02x:%02x:%02x:%02x:%02x",
                p_data->disc_new.bd_addr[0],
                p_data->disc_new.bd_addr[1],
                p_data->disc_new.bd_addr[2],
                p_data->disc_new.bd_addr[3],
                p_data->disc_new.bd_addr[4],
                p_data->disc_new.bd_addr[5]);
        APP_INFO1("\tName:%s", p_data->disc_new.name);
        APP_INFO1("\tClassOfDevice:%02x:%02x:%02x => %s",
                p_data->disc_new.class_of_device[0],
                p_data->disc_new.class_of_device[1],
                p_data->disc_new.class_of_device[2],
                app_get_cod_string(p_data->disc_new.class_of_device));
        APP_INFO1("\tServices:0x%08x (%s)",
                (int) p_data->disc_new.services,
                app_service_mask_to_string(p_data->disc_new.services));
        APP_INFO1("\tRssi:%d", p_data->disc_new.rssi);
        if (p_data->disc_new.eir_vid_pid[0].valid)
        {
            APP_INFO1("\tVidSrc:%d Vid:0x%04X Pid:0x%04X Version:0x%04X",
                    p_data->disc_new.eir_vid_pid[0].vendor_id_source,
                    p_data->disc_new.eir_vid_pid[0].vendor,
                    p_data->disc_new.eir_vid_pid[0].product,
                    p_data->disc_new.eir_vid_pid[0].version);
        }

        if (p_data->disc_new.eir_data[0])
        {
            app_disc_parse_eir(p_data->disc_new.eir_data);
        }

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
        APP_INFO1("\tinq_result_type:%d ble_addr_type:%d device_type:%d",
                p_data->disc_new.inq_result_type,
                p_data->disc_new.ble_addr_type,
                p_data->disc_new.device_type);
#endif
        break;

    case BSA_DISC_CMPL_EVT: /* Discovery complete. */
        APP_INFO0("Discovery complete");
        app_disc_cb.p_disc_cback = NULL;
        break;

    case BSA_DISC_DEV_INFO_EVT: /* Discovery Device Info */
        APP_INFO1("Discover Device Info status:%d", p_data->dev_info.status);
        APP_INFO1("\tBdaddr:%02x:%02x:%02x:%02x:%02x:%02x",
                 p_data->dev_info.bd_addr[0],
                 p_data->dev_info.bd_addr[1],
                 p_data->dev_info.bd_addr[2],
                 p_data->dev_info.bd_addr[3],
                 p_data->dev_info.bd_addr[4],
                 p_data->dev_info.bd_addr[5]);
        if (p_data->dev_info.status == BSA_SUCCESS)
        {
            UINT8 dev_platform = app_get_dev_platform(p_data->dev_info.vendor,
                        p_data->dev_info.vendor_id_source);

            APP_INFO1("\tDevice VendorIdSource:0x%x", p_data->dev_info.vendor_id_source);
            APP_INFO1("\tDevice Vendor:0x%x", p_data->dev_info.vendor);
            APP_INFO1("\tDevice Product:0x%x", p_data->dev_info.product);
            APP_INFO1("\tDevice Version:0x%x", p_data->dev_info.version);
            APP_INFO1("\tDevice SpecId:0x%x", p_data->dev_info.spec_id);

            if (dev_platform == BSA_DEV_PLATFORM_IOS)
            {
                tBSA_SEC_ADD_SI_DEV si_dev;
                BSA_SecAddSiDevInit(&si_dev);
                bdcpy(si_dev.bd_addr, p_data->dev_info.bd_addr);
                si_dev.platform = dev_platform;
                BSA_SecAddSiDev(&si_dev);

                app_xml_update_si_dev_platform_db(app_xml_si_devices_db,
                    APP_NUM_ELEMENTS(app_xml_si_devices_db),
                    p_data->dev_info.bd_addr, dev_platform);
                app_write_xml_si_devices();
            }
        }
        break;
    case BSA_DISC_REMOTE_NAME_EVT:
        APP_INFO1("BSA_DISC_REMOTE_NAME_EVT (status=%d)", p_data->remote_name.status);
        if (p_data->dev_info.status == BSA_SUCCESS)
        {
            APP_INFO1("Read device name for %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->remote_name.bd_addr[0], p_data->remote_name.bd_addr[1],
                p_data->remote_name.bd_addr[2], p_data->remote_name.bd_addr[3],
                p_data->remote_name.bd_addr[4], p_data->remote_name.bd_addr[5]);
            APP_INFO1("Name:%s", p_data->remote_name.remote_bd_name);
        }
        break;

    default:
        APP_ERROR1("app_generic_disc_cback unknown event:%d", event);
        break;
    }
}

/*******************************************************************************
 **
 ** Function         app_disc_start_regular
 **
 ** Description      Start Device discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_regular(tBSA_DISC_CBACK *p_custom_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Regular Discovery");

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = app_disc_cb.nb_devices;
    disc_start_param.duration = 4;
    disc_start_param.mode = BSA_DM_GENERAL_INQUIRY;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    disc_start_param.mode |= BSA_BLE_GENERAL_INQUIRY;
#endif

    /* Save the provided custom callback */
    app_disc_cb.p_disc_cback = p_custom_disc_cback;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

#if ((defined BLE_INCLUDED) && (BLE_INCLUDED == TRUE))
/*******************************************************************************
 **
 ** Function         app_disc_start_ble_regular
 **
 ** Description      Start BLE Device discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_ble_regular(tBSA_DISC_CBACK *p_custom_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Regular Discovery");

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = app_disc_cb.nb_devices;
    disc_start_param.duration = 4;
    disc_start_param.mode = BSA_BLE_GENERAL_INQUIRY;

    /* Save the provided custom callback */
    app_disc_cb.p_disc_cback = p_custom_disc_cback;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}
#endif

/*******************************************************************************
 **
 ** Function         app_disc_start_services
 **
 ** Description      Start Device service discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_services(tBSA_SERVICE_MASK services)
{
    int status;
    tBSA_DISC_START disc_start_param;

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = 0;
    disc_start_param.duration = 4;

    disc_start_param.services = services;
    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    APP_INFO0("Start Services filtered Discovery");
    APP_INFO1("Look for (%s) services", app_service_mask_to_string(services));
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_cod
 **
 ** Description      Start Device Class Of Device discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_cod(unsigned short services, unsigned char major, unsigned char minor,
        tBSA_DISC_CBACK *p_custom_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;
    DEV_CLASS dev_class;
    DEV_CLASS dev_class_mask;
    unsigned short services_mask;
    unsigned char major_mask;
    unsigned char minor_mask;

    APP_INFO0("Start COD filtered Discovery");
    APP_INFO1("Look for COD service:%x major:%x minor:%x", services, major, minor);

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = 0;
    disc_start_param.duration = 4;
    disc_start_param.services = 0;
    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    /* Filter on class of device */
    disc_start_param.filter_type = BSA_DM_INQ_DEV_CLASS;
    FIELDS_TO_COD(dev_class, minor, major, services);
    APP_INFO1("COD: 0x%02x 0x%02x 0x%02x", dev_class[0], dev_class[1],dev_class[2]);
    memcpy(disc_start_param.filter_cond.dev_class_cond.dev_class,
           dev_class,
           sizeof(DEV_CLASS));

    /*
     * Prepare Class of device Mask
     */
    services_mask = services;    /* Get all services requested */

    /* If application request a specific Major */
    if (major != 0)
        major_mask = BTM_COD_MAJOR_CLASS_MASK;    /* Major must match */
    else
        major_mask = 0;                         /* don't care of Major */

    /* If application request a specific Minor */
    if (minor != 0)
        minor_mask = BTM_COD_MINOR_CLASS_MASK;      /* Minor must match */
    else
        minor_mask = 0;                         /* don't care of Minor */

    FIELDS_TO_COD(dev_class_mask, minor_mask, major_mask, services_mask);
    APP_INFO1("COD Mask: 0x%02x 0x%02x 0x%02x", dev_class_mask[0], dev_class_mask[1],dev_class_mask[2]);
    memcpy(disc_start_param.filter_cond.dev_class_cond.dev_class_mask,
           dev_class_mask,
           sizeof(DEV_CLASS));

    /* Save the provided custom callback */
    app_disc_cb.p_disc_cback = p_custom_disc_cback;

    /* Start Discovery */
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_abort
 **
 ** Description      Abort Device discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_abort(void)
{
    tBSA_STATUS status;
    tBSA_DISC_ABORT disc_abort_param;

    APP_INFO0("Aborting Discovery");

    BSA_DiscAbortInit(&disc_abort_param);
    status = BSA_DiscAbort(&disc_abort_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function         app_disc_start_dev_info
 **
 ** Description      Start device information discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_dev_info(BD_ADDR bd_addr, tBSA_DISC_CBACK *p_custom_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO1("Start Device Info Discovery for %02x-%02x-%02x-%02x-%02x-%02x",
            bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3],
            bd_addr[4], bd_addr[5]);

    app_disc_cb.p_disc_cback = p_custom_disc_cback;

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.device_info = TRUE; /* Look for Device Info */

    /* Filter on class of device */
    disc_start_param.filter_type = BTA_DM_INQ_BD_ADDR;
    bdcpy(disc_start_param.filter_cond.bd_addr, bd_addr);

    /* Start Discovery */
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_brcm_filter_cod
 **
 ** Description      Start Device discovery based on Brcm and COD filters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_brcm_filter_cod(tBSA_DISC_BRCM_FILTER brcm_filter,
        unsigned short services, unsigned char major, unsigned char minor,
        tBSA_DISC_CBACK *p_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;
    DEV_CLASS dev_class;
    DEV_CLASS dev_class_mask;
    unsigned short services_mask;
    unsigned char major_mask;
    unsigned char minor_mask;

    APP_INFO0("Start COD and BRCM-specific filtered Discovery");
    APP_INFO1("Look for COD service:%x major:%x minor:%x and BRCM filter:0x%x",
            services, major, minor, brcm_filter);

    /* Save the provided callback */
    app_disc_cb.p_disc_cback = p_disc_cback;

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = 0;
    disc_start_param.duration = 4;
    disc_start_param.services = 0;
    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    /* Add BRCM Filter */
    disc_start_param.brcm_filter = brcm_filter;

    /* Filter on class of device */
    disc_start_param.filter_type = BSA_DM_INQ_DEV_CLASS;
    FIELDS_TO_COD(dev_class, minor, major, services);
    APP_INFO1("COD: 0x%02x 0x%02x 0x%02x", dev_class[0], dev_class[1],dev_class[2]);
    memcpy(disc_start_param.filter_cond.dev_class_cond.dev_class,
           dev_class,
           sizeof(DEV_CLASS));

    /*
     * Prepare Class of device Mask
     */
    services_mask = services;    /* Get all services requested */

    /* If application request a specific Major */
    if (major != 0)
        major_mask = BTM_COD_MAJOR_CLASS_MASK;    /* Major must match */
    else
        major_mask = 0;                         /* don't care of Major */

    /* If application request a specific Minor */
    if (minor != 0)
        minor_mask = BTM_COD_MINOR_CLASS_MASK;      /* Minor must match */
    else
        minor_mask = 0;                         /* don't care of Minor */

    FIELDS_TO_COD(dev_class_mask, minor_mask, major_mask, services_mask);
    APP_INFO1("COD Mask: 0x%02x 0x%02x 0x%02x", dev_class_mask[0], dev_class_mask[1],dev_class_mask[2]);
    memcpy(disc_start_param.filter_cond.dev_class_cond.dev_class_mask,
           dev_class_mask,
           sizeof(DEV_CLASS));

    /* Start Discovery */
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_bdaddr
 **
 ** Description      Start Device discovery based on BdAddr
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_bdaddr(BD_ADDR bd_addr, BOOLEAN name_req,
        tBSA_DISC_CBACK *p_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start BdAddr filtered Discovery");
    APP_INFO1("Look for BdAddr :%02X-%02X-%02X-%02X-%02X-%02X",
            bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

    /* Save the provided callback */
    app_disc_cb.p_disc_cback = p_disc_cback;

    /* now, let's prepare the Discovery parameters */
    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    /* Stop when the device is discovered */
    disc_start_param.nb_devices = 1;
    disc_start_param.duration = 1;  /* 1.28 second */
    disc_start_param.services = 0;
    /* Do not request remote name (to prevent BB connection) */
    disc_start_param.skip_name_request = TRUE;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    /* Filter on BdAddr */
    disc_start_param.filter_type = BSA_DM_INQ_BD_ADDR;
    bdcpy(disc_start_param.filter_cond.bd_addr, bd_addr);

    /* Start Discovery */
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_bdaddr_services
 **
 ** Description      Start Service discovery based on BdAddr
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_bdaddr_services(BD_ADDR bd_addr,
        tBSA_SERVICE_MASK services, tBSA_DISC_CBACK *p_disc_cback)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Services search for one device");
    APP_INFO1("Device BdAddr :%02X-%02X-%02X-%02X-%02X-%02X",
            bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

    /* Save the provided callback */
    app_disc_cb.p_disc_cback = p_disc_cback;

    /* now, let's prepare the Discovery parameters */
    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;

    /* Filter on BdAddr provided */
    disc_start_param.filter_type = BSA_DM_INQ_BD_ADDR;
    bdcpy(disc_start_param.filter_cond.bd_addr, bd_addr);

    /* Look for the service(s) requested */
    disc_start_param.services = services;

    /* The Server will skip the inquiry part, so we can ignore the Inquiry parameters */

    /* Clear the device table */
    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    /* Start Discovery */
    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_update
 **
 ** Description      Start Device discovery with update parameter
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_update(tBSA_DISC_UPDATE update)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Update mode specific Discovery");

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.nb_devices = 0;
    disc_start_param.duration = 8;
    disc_start_param.update = update;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_limited
 **
 ** Description      Start Device Limited discovery
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_disc_start_limited(void)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Limited Discovery");

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.mode = BSA_DM_LIMITED_INQUIRY;
    disc_start_param.nb_devices = 0;
    disc_start_param.duration = 4;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_start_power
 **
 ** Description      Start discovery with specific inquiry TX power
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int app_disc_start_power(INT8 inq_tx_power)
{
    int status;
    tBSA_DISC_START disc_start_param;

    APP_INFO0("Start Inquiry power specific Discovery");

    BSA_DiscStartInit(&disc_start_param);

    disc_start_param.cback = app_generic_disc_cback;
    disc_start_param.inq_tx_power = inq_tx_power;

    memset(app_discovery_cb.devs, 0, sizeof(app_discovery_cb.devs));

    status = BSA_DiscStart(&disc_start_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_disc_find_device
 **
 ** Description      Find a device in the list of discovered devices
 **
 ** Parameters       bda: BD address of the searched device
 **
 ** Returns          Pointer to the discovered device, NULL if not found
 **
 *******************************************************************************/
tBSA_DISC_DEV *app_disc_find_device(BD_ADDR bda)
{
    int index;

    /* Unfortunately, the BSA_SEC_AUTH_CMPL_EVT does not contain COD */
    /* let's look in the Discovery database */
    for (index = 0; index < APP_DISC_NB_DEVICES; index++)
    {
        if ((app_discovery_cb.devs[index].in_use != FALSE) &&
            (bdcmp(app_discovery_cb.devs[index].device.bd_addr, bda) == 0))
        {
            return &app_discovery_cb.devs[index];
        }
    }
    return NULL;
}


/*******************************************************************************
 **
 ** Function         app_disc_find_device_with_name
 **
 ** Description      Find a device in the list of discovered devices
 **
 ** Parameters       name: name of the searched device
 **
 ** Returns          Pointer to the discovered device, NULL if not found
 **
 *******************************************************************************/
tBSA_DISC_DEV *app_disc_find_device_with_name(char bd_name[])
{
    int index;

    /* Unfortunately, the BSA_SEC_AUTH_CMPL_EVT does not contain COD */
    /* let's look in the Discovery database */
    for (index = 0; index < APP_DISC_NB_DEVICES; index++)
    {
        if ((app_discovery_cb.devs[index].in_use != FALSE) &&
            (strcmp((char *)(app_discovery_cb.devs[index].device.name), bd_name) == 0))
        {
            return &app_discovery_cb.devs[index];
        }
    }
    return NULL;
}

/*******************************************************************************
 **
 ** Function         app_disc_set_nb_devices
 **
 ** Description      Set the maximum number of returned discovered devices
 **
 ** Parameters       nb_devices : maximum number of returned devices
 **
 ** Returns          0 if successful, -1 in case of error
 **
 *******************************************************************************/
int app_disc_set_nb_devices(int nb_devices)
{
    app_disc_cb.nb_devices = nb_devices;

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_get_dev_platform
 **
 ** Description      Get device platform
 **
 ** Returns          Device platform
 **
 *******************************************************************************/
static UINT8 app_get_dev_platform(UINT16 vendor, UINT16 vendor_id_source)
{
    UINT8 platform = BSA_DEV_PLATFORM_UNKNOWN;
    int i;

    for (i = 0; i < APP_VENDOR_APPLE_COUNT; i++)
    {
        if (app_vendor_apple[i].vendor == vendor &&
            app_vendor_apple[i].vendor_id_source == vendor_id_source)
        {
            platform = BSA_DEV_PLATFORM_IOS;
            break;
        }
    }

    return platform;
}
/*******************************************************************************
 **
 ** Function         app_disc_read_remote_device_name
 **
 ** Description      Start Read Remote Device Name
 **
 ** Returns          int
 **
 *******************************************************************************/
 int app_disc_read_remote_device_name(BD_ADDR bd_addr,tBSA_TRANSPORT transport)
{
    tBSA_STATUS status;
    tBSA_DISC_READ_REMOTE_NAME disc_read_remote_name_param;

    BSA_ReadRemoteNameInit(&disc_read_remote_name_param);
    disc_read_remote_name_param.cback = app_generic_disc_cback;
    disc_read_remote_name_param.transport = transport;
    bdcpy(disc_read_remote_name_param.bd_addr, bd_addr);

    status = BSA_ReadRemoteName(&disc_read_remote_name_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DiscStart failed status:%d", status);
        return -1;
    }
    return 0;
}
