#include <bluetooth_socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BLE_SERVER_UUID        0x9999
#define BLE_SERVICE_UUID       "0000180a-0000-1000-8000-00805f9b34fb"
#define BLE_CHAR_UUID          "00009999-0000-1000-8000-00805f9b34fb"
#define BLE_DESC_UUID          "00002902-0000-1000-8000-00805f9b34fb"

#define BLE_CHAR1_UUID         "00008888-0000-1000-8000-00805f9b34fb"

#define BLE_DESC1_UUID         "00002903-0000-1000-8000-00805f9b34fb"
#define BLE_DESC2_UUID         "00002904-0000-1000-8000-00805f9b34fb"

unsigned char rsp_data[300] = "productID:test_huang,dsn:123456,sessionId:e20d002c-2f8c-47cd-97bd-55c17c6f0451,codeChallenge:GQHx6s9uit1li8CVuRHmwG4MOI4qAah7hL4xpMls8ao,codeChallengeMethod:S256.";
c_bt c;
static int client_read_flag = 0;

void ble_event_f(BLE_EVENT event, void *reply, int *len)
{
    tBleCallbackData ble_callback_data;
    unsigned char read_char_rsp[4] = {0x11, 0x22, 0x33, 0x44};
    unsigned char read_char1_rsp[4] = {0x55, 0x66, 0x77, 0x88};

    switch(event)
    {
        case BLE_SE_CONNECTED_EVT:
        {
		  printf("Ble server connected event!\n");
		  break;
        }

        case BLE_SE_DISCONNECTED_EVT:
        {
		  printf("Ble server disconnected event!\n");
		  break;
        }
        case BLE_SE_READ_EVT:
        {
		int i = 0;
		  printf("BLE_SE_READ_EVT\n");



		  ble_callback_data = *(tBleCallbackData *)reply;
		  printf("server number %d\n", ble_callback_data.server_num);
		        printf("service number %d\n", ble_callback_data.service_num);
		        printf("server attr uuid %s\n", ble_callback_data.attr_uuid);

      for (i = 0; i < ble_callback_data.len; i++)
		  {
			  if (i%16 == 0)
			  {
			      printf("\t");
			  }
			  printf("%2x ", ble_callback_data.value[i]);
		  }
		  printf("\n");

		  if (strcasecmp(ble_callback_data.attr_uuid, BLE_CHAR_UUID) == 0){
		      c.ble_server_send_read_rsp(reply, read_char_rsp, 4);
		      //c.ble_server_send_read_rsp(reply, rsp_data, 256);
		  } else if (strcasecmp(ble_callback_data.attr_uuid, BLE_CHAR1_UUID) == 0) {
		      c.ble_server_send_read_rsp(reply, read_char1_rsp, 4);
		  }

		  break;
        }

        case BLE_SE_WRITE_EVT:
	      {
		  int i = 0;
		  printf("BLE_SE_WRITE_EVT\n");

		  ble_callback_data = *(tBleCallbackData *)reply;
		  printf("server number %d\n", ble_callback_data.server_num);
		  printf("service number %d\n", ble_callback_data.service_num);
		  printf("server attr uuid %s\n", ble_callback_data.attr_uuid);
		  printf("Write value len %d\n", ble_callback_data.len);
		  printf("Write value: \n");

		  for (i = 0; i < ble_callback_data.len; i++)
		  {
			  if (i%16 == 0)
			  {
			      printf("\t");
			  }
			  printf("%2x ", ble_callback_data.value[i]);
		  }
		  printf("\n");

      if ((strcasecmp(ble_callback_data.attr_uuid, BLE_DESC_UUID) == 0)
	  && (ble_callback_data.len == 2)) {
          if ((ble_callback_data.value[0] == 1)
		  && (ble_callback_data.value[1] == 0)){
	      client_read_flag = 1;
	  }

	  if ((ble_callback_data.value[0] == 0)
		  && (ble_callback_data.value[1] == 0)){
	      client_read_flag = 0;
	  }
      }

		  break;
        }
        default:
	          break;
	  }

}


int main(int argc, char *args[]){
    unsigned short server_uuid = 0;
    const char *service_uuid = NULL, *char_uuid = NULL, *desc_uuid = NULL;
    int char_attr_num = -1, desc_attr_num = -1;
    int server_num = -1, service_num = -1;
    const char *srvc_uuid_array = NULL;
    unsigned char ble_adv_value[6] = {0x2b, 0x1a, 0xaa, 0xbb, 0xcc, 0xdd}; /*First 2 byte is Company Identifier Eg: 0x1a2b refers to Bluetooth ORG, followed by 4bytes of data*/
    unsigned short appearance_data = 0;
    int count = 0;
    unsigned char data[10] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0xAA, 0XFF};

    int desc1_attr_num = -1, desc2_attr_num = -1;

    printf("bt off before on\n");
    c.bt_off();

    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL);
    }

    c.set_bt_name("ble test001");

    /* ble test */
    server_uuid = BLE_SERVER_UUID;
    printf("Register server uuid 0x%x\n", server_uuid);
    server_num = c.ble_server_register(server_uuid, ble_event_f);

    service_uuid = BLE_SERVICE_UUID;
    printf("Create service uuid %s\n", service_uuid);
    service_num = c.ble_server_create_service(server_num, service_uuid, 6, 1);

    char_uuid = BLE_CHAR_UUID;
    printf("Add characteristic uuid %s\n", char_uuid);
    char_attr_num = c.ble_server_add_char(server_num, service_num, char_uuid, 0, 0x11, 0x3A);

    desc_uuid = BLE_DESC_UUID;
    printf("Add descriptor uuid %s\n", desc_uuid);
    desc_attr_num = c.ble_server_add_char(server_num, service_num, desc_uuid, 1, 0x11, 0);

    char_uuid = BLE_CHAR1_UUID;
    printf("Add characteristic1 uuid %s\n", char_uuid);
    char_attr_num = c.ble_server_add_char(server_num, service_num, char_uuid, 0, 0x11, 0x3A);

    desc_uuid = BLE_DESC_UUID;
    printf("Add descriptor uuid %s\n", desc_uuid);
    desc_attr_num = c.ble_server_add_char(server_num, service_num, desc_uuid, 1, 0x11, 0);

    desc1_attr_num = c.ble_server_add_char(server_num, service_num, BLE_DESC1_UUID, 1, 0x11, 0);
    desc2_attr_num = c.ble_server_add_char(server_num, service_num, BLE_DESC2_UUID, 1, 0x11, 0);

    printf("Start ble server\n");
    c.ble_server_start_service(server_num, service_num);

    printf("Config ble adv data\n");
    srvc_uuid_array = BLE_SERVICE_UUID;
    appearance_data = 0x1122;
    c.ble_server_config_adv_data(srvc_uuid_array, ble_adv_value, 6, appearance_data, 0);

    c.set_ble_discoverable(1);
    c.set_ble_connectable(1);

    c.set_dev_discoverable(0);
    c.set_dev_connectable(0);


    while(1) {
        if (client_read_flag == 1) {
	      c.ble_server_send_data(server_num, char_attr_num, (data+count), 1);
	      count++;
	      count = count%10;
            usleep(10000*1000);
	  }

	  usleep(2000*1000);
    }
}
