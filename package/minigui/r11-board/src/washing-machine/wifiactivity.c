#include "wifiactivity.h"
#include "WifiWindow.h"
#include "light_menu.h"
#include "headbar_view.h"

//int event;
int inner_event;
int event_label = 0;
extern int pthread_stop_refreash;

static void wifi_event_handle(struct Manager *w, int event_label)
{
    wmg_printf(MSG_DEBUG,"event_label 0x%x\n", event_label);

    switch(w->StaEvt.state)
    {
                 case CONNECTING:
                 {
                         wmg_printf(MSG_INFO,"Connecting to the network......\n");
			 event = CONNECTING;
                         break;
                 }
                 case CONNECTED:
                 {
                         wmg_printf(MSG_INFO,"Connected to the AP\n");
			 event = CONNECTED;
                         start_udhcpc();
                         break;
                 }

                 case OBTAINING_IP:
                 {
                         wmg_printf(MSG_INFO,"Getting ip address......\n");
			 event = OBTAINING_IP;
                         break;
                 }

                 case NETWORK_CONNECTED:
                 {
                         wmg_printf(MSG_DEBUG,"Successful network connection\n");
			 event = NETWORK_CONNECTED;
                         break;
                 }
                case DISCONNECTED:
                {
                    wmg_printf(MSG_ERROR,"wifi Disconnected,the reason:%s\n",wmg_event_txt(w->StaEvt.event));
			event = w->StaEvt.event;
			bool_connect = -1;
			sleep(1);
			if(GetWifiWnd() != 0){
				printf("GetWifiWnd() != 0\n");
				pthread_stop_refreash = STATUS_OFF;
				ROTATE_STATUS = STATUS_OFF;
				pthread_cond_signal(&cond);
				SendMessage(GetWifiWnd(), MSG_REFASH_ITEM, NULL, NULL);
				EnableWindow(switch_button, TRUE);
				EnableWindow(GetContainerWnd(), TRUE);
			}
                    break;
                }
    }
}
int check_password(const char *passwd)
{
    int result = 0;
    int i=0;

    if(!passwd || *passwd =='\0'){
        return -1;
    }

    if(strlen(passwd) < 8 || strlen(passwd) > 64 ){
        wmg_printf(MSG_ERROR,"ERROR:passwd less than 8 or longer than 64");
        return -1;
    }

    for(i=0; passwd[i]!='\0'; i++){
        /* non printable char */
        if((passwd[i]<32) || (passwd[i] > 126)){
		wmg_printf(MSG_INFO,"ERROR:passwd include unprintable char");
		result = -1;
		break;
        }
    }

	return result;
}
static char *sto_utf8(const char *buf, char *dst)
{
        char buf_temp[20148] = {0};
        int x = 0;
        unsigned long i;
        while(*buf != '\0')
        {
                if(*buf == '\\')
                {
                        strcpy(buf_temp,buf);
                        *buf_temp = '0';
                        *(buf_temp + 4) = '\0';
                        i = strtoul(buf_temp, NULL, 16);
                        dst[x] = i;
                        buf += 3;
                }
                else
                {
                        dst[x] = *buf;
                }
                x++;
                buf++;


        }
	dst[x] = '\0';
        return dst;
}
static int set_log_level(int argv, char *argc[])
{
	if(argv >=2 && !strncmp(argc[1],"d",1)){
		char *debug = argc[1];
		printf("debug[1] = %c\n",debug[1]);
		if(strlen(debug) >=2 && debug[1] >= '0' &&
			debug[1] <= '9'){
			wmg_set_debug_level(debug[1] - '0');
			return 0;
		}else{
			printf("Illegal level\n");
			printf("Level range 0~5\n");
			return -1;
		}
	}
	return 0;
}
char *get_str_pos(char *s,char c,int n)
{
	int i = 0;
	char *obj_ptr = NULL;
	if(NULL == s)
		return NULL;
	while((obj_ptr=strchr(s,c)) != NULL){
		s=++obj_ptr;
		i++;
		if(i == n)
			return obj_ptr;
	}
	return NULL;
}

int get_scan_info(char *scan_results)
{
	int i = 0,j = 0;
	char *current_ptr = NULL;
	char *obj_ptr = NULL;
	char stohex_ssid[128] = {0x00};
	char level[4];
	int n_count = 0;
	memset(scan_info_arry, '\0', sizeof(scan_info_arry));
	memset(wifi_name_array, '\0', sizeof(wifi_name_array));
	if(NULL == scan_results){
		return -1;
	}

	if(strncmp(scan_results,"bssid",5)){
		wmg_printf(MSG_ERROR,"Incoming scan_results parameter error\n");
		return -1;
	}
	obj_ptr = scan_results;
	while(NULL != (current_ptr=strchr(obj_ptr,'\n'))){
		obj_ptr = current_ptr+1;
		n_count++;
	}
	if(n_count == 1)
		return 0;
	while(NULL != (current_ptr=strsep(&scan_results,"\n"))){
		wmg_printf(MSG_MSGDUMP,"scan_results=%s\n current_ptr=%s\n",scan_results,current_ptr);
		if(!strncmp(current_ptr,"bssid",5))
			continue;
		if(*current_ptr == '\0')
			break;
		obj_ptr = get_str_pos(current_ptr,'\t',2);
		if(obj_ptr != NULL){
			strncpy(level,obj_ptr+1,2);
			scan_info_arry[i].level = 100 - atoi(level);
		}
		obj_ptr = get_str_pos(current_ptr,'\t',4);
		if(obj_ptr != NULL){
			/*change_utf8*/
			sto_utf8(obj_ptr, stohex_ssid);
			strncpy(wifi_name_array[i].wifi_name,stohex_ssid,strlen(obj_ptr));
		}
		i++;
	}
	scan_number = i;
#if 1
/*remove duplicates*/
        for(i = 0; i < scan_number; i++){
                for(j = i+1; j < scan_number; j++){
                        if(strcmp(wifi_name_array[i].wifi_name, wifi_name_array[j].wifi_name) == 0){
                                memset(wifi_name_array[j].wifi_name, '\0', strlen(wifi_name_array[j].wifi_name));
                        }
                }
        }
	for(i = 0; i< scan_number; ++i){
		printf("wifi_name_array[%d].wifi_name=[%s]\n", i, wifi_name_array[i].wifi_name);
	}
/*Remove empty string*/
        j = 0;
        for(i = 0; i < scan_number; ++i){
                if(strlen(wifi_name_array[i].wifi_name) != 0){
                        strncpy(scan_info_arry[j].ssid, wifi_name_array[i].wifi_name, strlen(wifi_name_array[i].wifi_name));
                        j++;
                }
        }
	for(i = 0; i< scan_number; ++i){
                printf("scan_info_arry[%d].ssid=%s\n", i, scan_info_arry[i].ssid);
        }
        scan_number = j;
#endif
	return scan_number;
}

int wifi_on(const aw_wifi_interface_t *p_wifi_interface)
{
	p_wifi_interface = aw_wifi_on(wifi_event_handle, event_label);

	if(p_wifi_interface == NULL){
		wmg_printf(MSG_ERROR,"wifi on failed event 0x%x\n", event);
		return -1;
	}
	return 0;
}

void wifi_off(const aw_wifi_interface_t *p_wifi_interface)
{
	if(p_wifi_interface != NULL){
		aw_wifi_off(p_wifi_interface);
	}else{
	}
}
char *scan_waln(const aw_wifi_interface_t *p_wifi_interface)
{
    int ret = 0, i = 0;
    int len = 4096;
    int times = 0;
    char ssid[256] = {0}, scan_results[4096] = {0}, reply[4069]= {0};
    event_label = rand();
	printf("scan_waln scan_waln\n");
	if(p_wifi_interface == NULL){
        wmg_printf(MSG_ERROR,"wifi on failed event 0x%x\n", event);
        return ;
    }
	printf("p_wifi_interface->get_scan_results before\n");
        ret=( p_wifi_interface->get_scan_results(scan_results, &len));
        wmg_printf(MSG_DEBUG,"ret of get_scan_results is %d\n", ret);
    if(ret==0)
    {
	wmg_printf(MSG_INFO,"%s\n",scan_results);
	scan_number = get_scan_info(scan_results);
    }
    else
    {
	wmg_printf(MSG_ERROR,"Get_scan_results failed!\n");
    }
    return scan_info_arry;
}

int wlan_connect(const aw_wifi_interface_t *p_wifi_interface,char *ssid, char *password,int level,int event_label){
    int ret = 0, len = 0;
        struct wpa_status * sta;
        if(p_wifi_interface == NULL){
                wmg_printf(MSG_ERROR,"p_wifi_interface is NULL\n");
                return -1;
        }
        if(ssid == NULL || password == NULL){
                wmg_printf(MSG_ERROR,"ssid or password is NULL\n");
		bool_connect = -1;
		event = WSE_PASSWORD_INCORRECT;
		sleep(2);
                return -1;
        }
        if(check_password(password) == -1){
		bool_connect = -1;
		event = WSE_PASSWORD_INCORRECT;
		sleep(2);
                return -1;
	}

        if(level >=0 && level <=5)
                wmg_set_debug_level(level);
	event_label++;
        if(!strncmp(password,"NONE",4))
                p_wifi_interface->add_network(ssid,WIFIMG_NONE,NULL,event_label);
        else{
		printf("p_wifi_interface->connect_ap\n");
                p_wifi_interface->connect_ap(ssid,password, event_label);

	}
	if(aw_wifi_get_wifi_state()== NETWORK_CONNECTED){
		wmg_printf(MSG_INFO,"Wifi connect ap : Success!\n");
		event = CONNECTED;
                return 0;
        }
        else{
		event = w->StaEvt.event;
                return -1;
        }
    return -1;
}

int wlan_disconnect(const aw_wifi_interface_t *p_wifi_interface, int event_label)
{
	p_wifi_interface->disconnect_ap(event_label);
	return 0;
}
