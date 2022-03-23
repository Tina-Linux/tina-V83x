#include <bluetooth_socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG_MUSIC_INFO 0

#define BLE_SERVER_UUID        0x9999
#define BLE_SERVICE_UUID       "0000180a-0000-1000-8000-00805f9b34fb"
#define BLE_CHAR_UUID          "00009999-0000-1000-8000-00805f9b34fb"
#define BLE_DESC_UUID          "00002902-0000-1000-8000-00805f9b34fb"

static int last_status = 0;
static int status = 0;
static int playing = 0;
c_bt c;
static int client_read_flag = 0;

void bt_event_f(BT_EVENT event, void *reply, int *len)
{
    switch(event)
    {
	  case BT_AVK_CONNECTED_EVT:
	  {
		  printf("Media audio connected!\n");
		  status = 1;
		  break;
	  }

	  case BT_AVK_DISCONNECTED_EVT:
	  {
		  printf("Media audio disconnected!\n");
          printf("link down reason %d\n", *(int *)reply);
		  status = 0;
		  break;
	  }

	  case BT_AVK_START_EVT:
	  {
		  printf("Media start playing!\n");
                  playing = 1;
		  break;
	  }

	  case BT_AVK_STOP_EVT:
	  {
	      printf("Media stop playing!\n");
              playing = 0;
	      break;
	  }

	  case BT_HS_CONNECTED_EVT:
	  {
	      printf("HS and HF connected!\n");
	      break;
	  }

	  case BT_HS_DISCONNECTED_EVT:
	  {
	      printf("HS and HF disconnected!\n");
	      break;
	  }

	  case BT_HS_RING_EVT:
	  {
	      printf("HS ring call!\n");
	      break;
	  }

	  case BT_HS_OK_EVT:
	  {
	      printf("HS OK evt!\n");
	      break;
	  }
	  default:
	      break;
    }
}

void ble_event_f(BLE_EVENT event, void *reply, int *len)
{
    tBleCallbackData ble_callback_data;

    switch(event)
    {
        case BLE_SE_WRITE_EVT:
	      {
		  int i = 0;
		  printf("BLE_SE_WRITE_EVT\n");

		  ble_callback_data = *(tBleCallbackData *)reply;
		  printf("server number %d\n", ble_callback_data.server_num);
		  printf("service number %d\n", ble_callback_data.service_num);
		  printf("server attr uuid 0x%s\n", ble_callback_data.attr_uuid);
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
    int times = 0;
    int save_fd = -1, fd = -1;
    int ret = 0;
    char bd_addr[6]={0};
    tBT_AVK_MUSIC_INFO music_info;

    unsigned short server_uuid = 0;
    const char *service_uuid = NULL, *char_uuid = NULL, *desc_uuid = NULL;
    int char_attr_num = -1, desc_attr_num = -1;
    int server_num = -1, service_num = -1;
    const char *srvc_uuid_array = NULL;
    unsigned char ble_adv_value[6] = {0x2b, 0x1a, 0xaa, 0xbb, 0xcc, 0xdd}; /*First 2 byte is Company Identifier Eg: 0x1a2b refers to Bluetooth ORG, followed by 4bytes of data*/
    unsigned short appearance_data = 0;
    int count = 0;
    unsigned char data[10] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0xAA, 0XFF};

	c.set_callback(bt_event_f);

    printf("bt off before on\n");
    c.bt_off();

    last_status = 0;
    status = 0;
    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL);
    }

    c.set_bt_name("aw bt ble test001");

       /* ble test */
    server_uuid = BLE_SERVER_UUID;
    printf("Register server uuid 0x%x\n", server_uuid);
    server_num = c.ble_server_register(server_uuid, ble_event_f);

    service_uuid = BLE_SERVICE_UUID;
    printf("Create service uuid %s\n", service_uuid);
    service_num = c.ble_server_create_service(server_num, service_uuid, 1, 1);

    char_uuid = BLE_CHAR_UUID;
    printf("Add characteristic uuid %s\n", char_uuid);
    char_attr_num = c.ble_server_add_char(server_num, service_num, char_uuid, 0, 0x11, 0x3A);

    desc_uuid = BLE_DESC_UUID;
    printf("Add descriptor uuid %s\n", desc_uuid);
    desc_attr_num = c.ble_server_add_char(server_num, service_num, desc_uuid, 1, 0x11, 0);

    printf("Start ble server\n");
    c.ble_server_start_service(server_num, service_num);

    printf("Config ble adv data\n");
    srvc_uuid_array = BLE_SERVICE_UUID;
    appearance_data = 0x1122;
    c.ble_server_config_adv_data(srvc_uuid_array, ble_adv_value, 6, appearance_data, 0);

    c.set_ble_discoverable(1);
    c.set_ble_connectable(1);


    while(1){
	usleep(2000*1000);

	/* connected */
	if ((last_status == 0) && (status == 1)){
	    c.set_dev_discoverable(0);
	    c.set_dev_connectable(0);
	    last_status = 1;
	}

	/* disconnected */
	if ((last_status == 1) && (status == 0)){
	    c.set_dev_discoverable(1);
	    c.set_dev_connectable(1);
	    last_status = 0;
	}

#if(DEBUG_MUSIC_INFO == 1)
	if(playing == 1){
	    c.avk_get_music_info(&music_info);
	    printf("Title: %s\n", music_info.title);
	    printf("Artist: %s\n", music_info.artist);
	    printf("Album: %s\n", music_info.album);
	    //printf("Time: %s\n", music_info.playing_time);
	}
#endif

    if (client_read_flag == 1) {
	      c.ble_server_send_data(server_num, char_attr_num, (data+count), 1);
	      count++;
	      count = count%10;
            usleep(5000*1000);
	  }
    }
}
