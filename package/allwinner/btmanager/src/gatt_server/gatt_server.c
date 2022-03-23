/*
  * Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
  * Author        laumy <liumingyuan@allwinnertech.com>
  * Version       0.0.1
  *
  * Author: Kevin
  * Version: 1.0.0
  * Commit: redifine all the APIs and add new APIs
  *
  */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib/bluetooth.h>
#include <lib/hci.h>
#include <lib/hci_lib.h>

#include "aw_gatt_server.h"
#include "gatt_server_glue.h"
#include "log.h"

static bool adv_enable;

#define AW_BLE_VERSION_CODE "1.0.2(2019-11-06,17:07)"
#define MANUFACTURER_DATA_LEN	8
#define WIFI_LEN				6

#define BLE_CONF "/etc/bluetooth/bt_init.sh ble_start"

gatt_server_cb_t *gatt_server_cb = NULL;

int bt_manager_gatt_server_init(gatt_server_cb_t *cb)
{
	printf("===AW BLE server version: %s===\n", AW_BLE_VERSION_CODE);
	system(BLE_CONF);
	gatt_server_cb = cb;
	return CSM_init();
}

void bt_manager_gatt_server_deinit()
{
	gatt_server_cb = NULL;
	CSM_deinitGatts();
	return;
}

gatt_server_cb_t *aw_get_gatt_server_cb()
{
	return gatt_server_cb;
}

int bt_manager_gatt_create_service(gatt_add_svc_t *svc)
{
	return CSM_addService(svc->uuid,svc->primary,svc->number);
}

int bt_manager_gatt_add_characteristic(gatt_add_char_t *chr)
{
	return CSM_addChar(chr->svc_handle,chr->uuid,chr->properties,
			chr->permissions);
}

int bt_manager_gatt_add_descriptor(gatt_add_desc_t *desc)
{
	return CSM_addDesc(desc->svc_handle,desc->uuid,desc->permissions);
}

int bt_manager_gatt_start_service(gatt_star_svc_t *start_svc)
{
	return CSM_startService(start_svc->svc_handle);
}

int bt_manager_gatt_stop_service(gatt_stop_svc_t *stop_svc)
{
	return CSM_stopService(stop_svc->svc_handle);
}

int bt_manager_gatt_delete_service(gatt_del_svc_t *del_svc)
{
	return CSM_deleteService(del_svc->svc_handle);
}

int bt_manager_gatt_send_read_response(gatt_send_read_rsp_t *pData)
{
	CSM_sendResponse(pData->trans_id,pData->status,pData->svc_handle,
			pData->value,pData->value_len,pData->auth_req);
}

int bt_manager_gatt_send_write_response(gatt_write_rsp_t *pData)
{
	CSM_sendWriteResponse(pData->trans_id, pData->attr_handle, pData->state);
}

int bt_manager_gatt_send_notification(gatt_notify_rsp_t *pData)
{
	return CSM_sendIndication(pData->attr_handle,false,
			pData->value,pData->value_len);
}

int bt_manager_gatt_send_indication(gatt_indication_rsp_t *pData)
{
	return CSM_sendIndication(pData->attr_handle,true,
			pData->value,pData->value_len);
}

int bt_manager_gatt_enable_adv(bool enable, gatt_le_advertising_parameters_t *adv_param)
{
	struct hci_request rq;
	le_set_advertising_parameters_cp adv_params_cp;
	le_set_advertise_enable_cp advertise_cp;
	uint8_t status;
	int dd, ret;
	int hdev = 0;
	char *opt = NULL;

	if (!enable)
		goto enable_disable_adv;

	if (!adv_param) {
		pr_error("Invalid advertising parameters!");
		return -1;
	}

	memset(&adv_params_cp, 0, sizeof(adv_params_cp));

	if (adv_param->adv_type >= LE_ADV_TYPE_MAX) {
		pr_error("Invalid advertising type!");
		return -1;
	} else {
		adv_params_cp.advtype = adv_param->adv_type;
	}

	if (adv_param->adv_type != LE_ADV_DIRECT_HIGH_IND &&
			(adv_param->min_interval < 0x0020 ||
			 adv_param->min_interval > 0x4000 ||
			 adv_param->max_interval < 0x0020 ||
			 adv_param->max_interval > 0x4000 ||
			 adv_param->max_interval < adv_param->min_interval)) {
		pr_error("Invalid advertising interval range");
		return -1;
	} else if (adv_param->adv_type != LE_ADV_DIRECT_HIGH_IND) {
		adv_params_cp.min_interval = htobs(adv_param->min_interval);
		adv_params_cp.max_interval = htobs(adv_param->max_interval); /*range from 0x0020 to 0x4000*/
	}

	if (adv_param->own_addr_type >= LE_ADDRESS_MAX) {
		pr_error("Invalid own addr type!");
		return -1;
	} else {
		adv_params_cp.own_bdaddr_type = adv_param->own_addr_type;
	}

	if (adv_param->adv_type == LE_ADV_DIRECT_HIGH_IND ||
		adv_param->adv_type == LE_ADV_DIRECT_LOW_IND) {
		if (adv_param->peer_addr_type >= LE_PEER_ADDRESS_MAX) {
			pr_error("Invalid peer addr type!");
			return -1;
		}
		str2ba(adv_param->peer_addr, &adv_params_cp.direct_bdaddr);
		adv_params_cp.direct_bdaddr_type = adv_param->peer_addr_type;
	} else {
		/* advertising filter policy shall be ignored for directed advertising*/
		if (adv_param->filter >= LE_FILTER_POLICY_MAX) {
			pr_error("Invalid advertising filter policy!");
			return -1;
		} else {
			adv_params_cp.filter = adv_param->filter;
		}
	}

	if (adv_param->chan_map > LE_ADV_CHANNEL_ALL || adv_param->chan_map == LE_ADV_CHANNEL_NONE) {
		pr_error("Invalid advertising channel map:%d!", adv_param->chan_map);
		return -1;
	} else {
		pr_info("advertising channel map: 0x%x", adv_param->chan_map);
		adv_params_cp.chan_map = adv_param->chan_map; /*bit0:channel 37, bit1: channel 38, bit2: channel39*/
	}

	dd = hci_open_dev(hdev);

	if (dd < 0) {
		pr_error("Could not open device");
		return -1;
	}

//	adv_params_cp.min_interval = htobs(0x01E0);
//	adv_params_cp.min_interval = htobs(0x0020);
//	adv_params_cp.max_interval = htobs(0x01E0); /*range from 0x0020 to 0x4000*/
//	adv_params_cp.own_bdaddr_type = LE_PUBLIC_ADDRESS;
//	adv_params_cp.advtype = LE_ADV_IND; /*ADV_IND*/
//	adv_params_cp.chan_map = 0x07; /*bit0:channel 37, bit1: channel 38, bit2: channel39*/
//	adv_params_cp.filter = LE_PROCESS_ALL_REQ;

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
	rq.cparam = &adv_params_cp;
	rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);

	if (ret < 0) {
		pr_error("cannot send advertising param, ret: %d, status: %d", ret, status);
		goto done;
	} else if (status == 0x11) {
		pr_error("unsupported advertising interval range by the controller!");
		goto done;
	}

enable_disable_adv:
	memset(&advertise_cp, 0, sizeof(advertise_cp));
	if (enable)
		advertise_cp.enable = (uint8_t)1;
	else {
		dd = hci_open_dev(hdev);
		if (dd < 0) {
			pr_error("Could not open device");
			return -1;
		}
		advertise_cp.enable = (uint8_t)0;
	}

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
	rq.cparam = &advertise_cp;
	rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);
	if (ret < 0) {
		pr_error("cannot send enable advertising, ret: %d, status: %d", ret, status);
		goto done;
	} else {
		if(status == 0)
			pr_info("set adv enable");
		else if (status == 12)
			pr_info("likely already advertising...");
		else
			pr_info("UnExpected status");

		//if(cbs.onAdvEnable)
		//	cbs.onAdvEnable(0);
	}

	hci_close_dev(dd);
	return 0;

done:
//	if(cbs.onAdvEnable)
//		cbs.onAdvEnable(1);
	hci_close_dev(dd);
	return -1;
}

int bt_manager_gatt_server_disconnect(unsigned int handle, unsigned char reason)
{
	struct hci_request rq;
	disconnect_cp disconnect_params_cp;
	uint8_t status;
	int dd, ret;
	int hdev = 0;

	dd = hci_open_dev(hdev);

	if (dd < 0) {
		printf("%s:%d Could not open device\n", __func__, __LINE__);
		return -1;
	}

	memset(&disconnect_params_cp, 0, sizeof(disconnect_params_cp));
	disconnect_params_cp.handle = handle; /*0x0200*/
	disconnect_params_cp.reason = reason; /*#define 	HCI_ERROR_CODE_REMOTE_USER_TERM_CONN   0x13*/

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LINK_CTL;
	rq.ocf = OCF_DISCONNECT;
	rq.cparam = &disconnect_params_cp;
	rq.clen = DISCONNECT_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);
	if (ret < 0)
		goto done;

done:
	hci_close_dev(dd);
	return -1;
}

int bt_manager_gatt_server_clear_white_list()
{
	struct hci_request rq;
	uint8_t status;
	int dd, ret;
	int hdev = 0;

	dd = hci_open_dev(hdev);

	if (dd < 0) {
		printf("%s:%d Could not open device\n", __func__, __LINE__);
		return -1;
	}

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = OCF_LE_CLEAR_WHITE_LIST;
	rq.rparam = &status;
	rq.rlen = 1;

	ret = hci_send_req(dd, &rq, 1000);
	if (ret < 0)
		goto done;

done:
	hci_close_dev(dd);
	return -1;

}

static void hex_dump(char *pref, int width, unsigned char *buf, int len)
{
	register int i,n;

	for (i = 0, n = 1; i < len; i++, n++) {
		if (n == 1)
			printf("%s", pref);
		printf("%2.2X ", buf[i]);
		if (n == width) {
			printf("\n");
			n = 0;
		}
	}
	if (i && n!=1)
		printf("\n");
}

//int aw_gatt_set_local_name(int flag,char *ble_name, char *uuid)
int bt_manager_gatt_set_local_name(int flag,char *ble_name)
{
	int dd;
	uint8_t manuf_len;
	uint16_t ogf, ocf;
	int index;
	char advdata[32] = { 0 };
	char uuid_buf[5] = {0};

//    CHAR manufacturer_data[MANUFACTURER_DATA_LEN + WIFI_LEN]=
//   {
//        0x54,0x4d,0x5f,0x47,0x45,0x4e,0x49,0x45,0,0,0,0,0,0
//    };//"TMALL_GENIE";

	dd = hci_open_dev(0);
	if (dd < 0) {
		pr_error("Could not open device");
		return -1;
	}


//#define AD_LEN_MANUF	16
#define AD_LEN_FLAG		3
//#define AD_LEN_APPEARANCE	4
//#define AD_LEN_LIST_OF_UUID	4
#define AD_LEN_LOCAL_NAME	10

	index = 1;
//	manuf_len = 1 + 14;
//	advdata[index] = manuf_len;	/* manuf len */
//	advdata[index + 1] = 0xff;		/* ad type */
//	memcpy(advdata + 3, manufacturer_data, MANUFACTURER_DATA_LEN + WIFI_LEN);

//	index += AD_LEN_MANUF;
	advdata[index] = 0x02;			/* flag len */
	advdata[index + 1] = 0x01;		/* type for flag */
	advdata[index + 2] = flag;  //0x05
	index += AD_LEN_FLAG;

//	advdata[index] = 3;				/* appearance len */
//	advdata[index + 1] = 0x19;		/* type for appearance */
//	advdata[index + 2] = 0x01;
//	advdata[index + 3] = 0x00;

//	index += AD_LEN_APPEARANCE;
//	advdata[index] = 0x03;			/* uuid len */
//	advdata[index + 1] = 0x03;		/* type for complete list of 16-bit uuid */
//	advdata[index + 2] = 0x20;
//	advdata[index + 3] = 0x9e;


//	advdata[index] = 2+1;			/* uuid len */
//	advdata[index + 1] = 0x03;		/* type for  complete list of 16-bit uuid*/
//	sscanf(uuid_buf+2, "%2x", &advdata[index + 2]);		/* type for  complete list of 16-bit uuid*/
//	uuid_buf[2] = '\0';
//	sscanf(uuid, "%2x", &advdata[index + 3]);		/* type for  complete list of 16-bit uuid*/
//	index += AD_LEN_LIST_OF_UUID;

//	advdata[index] = 0x0A;			/* uuid len */
	advdata[index] = strlen(ble_name)+1;			/* name len */
	advdata[index + 1] = 0x09;		/* type for local name */
	index+=2;
	int i, name_len;
	name_len = strlen(ble_name);
	for(i=0;i<=name_len;i++) {
		advdata[index + i] = ble_name[i];
	}
//	advdata[index + 2] = 0x50;
//	advdata[index + 3] = 0x65;
//	advdata[index + 4] = 0x64;
//	advdata[index + 5] = 0x6F;
//	advdata[index + 6] = 0x6D;
//	advdata[index + 7] = 0x65;
//	advdata[index + 8] = 0x75;
//	advdata[index + 9] = 0x65;
//	advdata[index + 10] = 0x72;

	// total length
//	advdata[0] = AD_LEN_MANUF + AD_LEN_FLAG + AD_LEN_APPEARANCE	+ AD_LEN_LIST_OF_UUID + AD_LEN_LOCAL_NAME;
	//advdata[0] = AD_LEN_FLAG + AD_LEN_LIST_OF_UUID + (name_len + 2);
	advdata[0] = AD_LEN_FLAG + (name_len + 2);
	hex_dump("  ", 8, (unsigned char *)advdata , 32); fflush(stdout);

	ogf = OGF_LE_CTL; ocf = OCF_LE_SET_ADVERTISING_DATA;
	if (hci_send_cmd(dd, ogf, ocf, LE_SET_ADVERTISING_DATA_CP_SIZE, advdata) < 0) {
		pr_info("Send failed");
		return -2;
	}

	hci_close_dev(dd);
	return 0;
}

int bt_manager_gatt_set_adv_data(gatt_adv_data_t *adv_data)
{
	int dd;
	uint8_t manuf_len;
	uint16_t ogf, ocf;
	int index;
//	char advdata[32] = { 0 };
	le_set_advertising_data_cp advdata;

	dd = hci_open_dev(0);
	if (dd < 0) {
		pr_error("Could not open device");
		return -1;
	}

//	advdata[0] = adv_data->data_len;
//	memcpy(&advdata[1], adv_data->data, adv_data->data_len);
	memset(&advdata, 0, sizeof(le_set_advertising_data_cp));
	advdata.length = adv_data->data_len;
	memcpy(advdata.data, adv_data->data, adv_data->data_len);

	ogf = OGF_LE_CTL; ocf = OCF_LE_SET_ADVERTISING_DATA;
	if (hci_send_cmd(dd, ogf, ocf, 32, (void *)&advdata) < 0) {
		pr_info("Send failed");
		return -2;
	}

	hci_close_dev(dd);
	return 0;
}

int bt_manager_gatt_set_scan_rsp_data(gatt_rsp_data_t *rsp_data)
{
	int dd;
	uint8_t manuf_len;
	uint16_t ogf, ocf;
	int index;
	char rspdata[32] = { 0 };

	dd = hci_open_dev(0);
	if (dd < 0) {
		pr_error("Could not open device");
		return -1;
	}

	rspdata[0] = rsp_data->data_len;
	memcpy(&rspdata[1], rsp_data->data, rsp_data->data_len);

	ogf = OGF_LE_CTL; ocf = OCF_LE_SET_SCAN_RESPONSE_DATA;
	if (hci_send_cmd(dd, ogf, ocf, 32, rspdata) < 0) {
		pr_info("Send failed");
		return -2;
	}

	hci_close_dev(dd);
	return 0;
}
