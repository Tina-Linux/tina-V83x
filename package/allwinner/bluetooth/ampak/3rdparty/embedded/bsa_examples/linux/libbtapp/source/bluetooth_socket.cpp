#include "bluetooth_socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


extern "C"
{
#include "app_disc.h"
#include "bsa_api.h"
#include "app_utils.h"
#include "bluetooth_interface.h"
#include "bluetooth_params.h"
}

c_bt::c_bt(){
    bt_wd[0] = '.';
    bt_wd[1] = '\0';
    bt_on_status = 0;
    bluetooth_init();
}

c_bt::~c_bt(){

}

int c_bt::bt_on(char *bt_addr){

    char cmd[512] = {0};
    struct bluetooth_params bt_params={0};

    printf("do bt cmd bt on bt_addr path %s\n", bt_addr);
    if(this->bt_on_status == 1){
        printf("bt already on\n");
        return 0;
    }

#ifdef SYSTEM_INIT_PROCD
    /* get bt config */
    get_params_from_config("/etc/config/aw_bluetooth", &bt_params);
#endif
    /* start bt server */
	if (bt_params.device[0] == '\0') {
	printf("param is NULL, /etc/bluetooth/btenable.sh on /dev/ttyS1\n");
	strncpy(cmd, "/etc/bluetooth/btenable.sh on /dev/ttyS1", 512);
	}
	else {
	printf("/etc/bluetooth/btenable.sh on %s\n", bt_params.device);
	snprintf(cmd, 512, "/etc/bluetooth/btenable.sh on %s", bt_params.device);
	}
    system(cmd);
    usleep(1000000);

    /*start bluetooth app*/
    bluetooth_start(this,bt_addr);

#if (BT_AVK_ENABLE == 1)
    /*start app avk*/
    start_app_avk();
#endif

#if (BT_HS_ENABLE == 1)
    /* start hs */
    start_app_hs();
#endif

#if (BT_BLE_ENABLE == 1)
    /* stop app ble */
    start_app_ble();
#endif

    this->bt_on_status = 1;
    return 0;
}

int c_bt::bt_on_no_avrcp(char *bt_addr){

    char cmd[512] = {0};

    printf("do bt cmd bt on bt_addr path %s\n", bt_addr);
    if(this->bt_on_status == 1){
        printf("bt already on\n");
        return 0;
    }

    /* start bt server */
    snprintf(cmd, 512, "/etc/bluetooth/btenable.sh on");
    system(cmd);
    usleep(1000000);

    /*start bluetooth app*/
    bluetooth_start(this,bt_addr);

#if (BT_AVK_ENABLE == 1)
    /*start app avk*/
    start_app_avk_no_avrcp();
#endif

    this->bt_on_status = 1;
    return 0;
}

int c_bt::bt_off(){
    printf("do bt cmd bt off\n");

    if(this->bt_on_status == 0){
        printf("bt is not on\n");
        return 0;
    }

#if (BT_HS_ENABLE == 1)
    /* stop app hs */
    stop_app_hs();
#endif

#if (BT_AVK_ENABLE == 1)
    /*stop app avk*/
    stop_app_avk();
#endif

#if (BT_BLE_ENABLE == 1)
    /* stop app ble */
    stop_app_ble();
#endif

    /* stop bluetooth app */
    bluetooth_stop();
    usleep(1000*1000);

    /* stop bt server */
    system("/etc/bluetooth/btenable.sh off");
    usleep(500*1000);
    this->bt_on_status = 0;
    return 0;
}

int c_bt::set_bt_name(const char *name)
{
    if(!name || !name[0]){
        printf("Error: set bt name is NULL!\n");
        return -1;
    }

    printf("do bt cmd set bt name %s\n", name);

    s_set_bt_name(name);
    return 0;
}

int c_bt::get_bd_addr(unsigned char bd_addr[])
{
    if(NULL == bd_addr)
	return -1;
    return (s_get_bd_addr(bd_addr));
}

int c_bt::set_dev_discoverable(int enable)
{
    printf("do bt cmd set dev discoverable %d\n", enable);
    s_set_discoverable(enable);
    return 0;
}

int c_bt::set_dev_connectable(int enable)
{
    printf("do bt cmd set dev connectable %d\n", enable);
    s_set_connectable(enable);
    return 0;
}

int c_bt::start_discovery(int time)
{
    printf("do bt cmd start discovery\n");
    s_start_discovery(time);
    return 0;
}

int c_bt::get_disc_results(char *disc_results, int *len)
{
    printf("do bt cmd get discover results\n");
    return s_get_disc_results(disc_results, len);
}

int c_bt::connect_auto()
{
    printf("do bt cmd connect_auto\n");

    return s_connect_auto();
}

int c_bt::connect_dev_by_addr(BT_ADDR bt_addr)
{
    int i = 0;
    S_BT_ADDR s_bt_addr;

    printf("do bt cmd connect dev by addr %02X:%02X:%02X:%02X:%02X:%02X\n",
                    bt_addr[0], bt_addr[1], bt_addr[2],
                    bt_addr[3], bt_addr[4], bt_addr[5]);

    for(i = 0; i < BT_ADDR_LEN; i++){
        s_bt_addr[i] = bt_addr[i];
    }
    s_connect_dev_by_addr(s_bt_addr);
    return 0;
}

int c_bt::disconnect()
{
    printf("do bt cmd disconnect\n");
    s_disconnect();
    return 0;
}

int c_bt::reset_avk_status()
{
    printf("do bt cmd reset avk status\n");
    if(this->bt_on_status == 0){
        printf("bt status is off\n");
        return 0;
    }

    /* stop app avk */
    stop_app_avk();
    usleep(200*1000);

    /* start app avk */
    start_app_avk();
    usleep(200*1000);
    return 0;
}

int c_bt::avk_play()
{
    printf("do bt cmd avk play\n");

    s_avk_play();
    return 0;
}

int c_bt::avk_pause()
{
    printf("do bt cmd avk pause\n");
    s_avk_pause();
    return 0;
}

int c_bt::avk_stop()
{
    printf("do bt cmd avk stop\n");
    s_avk_stop();
    return 0;
}

int c_bt::avk_close_pcm_alsa()
{
    printf("do bt cmd avk pcm alsa\n");
    s_avk_close_pcm_alsa();
    return 0;
}

int c_bt::avk_resume_pcm_alsa()
{
    printf("do bt cmd avk resume pcm alsa\n");
    s_avk_resume_pcm_alsa();
    return 0;
}

int c_bt::avk_previous()
{
    printf("do bt cmd avk previous\n");
    s_avk_play_previous();
    return 0;
}

int c_bt::avk_next()
{
    printf("do bt cmd avk next\n");
    s_avk_play_next();
    return 0;
}

int c_bt::avk_set_volume_up()
{
	  printf("do bt cmd set volume up\n");
	  s_set_volume_up();
	  return 0;
}

int c_bt::avk_set_volume_down()
{
	  printf("do bt cmd set volume down\n");
	  s_set_volume_down();
	  return 0;
}

int c_bt::avk_get_music_info(tBT_AVK_MUSIC_INFO *p_avk_music_info)
{
    s_avk_element_attr_t s_avk_element_attr;

    printf("do bt cmd avk get music info\n");

    if(!p_avk_music_info){
	printf("Error: avk music info is NULL!\n");
	return -1;
    }
    s_avk_get_element_attr(&s_avk_element_attr);

    memcpy(p_avk_music_info->title, s_avk_element_attr.title, AVK_MUSIC_INFO_LEN_MAX);
    memcpy(p_avk_music_info->artist, s_avk_element_attr.artist, AVK_MUSIC_INFO_LEN_MAX);
    memcpy(p_avk_music_info->album, s_avk_element_attr.album, AVK_MUSIC_INFO_LEN_MAX);
    //memcpy(p_avk_music_info->playing_time, s_avk_element_attr.playing_time, AVK_MUSIC_INFO_LEN_MAX);
    return 0;
}

int c_bt::hs_pick_up()
{
    s_hs_pick_up();
    return 0;
}

int c_bt::hs_hung_up()
{
    s_hs_hung_up();
    return 0;
}

void c_bt::set_callback(tBtCallback *pCb)
{
    pBtCb = pCb;
}

void c_bt::event_callback(BT_EVENT bt_event, void *reply, int *len)
{
    printf("received bt event 0x%x\n", bt_event);
    if(pBtCb){
        pBtCb(bt_event, reply, len);
    }
}

void c_bt::do_test()
{
    ;
}

int c_bt::initial_disc_param()
{
    s_initial_disc_param();
    return 0;
}

int c_bt::set_discovery_callback(void *u_custom_disc_cback)
{
	return (s_set_discovery_callback((tBT_DISC_CBACK *)u_custom_disc_cback));
}

int c_bt::set_discovery_bd_addr(unsigned char bt_addr[])
{
	return (s_set_discovery_bd_addr(bt_addr));
}

int c_bt::set_discovery_name(char bt_name[])
{
    return (s_set_discovery_name(bt_name));
}

int c_bt::start_discovery_with_get_rssi()
{
    s_start_discovery_with_get_rssi();
    return 0;
}

int c_bt::get_disc_results_with_rssi(char *disc_results, int *len)
{
    return (s_get_disc_results_with_rssi(disc_results, len));
}

int c_bt::convert_str_to_ad(unsigned char bd_ad[], char *pts)
{
    int i = 0, j = 0;
    unsigned char var, var_lo = 0, var_hg = 0;

    if(strlen(pts) != 17)
    {
        printf("The input address is not suitable!\n");
        return -1;
    }

    for(i = 0; *(pts+i) != '\0'; i++)
    {
        if(*(pts+i) == ':')
	{
	    if( i%3 == 2)
                continue;
	    else
	    {
		printf("convert_str_to_ad: The format of the address is wrong, i:%d!\n", i);
		return -1;
	    }
	}

        if((*(pts+i) >= '0') && (*(pts+i) <= '9'))
        {
                var = *(pts+i) - 48;
        }
        else if((*(pts+i) >= 'A') && (*(pts+i) <= 'F'))
                var = *(pts+i) - 55;
        else if((*(pts+i) >= 'a') && (*(pts+i) <= 'f'))
                var = *(pts+i) - 87;
        else
        {
                printf("ERROR: wrong charactor in address:%c\n",*(pts+i));
		return -1;
        }

        if(i%3 == 0)
        {
            var_hg = var;
        }
        else if(i%3 == 1)
        {
            var_lo = var;
            var = (unsigned char)((int)var_hg*16+(int)var_lo);
            j = i/3;
            bd_ad[j] = (unsigned char)var;
        }
    }
    printf("Get the bd_address:%02x:%02x:%02x:%02x:%02x:%02x!\n", bd_ad[0], bd_ad[1], bd_ad[2], bd_ad[3], bd_ad[4], bd_ad[5]);

    return 0;
}

int c_bt::ble_server_register(unsigned short uuid, tBleCallback *p_cback)
{
	  int server_num = -1;
	  printf("do bt cmd ble server register\n");

	  server_num = s_ble_server_register(uuid, NULL);
	  pBleCb = p_cback;
	  return server_num;
}

int c_bt::ble_server_deregister(int server_num)
{
	  printf("do bt cmd ble server deregister\n");
	  return s_ble_server_deregister(server_num);
}

int c_bt::ble_server_create_service(int server_num, const char *service_uuid128, int char_num, int is_primary)
{
	  int service_num = -1;
	  printf("do bt cmd ble server create service\n");

	  service_num = s_ble_server_create_service(server_num, service_uuid128, char_num, is_primary);
	  return service_num;
}

int c_bt::ble_server_add_char(int server_num, int service_num, const char *char_uuid128,
    int is_descript, unsigned short attribute_permission, unsigned short characteristic_property)
{
	  int ret = -1;

	  printf("do bt cmd ble server add char\n");
	  ret = s_ble_server_add_char(server_num, service_num, char_uuid128,
	      is_descript, attribute_permission, characteristic_property);

	  return ret;

}

int c_bt::ble_server_start_service(int server_num, int srvc_attr_num)
{
    int ret = -1;

    printf("do bt cmd ble server start service\n");
    ret = s_ble_server_start_service(server_num, srvc_attr_num);

    return ret;
}

int c_bt::ble_server_config_adv_data(const char *srvc_uuid128_array,
        unsigned char app_ble_adv_value[], int data_len,
        unsigned short appearance_data, int is_scan_rsp)
{
	  int ret = -1;

	  printf("do bt cmd ble server config adv data\n");
	  ret = s_ble_server_config_adv_data(srvc_uuid128_array,
        app_ble_adv_value, data_len,
        appearance_data, is_scan_rsp);

    return ret;
}

int c_bt::ble_server_send_read_rsp(void *p_cb_reply, unsigned char data[], int data_len)
{
	  int ret = -1;

	  printf("do bt cmd ble server send read rsp\n");
	  ret = s_ble_server_send_read_rsp(p_cb_reply, data, data_len);

	  return ret;
}

int c_bt::ble_server_send_data(int server_num, int attr_num,
        unsigned char data[], int data_len)
{
	  int ret = -1;

	  printf("do bt cmd ble server send data\n");
	  ret = s_ble_server_send_indication(server_num, attr_num, data, data_len);

	  return ret;
}

int c_bt::set_ble_discoverable(int enable)
{
    printf("do bt cmd set ble discoverable %d\n", enable);
    s_set_ble_discoverable(enable);
    return 0;
}

int c_bt::set_ble_connectable(int enable)
{
    printf("do bt cmd set ble connectable %d\n", enable);
    s_set_ble_connectable(enable);
    return 0;
}

void c_bt::ble_event_callback(BLE_EVENT ble_event, void *reply, int *len)
{
    printf("received ble event 0x%x\n", ble_event);
    if(pBleCb){
        pBleCb(ble_event, reply, len);
    }
}


extern "C" void bt_event_transact(void *p, APP_BT_EVENT event, void *reply, int *len)
{
    c_bt *p_c_bt = (c_bt *)p;
    switch(event)
    {
          case APP_AVK_CONNECTED_EVT:
          {
              p_c_bt->event_callback(BT_AVK_CONNECTED_EVT, NULL, NULL);
              break;
          }

          case APP_AVK_DISCONNECTED_EVT:
          {
              p_c_bt->event_callback(BT_AVK_DISCONNECTED_EVT, reply, len);
              break;
          }

        case APP_AVK_CONNECT_COMPLETED_EVT:
        {
            p_c_bt->event_callback(BT_AVK_CONNECT_COMPLETED_EVT, NULL, NULL);
            break;
        }

        case APP_AVK_START_EVT:
        {
            p_c_bt->event_callback(BT_AVK_START_EVT, NULL, NULL);
            break;
        }

        case APP_AVK_STOP_EVT:
        {
            p_c_bt->event_callback(BT_AVK_STOP_EVT, NULL, NULL);
            break;
        }

	case APP_HS_CONNECTED_EVT:
	{
	    p_c_bt->event_callback(BT_HS_CONNECTED_EVT, NULL, NULL);
	    break;
	}

	case APP_HS_DISCONNECTED_EVT:
	{
	    p_c_bt->event_callback(BT_HS_DISCONNECTED_EVT, NULL, NULL);
	    break;
	}

	case APP_HS_AUDIO_OPEN_EVT:
	{
	    p_c_bt->event_callback(BT_HS_RING_EVT, NULL, NULL);
	    break;
	}

	case APP_HS_AUDIO_CLOSE_EVT:
	{
	    p_c_bt->event_callback(BT_HS_OK_EVT, NULL, NULL);
	    break;
	}

	case APP_HS_RING_EVT:
	{
	    p_c_bt->event_callback(BT_HS_RING_EVT, NULL, NULL);
	}
	default:
                ;
    }
}

extern "C" void ble_event_transact(void *p, APP_BLE_EVENT event, void *reply, int *len)
{
    c_bt *p_c_bt = (c_bt *)p;
    s_tBleCallbackData *p_s_ble;
    tBleCallbackData p_ble;


    switch(event)
    {
          case APP_BLE_SE_CONNECTED_EVT:
          {
		  p_c_bt->ble_event_callback(BLE_SE_CONNECTED_EVT, reply, len);
              break;
          }

          case APP_BLE_SE_DISCONNECTED_EVT:
          {
		  p_c_bt->ble_event_callback(BLE_SE_DISCONNECTED_EVT, reply, len);
              break;
          }

          case APP_BLE_SE_READ_EVT:
          {
		  p_s_ble = (s_tBleCallbackData *)reply;

		  p_ble.server_num = p_s_ble->server_num;
		  p_ble.service_num = p_s_ble->service_num;
		  p_ble.char_attr_num = p_s_ble->char_attr_num;
		  p_ble.attr_uuid = p_s_ble->attr_uuid128;
		  p_ble.value = p_s_ble->value;
		  p_ble.len = p_s_ble->len;
		  p_c_bt->ble_event_callback(BLE_SE_READ_EVT, (void *)&p_ble, len);
		  break;
          }

          case APP_BLE_SE_WRITE_EVT:
          {
		  p_s_ble = (s_tBleCallbackData *)reply;

		  p_ble.server_num = p_s_ble->server_num;
		  p_ble.service_num = p_s_ble->service_num;
		  p_ble.char_attr_num = p_s_ble->char_attr_num;
		  p_ble.attr_uuid = p_s_ble->attr_uuid128;
		  p_ble.value = p_s_ble->value;
		  p_ble.len = p_s_ble->len;
              p_c_bt->ble_event_callback(BLE_SE_WRITE_EVT, (void *)&p_ble, len);
              break;
          }

          default:
          ;
    }
}
