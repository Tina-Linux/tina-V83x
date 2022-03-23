#include "ms_uart_process.h"

#include "ms_common.h"
#include "ms_net_app.h"
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


#define MS_DEV_SN_LENGTH 32
#define MAX_SN_REQUEST_TIMES 10
#define MAX_E1_REQUEST_TIMES 3
#define MAX_A0_REQUEST_TIMES 3
#define MAX_E2_REQUEST_TIMES 3
#define MAX_E3_REQUEST_TIMES 3
#define MAX_E5_REQUEST_TIMES 3
#define MS_FW_UART_BUF_LENGTH	300

typedef enum MS_THIRD_PARTY_INFO_TYPE_T
{
	ENUM_MS_THIRD_PARTY_INFO_TYPE_JD = 0X02,
	ENUM_MS_THIRD_PARTY_INFO_TYPE_HILINK,
}ms_third_party_info_type_t;

typedef enum MS_0X6A_CONFIG_INFO_T
{
	ENUM_MS_0X6A_CONFIG_INFO_NONE = 0X00,
	ENUM_MS_0X6A_CONFIG_INFO_SUCCESS,
	ENUM_MS_0X6A_CONFIG_INFO_ERROR,
}ms_0x6a_config_info_t;


/******************************************************************
				GLOBAL	VARABLE	 DEFINATE
*******************************************************************/
static uint8_t device_sn[33] = {0};//device sn, global varable.

ms_uart_sm_t uartStateMachine = MS_UART_SM_GET_SN;
uint8_t g_wifi_scanning_flag = false;

static uint8_t gs_dev_type = 0xFF;


static uint8_t g_uart_msg_id = 0;

static uint8_t g_sn_status = FAILD;
static uint8_t gs_E1_loop_cnt = 0;


static uint8_t gs_A0_loop_cnt = 0;

static bool gs_uart_wait_ack_flag = false;	//???éæ¤æ·éæ¤æ·???????éæ¤æ???

uint8_t g_SN_0x14[33] = {0};
bool g_sn_0x14_got_flag = false;
bool g_router_connect_flag_0x14 = false;
int g_serial_fd;

static bool	 gs_self_mode_flag = false;

ms_0x6a_config_info_t ms_0x6a_config_info = ENUM_MS_0X6A_CONFIG_INFO_NONE;
uint8_t ssid_len_0x6a = 0;
uint8_t pwd_len_0x6a = 0;
uint8_t ssid_0x6a[MS_SSID_MAX_LENGTH + 1/*'\0'*/];
uint8_t pwd_0x6a[MS_PWD_MAX_LENGTH + 1/*'\0'*/];

static T_EventFlag gstEventFlag;
static pthread_mutex_t gEventMutex;     //for protect gstEventFlag
static int gMutexInitFlag = 0;


/******************************************************************
				EXTERN	FUNCTION  REFERENCE
*******************************************************************/
extern int deamon_pid;
extern ms_stored_info_t ms_needed_stored_info;
extern int  ms_hal_hw_uart_write(int serialfd,uint8_t *buffer, int len );
extern void ms_get_module_version(unsigned char *p_in, int len);
extern ms_result_t ms_check_sn_is_valid(uint8_t *str , uint8_t len);
extern int ms_hal_read_msmart_config_info(const char *path, uint8_t *buf,int len);
extern ms_result_t ms_sn_decrypt(uint8_t *p_sn_in, int inlen, uint8_t *p_sn_out, int *poutlen);
extern ms_result_t ms_check_is_air_condition_sn(uint8_t *str , uint8_t *airSN);
extern status_t ms_hal_wlan_get_mac_address(uint8_t* macbuf);

//extern ms_hal_os_queue_t ms_uart_msg_queue;
status_t ms_valid_msg(uint8_t *data, uint16_t len);


/*
  fd:  0-NET; 1-LOCAL_NET; 2_SPEECH; 3-ALL
*/
static int _set_event_flag(int fd, const T_MsTransStream *pStream)
{
    unsigned char ucCmdFlag     = 0;
    unsigned char ucMsgIdentify   = 0;

    if (0 == gMutexInitFlag)
    {
        MS_TRACE("init gMutexInitFlag");
        pthread_mutex_init(&gEventMutex, NULL);
        gMutexInitFlag = 1;
    }

    status_t ret = ms_valid_msg((uint8_t *)pStream->data, pStream->size);
    if(OK != ret){
        MS_TRACE("MSG error ");
        return -1;
    }

    if ((pStream->data[9] == MS_UART_CTL_CMD) || (pStream->data[9] == MS_UART_QUERY_CMD))
    {
        ucMsgIdentify   = pStream->data[6];
        ucCmdFlag       = pStream->data[9];

        pthread_mutex_lock(&gEventMutex);
        switch (fd)
        {
            case 0:
                gstEventFlag.stNet.iCmdFlag      = ucCmdFlag;
                gstEventFlag.stNet.ucMsgIdentify = ucMsgIdentify;
                break;
            case 1:
                gstEventFlag.stLocalNet.iCmdFlag      = ucCmdFlag;
                gstEventFlag.stLocalNet.ucMsgIdentify = ucMsgIdentify;
                break;
            case 2:
                gstEventFlag.stSpeech.iCmdFlag      = ucCmdFlag;
                gstEventFlag.stSpeech.ucMsgIdentify = ucMsgIdentify;
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&gEventMutex);
        MS_TRACE("fd: %d, SET gstEventFlag: NET[0x%x 0x%x] LOCALNET[0x%x 0x%x] SPEECH[0x%x 0x%x]", fd, \
                  gstEventFlag.stNet.ucMsgIdentify, gstEventFlag.stNet.iCmdFlag, \
                  gstEventFlag.stLocalNet.ucMsgIdentify, gstEventFlag.stLocalNet.iCmdFlag, \
                  gstEventFlag.stSpeech.ucMsgIdentify, gstEventFlag.stSpeech.iCmdFlag);

        return 0;
    }
    else
    {
        MS_TRACE("not CTL or QUERY cmd");
    }

    return -1;
}

/*
  *pFd:  0-NET; 1-LOCAL_NET; 2_SPEECH; 3-ALL
*/
static int _ceck_event_flag(int *pFd, const T_MsTransStream *pStream)
{
    int fd = 3;

    if ((NULL == pFd) || (NULL == pStream))
    {
        MS_ERR_TRACE("invalid para");
        return -1;
    }

    if ((pStream->data[9] == MS_UART_CTL_CMD) || (pStream->data[9] == MS_UART_QUERY_CMD))
    {
        pthread_mutex_lock(&gEventMutex);
        if ((pStream->data[6] == gstEventFlag.stNet.ucMsgIdentify) && (pStream->data[9] == gstEventFlag.stNet.iCmdFlag))
        {
            fd = 0;
            gstEventFlag.stNet.ucMsgIdentify = 0;
            gstEventFlag.stNet.iCmdFlag      = 0;
        }
        else if ((pStream->data[6] == gstEventFlag.stLocalNet.ucMsgIdentify) && (pStream->data[9] == gstEventFlag.stLocalNet.iCmdFlag))
        {
            fd = 1;
            gstEventFlag.stLocalNet.ucMsgIdentify = 0;
            gstEventFlag.stLocalNet.iCmdFlag      = 0;
        }
        else if ((pStream->data[6] == gstEventFlag.stSpeech.ucMsgIdentify) && (pStream->data[9] == gstEventFlag.stSpeech.iCmdFlag))
        {
            fd = 2;
            gstEventFlag.stSpeech.ucMsgIdentify = 0;
            gstEventFlag.stSpeech.iCmdFlag      = 0;
        }
        else
        {
           MS_TRACE("[ERROR]not NET or localNet or Speech");
            fd = 3;
        }
        pthread_mutex_unlock(&gEventMutex);
        MS_TRACE("fd: %d, GET gstEventFlag: NET[0x%x 0x%x] LOCALNET[0x%x 0x%x] SPEECH[0x%x 0x%x]", fd, \
                  gstEventFlag.stNet.ucMsgIdentify, gstEventFlag.stNet.iCmdFlag, \
                  gstEventFlag.stLocalNet.ucMsgIdentify, gstEventFlag.stLocalNet.iCmdFlag, \
                  gstEventFlag.stSpeech.ucMsgIdentify, gstEventFlag.stSpeech.iCmdFlag);
    }
    else
    {
        MS_TRACE("not CTL or QUERY command");
        fd = 3;
    }

    *pFd = fd;

    return 0;
}


void ms_uart_0xE1_process(uint8_t *buff);
/******************************************************************
				FUNCTION	 IMPLEMENT
*******************************************************************/

static bool ms_sys_api_ticks_compare_result(uint32_t end_tick, uint32_t start_tick, uint32_t interval_tick)
{
        if(start_tick > end_tick)
        {
                //end_tick += 0xffffffff;
                return ((0XFFFFFFFF - start_tick + end_tick >= interval_tick) ? true : false);
        }
        else
        {
                return ((end_tick - start_tick >= interval_tick) ? true : false);
        }
}


#if defined(CONFIG_AUDIO)
static uint8_t ms_handle_audio_play(uint8_t *recv_buf){

	uint8_t len,num =0,count = 0,index = 0;
    int dynamicID = 0;

    char url[50] ;

    ms_audio_type_t type[MS_MAX_ONCE_PLAY_NUM];

    memset(url,0,50);
//    MS_TRACE("recv_buf LENGTH:%d",recv_buf[1]);
	if(recv_buf[9] == 0xCB){
        num = recv_buf[MS_AUDIO_NUM_INDEX];
        if(num >6){
	    MS_TRACE("ERROR!!! Invalid audio number");
	}else{
		if(recv_buf[MS_AUDIO_PLAY_FLAG] != DYNAMIC_UPDATE_PALY){

		for(count = 0;count <num;count++){
			if(recv_buf[MS_AUDIO_NUM_ID1+index] == 0){
				 MS_TRACE("ERROR!!! Invalid audio type");
			}
	        if(recv_buf[MS_AUDIO_NUM_ID1+index] == 0xFF){

			dynamicID = recv_buf[MS_AUDIO_NUM_ID1 + index + 3];

				MS_TRACE("dynamicID:%d",dynamicID);

				dynamicID = (dynamicID<<8) + recv_buf[MS_AUDIO_NUM_ID1 + index + 4];
//				MS_TRACE("dynamicID1:%d",dynamicID);
				type[count] = MS_AUDIO_MS_DYNAMIC_F4;

				index = index +4;

				continue;
		}
		if(((recv_buf[MS_AUDIO_NUM_ID1+index])>0)&&((recv_buf[MS_AUDIO_NUM_ID1+index]) <=(MS_AUDIO_TYPE_LAST+1))||((recv_buf[MS_AUDIO_NUM_ID1+index] == 0xF4))){

			type[count] = recv_buf[MS_AUDIO_NUM_ID1+index]-1;

		}else{

		    MS_TRACE("ERROR!!! Invalid audio type");

		}

			index++;

//			MS_TRACE("type[%d]:%d",count,type[count]);
	    }

		}else if(recv_buf[MS_AUDIO_PLAY_FLAG] == DYNAMIC_UPDATE_PALY){

			dynamicID = recv_buf[MS_AUDIO_NUM_ID1 + 2];

			dynamicID = (dynamicID<<8) + recv_buf[MS_AUDIO_NUM_ID1 + 3];
			type[0] = MS_AUDIO_MS_DYNAMIC_F4;
			num = 1;
			MS_TRACE("dynamicID:%d,ID3:%d,ID4:%d",dynamicID,recv_buf[MS_AUDIO_NUM_ID1 + 2],recv_buf[MS_AUDIO_NUM_ID1 + 3]);

	        }
			MS_TRACE("dynamicID3:%d",dynamicID);
	        if(dynamicID){

//		    snprintf(url,50,"http://172.20.10.2/voice/%d.mp3", dynamicID);
			snprintf(url,50,"http://dev-ce-cdn.midea.com/voice/%d.mp3", dynamicID);
//			snprintf(url,50,"http://ce5.midea.com/voice/test/%d.mp3", dynamicID);
//			snprintf(url,60,"http://10.33.181.108/%d.mp3", dynamicID);
			MS_TRACE("Begin update dynamic eara,url addr:%s",url);

	        ms_audio_start_ota(url);

	        }
	        ms_hal_os_thread_sleep(100);
	    switch(recv_buf[MS_AUDIO_PLAY_FLAG]){
	        case SINGLE_PLAY:
	        case MERGER_PLAY:
		    case DYNAMIC_UPDATE_PALY:
		     ms_audio_play(type,recv_buf[MS_AUDIO_PLAY_VOL],MS_AUDIO_MODE_ONE_TIMES,num);
		     break;
	        case STATIC_UPDATE:
//		     ms_audio_start_ota("http://172.20.10.2/MP3_FLASH.bin");
			 ms_audio_start_ota("http://dev-ce-cdn.midea.com/voice/MP3_FLASH.bin");
//			 snprintf(url,50,"http://ce5.midea.com/voice/test/%d.mp3", dynamicID);
//			 snprintf(url,60,"http://10.33.181.108/%d.mp3", dynamicID);
			 break;
	        default:
		     break;
	     }
	     if(recv_buf[MS_AUDIO_PLAY_STOP_FLAG] == 0){
		 ms_audio_play_stop(1);
	     }
	   }
        }
	return 0;
}
#endif


bool ms_sys_api_is_in_self_check_mode(void)
{
	return gs_self_mode_flag;
}

void ms_uart_wait_ack_cb(void *arg)
{
	gs_uart_wait_ack_flag = false;
}

void ms_fw_uart_debug_hex(char *info, uint8_t *data, uint16_t len)
{
	int i;
	printf("%s:", info);
	for(i = 0; i < len; i++)
	{
		printf("%02x ",data[i]);
	}
	printf("]\r\n");
}

uint8_t ms_fw_uart_flag_invalid_frame(uint8_t data)
{
	return (data == MS_MSMART_UART_HEAD)?0:1;
}

void printMsMsg(uint8_t *data, uint16_t len)
{
	int i = 0;
	for(i=0; i<len; i++)
	{
		printf("%02x", *(data+i));
	}
	printf("\n\r");
}

status_t ms_valid_msg(uint8_t *data, uint16_t len)
{
	if(0xAA != data[0]){return FAILD;}									//check head
	if(len != data[1]+1){return FAILD;}								//check length
	if(data[1] < 10){return FAILD;}										//msamrt éæ¤æ·éèç«¯ææ·éæ¤æ·éæ¤æ·å°éæ¤æ·éæ¤æ?10
	//if((0 != data[3]) && (data[3] != (data[1]^data[2]))){return FAIL;}	//check XOR
	int i = 0;
	uint8_t check_sum = 0;
	for(i = 1; i < data[1]; i++)
	{
		check_sum += data[i];
	}
	check_sum = (~(check_sum)) + 1;
	if(check_sum != data[data[1]]){return FAILD;}
	if(MS_UART_SN_ACQUIRE == data[9])
	{
		if(data[1] != 42)
		{
			return FAILD;//FAIL: 0
		}
	}

	return OK;// OK: 1
}


uint8_t ms_uart_pro_check_sum(const uint8_t *offset_rx, uint8_t rx_len)
{
	uint8_t check_sum = 0;
	uint8_t i = 0;

	for (i = 0; i < rx_len; i++){
		check_sum += offset_rx[i];
	}
	check_sum = ~(check_sum);
	check_sum += 1;

	return check_sum;
}

#define SN_LEN	32

void ms_uart_ack(uint8_t *ack_data, uint16_t data_len, uint8_t *src_data)
{
	ms_uart_do_packet_para_t para;
	ms_uart_msg_head_t *p_msg_data = NULL;
	p_msg_data = (ms_uart_msg_head_t *)src_data;

	para.data = ack_data;
	para.data_len = &data_len;
	para.device_type = p_msg_data->dev_type;
	para.msg_id    = p_msg_data->msg_id;
	para.cmd	   = p_msg_data->cmd_type;
	ms_uart_do_package(&para);
	ms_hal_hw_uart_write(g_serial_fd,para.data, *(para.data_len));
}

void ms_uart_sn_process(void)
{
	MS_TRACE("Get SN: %s",device_sn);

	if (memcmp(ms_needed_stored_info.sn, device_sn, 32) != 0) // sn change or get first sn(stored sn = 32 * 0xff)
	{
		memcpy(ms_needed_stored_info.sn, device_sn, 32);
		ms_needed_stored_info.sn[32] = '\0';
		MS_TRACE("%s", ((ms_needed_stored_info.device_type[1] == 0) ? "first time to get sn": "sn changed..."));
	}

	/* ---------------------------------------*/
	/*  ç¶æéæ¤æ·éæ¤æ·è½¬ : SN get. --> request A0.  */
	/*----------------------------------------*/
	uartStateMachine = MS_UART_SM_GET_A0;
}



void ms_uart_do_package(ms_uart_do_packet_para_t *para)
{
	para->data[0] = 0xAA;
	para->data[1] = *(para->data_len)+10;
	para->data[2] = para->device_type;
	para->data[3] = para->data[1]^para->data[2];
	para->data[4] = 0x00;
	para->data[5] = 0x00;
	para->data[6] = para->msg_id;
	para->data[7] = 0x00;
	para->data[8] = 0x00;
	para->data[9] = para->cmd;

	if(para->device_type != 0xFF)
	{
		para->device_type = gs_dev_type;
	}
	para->data[para->data[1]] = ms_uart_pro_check_sum(para->data+1, para->data[1]-1);
	*(para->data_len) = para->data[1]+1;
}

void ms_uart_fw_ask_for_sn(uint8_t cmd) //0911 the meaning is ... ?
{
	MS_TRACE("get sn...");
	printf("get sn...\n");
	uint16_t data_len = 0;
	uint8_t buff[32];
	ms_uart_do_packet_para_t para;
	memset(buff, 0, 32);

	if(0 == cmd)
	{
		data_len = 1;
		para.data = buff;
		para.data_len  = &data_len;
		para.device_type = 0xff;
		para.msg_id    = g_uart_msg_id++;
		para.cmd	   = MS_UART_SN_ACQUIRE;
		ms_uart_do_package(&para);
		ms_hal_hw_uart_write(g_serial_fd,buff, data_len);
	}
	else if(0x65 == cmd || 0x07 == cmd)
	{
		data_len = (0x07 == cmd) ? 1 : 20;
		para.data = buff;
		para.data_len  = &data_len;
		para.device_type = 0xff;
		para.msg_id    = g_uart_msg_id++;
		para.cmd	   = cmd;//07 or 65
		ms_uart_do_package(&para);

        // add by 926 @20190625
        #if 0
        printf("\nms_uart_fw_ask_for_sn:");
        for (int i=0; i < data_len; i++) {
            printf("%#02x ", buff[i]);
        }
        printf("\n");
        #endif

		ms_hal_hw_uart_write(g_serial_fd,buff, data_len);
	}
	else
	{
		MS_TRACE("Wrong call with this sn request function @ %s %d !", __FUNCTION__, __LINE__);
	}
}

void ms_uart_sys_status_report(ms_sys_status_t *sys_status)
{
	uint8_t buff_cache[40];
	memset(buff_cache, 0, 40);
	ms_uart_do_packet_para_t para;

	memcpy((ms_sys_status_t *)&buff_cache[MS_UART_MSG_DATA_BEGIN_BYTE], sys_status, sizeof(ms_sys_status_t));

	uint16_t data_len  = sizeof(ms_sys_status_t);
	para.data = buff_cache;
	para.data_len = &data_len;
	para.device_type = gs_dev_type;
	para.msg_id = g_uart_msg_id++;
	para.cmd = MS_UART_ONLINE_NOTIFY;
	ms_uart_do_package(&para);
#ifndef MS_TEST_FOR_AUTOLINK
	ms_hal_hw_uart_write(g_serial_fd,buff_cache, data_len);
#endif
}


void ms_uart_0x07_process(uint8_t *buff)
{
	int status;
	uint8_t * msg_act_data;
	ms_uart_msg_head_t *p_msg_data;
	uint8_t sn_arr[33] = {0};
	p_msg_data = (ms_uart_msg_head_t *)buff;
	msg_act_data = &buff[10];//10: data begin byte
	gs_dev_type = p_msg_data->dev_type;

	if(MS_UART_SM_SELFCHECK == uartStateMachine)
	{
		status = ms_check_sn_is_valid(msg_act_data , p_msg_data->msg_length-10);
		if(status == OK)
		{
			if(false == g_sn_0x14_got_flag)
			{
				memcpy(g_SN_0x14, (uint8_t *)msg_act_data, MS_DEV_SN_LENGTH);
				g_SN_0x14[32] = '\0';
				g_sn_0x14_got_flag = true;
				//g_report_state_0x14 = GOT_SN_INSIDE_0X14;
				MS_TRACE("get sn in process of 0x14.");
			}
			memcpy(sn_arr, (uint8_t *)msg_act_data, MS_DEV_SN_LENGTH);
			sn_arr[32] = '\0';
			if(0 != strcmp((char *)sn_arr, (char*)g_SN_0x14))
			{
				MS_TRACE("New sn (0x14).");
				MS_TRACE("old sn (0x14): %s", g_SN_0x14);
				MS_TRACE("New sn (0x14): %s", sn_arr);
				ms_sys_status_t sys_status;
				CLR_STRUCT(&sys_status);
				ms_hal_read_msmart_config_info(MS_SYS_STATUS_PATH,(uint8_t *)&sys_status, sizeof(ms_sys_status_t));
				sys_status.lan_status = 0xff;
				ms_uart_sys_status_report(&sys_status);
			}
		}
		else
		{
			MS_TRACE("Received SN data from device, but invailed !");
		}
	}
	else
	{
		if(g_sn_status == OK)
		{
			return;
		}

		uint8_t ms_sn[MS_DEV_SN_LENGTH] = {0};
		int ms_sn_len = MS_DEV_SN_LENGTH;

		ms_needed_stored_info.device_type[1] = p_msg_data->dev_type;
		MS_TRACE("device_type: %02x[%02x]", gs_dev_type, ms_needed_stored_info.device_type[1]);

		MS_TRACE("before sn decrypt...");
		//status = ms_check_sn_is_valid(msg_act_data , p_msg_data->msg_length-10);
		status = ms_sn_decrypt(msg_act_data, p_msg_data->msg_length-10, ms_sn, &ms_sn_len);
		if(status == OK)
		{
			MS_TRACE("after sn decrypt[%d]", ms_sn_len);
			memcpy((uint8_t *)device_sn, (uint8_t *)ms_sn, ms_sn_len);
			g_sn_status = OK; //get SN already.
			ms_uart_sn_process();
		}
		else
		{
			MS_ERR_TRACE("sn invalid");
			g_sn_status = ERROR;
		}
	}
}

void ms_uart_0x65_process(uint8_t *buff)
{
	int status;
	uint8_t * msg_act_data;
	ms_uart_msg_head_t *p_msg_data;
	p_msg_data = (ms_uart_msg_head_t *)buff;
	msg_act_data = &buff[10];//10: data begin byte
	gs_dev_type = p_msg_data->dev_type;

	if(MS_UART_SM_SELFCHECK == uartStateMachine)
	{
		uint8_t sn_arr[33] = {0};
		status = ms_check_is_air_condition_sn(msg_act_data + 5, sn_arr);
		if(status == OK)
		{
			if(false == g_sn_0x14_got_flag)
			{
				memcpy(g_SN_0x14, (uint8_t *)sn_arr, MS_DEV_SN_LENGTH);
				g_SN_0x14[32] = '\0';
				g_sn_0x14_got_flag = true;
				//g_report_state_0x14 = GOT_SN_INSIDE_0X14;
				MS_TRACE("get sn in process of 0x14.");
			}
			memcpy(sn_arr, (uint8_t *)sn_arr, MS_DEV_SN_LENGTH);
			sn_arr[32] = '\0';
			if(0 != strcmp((char *)sn_arr, (char *)g_SN_0x14))
			{
				MS_TRACE("New sn (0x14).");
				MS_TRACE("old sn (0x14): %s", g_SN_0x14);
				MS_TRACE("New sn (0x14): %s", sn_arr);
				ms_sys_status_t sys_status;
				CLR_STRUCT(&sys_status);
				ms_hal_read_msmart_config_info(MS_SYS_STATUS_PATH,(uint8_t *)&sys_status, sizeof(ms_sys_status_t));
				sys_status.lan_status = 0xff;
				ms_uart_sys_status_report(&sys_status);
			}
		}
		else
		{
			MS_TRACE("Received SN data from device, but invailed !");
		}
	}
	else
	{
		if(g_sn_status == OK)
		{
			return;
		}

		uint8_t ms_sn[MS_DEV_SN_LENGTH] = {0};
		int ms_sn_len = MS_DEV_SN_LENGTH;

		ms_needed_stored_info.device_type[1] = p_msg_data->dev_type;
		MS_TRACE("device_type: %02x[%02x]", gs_dev_type, ms_needed_stored_info.device_type[1]);

		MS_TRACE("before sn decrypt...");
		//status = ms_check_sn_is_valid(msg_act_data , p_msg_data->msg_length-10);
		status = ms_sn_decrypt(msg_act_data, p_msg_data->msg_length-10, ms_sn, &ms_sn_len);
		if(status == OK)
		{
			MS_TRACE("after sn decrypt[%d]", ms_sn_len);
			memcpy((uint8_t *)device_sn, (uint8_t *)ms_sn, ms_sn_len);
			g_sn_status = OK; //get SN already.
			ms_uart_sn_process();
		}
		else
		{
			MS_ERR_TRACE("sn invalid");
			g_sn_status = ERROR;
		}
	}
}

void ms_uart_request_E1(void)
{
	static uint32_t timestamp = 0;

	if(gs_E1_loop_cnt >= MAX_E1_REQUEST_TIMES)//éæ¤æ·éæ¤æ·éæ¤æ·éæ¤æ·éæ¤æ·éç´?éæ¤æ·éæ¤æ·éæ¤æ·éè½¿è¾¾æ·çéç´¼1éæªé©æ·éæ¤æ·éæ¤æ·éå¿ç­¹æ·éæ¤æ·åE1éæ¤æ·éæ¤æ·
	{
		uartStateMachine = MS_UART_SM_GET_E1;

	         uint8_t bufftemp[]={ 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0x81,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00, 0x00, 0x00, 0x00};
	         MS_TRACE("0xe1 overtime, use default:softap, forever");
	         ms_uart_0xE1_process(bufftemp);

		return;
	}

	if(ms_sys_api_ticks_compare_result(msm_timer_tickcount_get(), timestamp,  300))
	{
		uint16_t data_len = 0;
		uint8_t buff_cache[32];
		ms_uart_do_packet_para_t para;

		timestamp = msm_timer_tickcount_get();

		MS_TRACE("get e1...");
		memset(buff_cache, 0, 32);
		data_len = 1;
		para.data = buff_cache;
		para.data_len    = &data_len;
		para.device_type = gs_dev_type;
		para.msg_id    = g_uart_msg_id++;
		para.cmd	   = MS_UART_DEV_SERVICE_QUERY;
		ms_uart_do_package(&para);
		ms_hal_hw_uart_write(g_serial_fd,buff_cache, data_len);
		gs_E1_loop_cnt++;
	}
}

void ms_uart_0xE1_process(uint8_t *buff)
{
	uint8_t *msg_act_data = NULL;

	if(MS_UART_SM_GET_E1 != uartStateMachine)
	{
		MS_TRACE("drop 0xe1");
		return ;
	}

	msg_act_data = &buff[10];

//	if(!((msg_act_data[0] == 0x81 && msg_act_data[1] == 0x81) ||(msg_act_data[0] == 0x01 && msg_act_data[1] == 0x00)))//åªéæ¤æ·éæ¤æ·éé°ç¢æ·éï¿?
//	{
//		if(ms_needed_stored_info.device_type[1] == 0)
//		{
//			ms_needed_stored_info.boot_mode = MS_BOOT_MODE_CONFIG_MS_AP;
//			ms_needed_stored_info.ap_sniffer_keep_alive_tm = 0;
//			ms_needed_stored_info.e1_default_mode = ms_needed_stored_info.boot_mode;
//		}
//		MS_TRACE("old device, used default setting :%d,%d",ms_needed_stored_info.boot_mode,ms_needed_stored_info.e1_default_mode);
//		uartStateMachine = MS_UART_SM_START_OK;
//		return ;
//	}

	if(msg_act_data[2] == 0x01)
	{
		MS_TRACE("not support uart retrans.");
	}

	if(msg_act_data[3] == 0x01)
	{
		MS_TRACE("not support appliance upgrade.");
	}

	if(msg_act_data[4] != 0x00)
	{
		MS_TRACE("only support time-trans.");
	}

	if(msg_act_data[5] == 0x00)
	{
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_CONFIG_MS_AP;
	}
	else if(msg_act_data[5] == 0x01)
	{
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_CONFIG_MIX_SNIFFER;
	}
	else if(msg_act_data[5] == 0x02)
	{
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_STA;
	}
	else if(msg_act_data[5] == 0x03)
	{
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_CONFIG_AIRKISS;
	}
	else if(msg_act_data[5] == 0x06)
	{
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_CONFIG_MS_SNIFFER;
	}
	else
	{
		MS_TRACE("not support mode[%02x], default idle", msg_act_data[5]);
		ms_needed_stored_info.e1_default_mode = MS_BOOT_MODE_IDLE;
	}

//	if(ms_needed_stored_info.device_type[1] != 0/*not first time run*/ && ms_needed_stored_info.boot_mode != ms_needed_stored_info.e1_default_mode)
//	{
//		ms_needed_stored_info.boot_mode = ms_needed_stored_info.e1_default_mode;
//		MS_TRACE("use e1[%02x]", ms_needed_stored_info.boot_mode);
//		MS_TRACE("use boot mode[%02x](%s) not e1[%02x]", ms_needed_stored_info.boot_mode,  (ms_needed_stored_info.boot_mode == MS_BOOT_MODE_STA ? "sta" : "0x64"), ms_needed_stored_info.e1_default_mode);
//	}

	ms_needed_stored_info.ap_sniffer_keep_alive_tm = (msg_act_data[9] <= 0x0F) ? msg_act_data[9] : 0x0F;

	ms_needed_stored_info.protocol_support = msg_act_data[10];

	MS_TRACE("default boot mode = %d", ms_needed_stored_info.e1_default_mode);
	MS_TRACE("running time = %d", ms_needed_stored_info.ap_sniffer_keep_alive_tm);
	MS_TRACE("third_protocol_opt = %d", ms_needed_stored_info.protocol_support);

	/* -------------------------------------------*/
	/*  ç¶æéæ¤æ·éæ¤æ·è½¬ :   get e1. --> START_OK.      */
	/*--------------------------------------------*/
	uartStateMachine = MS_UART_SM_START_OK;
}

void ms_uart_request_A0(void)
{
	static uint32_t timestamp = 0;

	if(gs_A0_loop_cnt >= MAX_A0_REQUEST_TIMES)
	{
		/* -------------------------------------------*/
		/*  ç¶æéæ¤æ·éæ¤æ·è½¬ : not get A0. --> get e1.    */
		/*--------------------------------------------*/
		uartStateMachine = MS_UART_SM_GET_E1;
	}

	if(ms_sys_api_ticks_compare_result(msm_timer_tickcount_get(), timestamp,  300))
	{
		uint16_t data_len = 0;
		uint8_t buff_cache[32];
		ms_uart_do_packet_para_t para;

		timestamp = msm_timer_tickcount_get();

		MS_TRACE("get a0...");
		memset(buff_cache, 0, 32);
		data_len  = 20;
		para.data = buff_cache;
		para.data_len = &data_len;
		para.device_type = gs_dev_type;
		para.msg_id = g_uart_msg_id++;
		para.cmd = MS_UART_QUERY_DEV_INFO;
		ms_uart_do_package(&para);
		ms_hal_hw_uart_write(g_serial_fd,buff_cache, data_len);
		gs_A0_loop_cnt++;
	}
}

void ms_uart_0xA0_process(uint8_t *buff)
{
	if(MS_UART_SM_GET_A0 == uartStateMachine)
	{
		/* -------------------------------------------*/
		/*  ç¶æéæ¤æ·éæ¤æ·è½¬ :   get A0. --> get e1.      */
		/*--------------------------------------------*/
		uartStateMachine = MS_UART_SM_GET_E1;


		uint8_t *msg_act_data = NULL;
		msg_act_data = &buff[10];

		memcpy(ms_needed_stored_info.device_type + 2, msg_act_data + 2, 2);
		MS_TRACE("appliance info: %02x:%02x:%02x:%02x",ms_needed_stored_info.device_type[0], ms_needed_stored_info.device_type[1], ms_needed_stored_info.device_type[2], ms_needed_stored_info.device_type[3]);
	}
}

static ms_result_t ms_send_msg_to_msmart_fifo(T_MsTransStream *stream){
	int fd_net_in = -1;
	int fd_local_net_in = -1;
	int fd_speech_in = -1;
	int fd = 3; //which FIFO to send

	ms_result_t ret =  MS_RESULT_SUCCESS;
	MS_TRACE("DBG1");
	if(-1 == (fd_net_in = open(MSMART_NET_IN ,O_WRONLY | O_NONBLOCK))){
			MS_ERR_TRACE("/tmp/msmart_net_in OPEN ERROR:%s:",strerror(errno));
			ret = MS_RESULT_FAIL;
			//goto LAST;
	}

	if(-1 == (fd_local_net_in = open(MSMART_LOCAL_NET_IN,O_WRONLY|O_NONBLOCK))){
		MS_ERR_TRACE("/tmp/msmart_local_net_in OPEN ERROR:%s:",strerror(errno));
		ret = MS_RESULT_FAIL;
		//goto LAST;
	}
	MS_TRACE("DBG2");

	if(-1 == (fd_speech_in = open(MAISPEECH_IN,O_WRONLY|O_NONBLOCK))){
		MS_ERR_TRACE("/tmp/maipseech_in OPEN ERROR:%s:",strerror(errno));
		ret = MS_RESULT_FAIL;
		//goto LAST;
	}
	MS_TRACE("DBG3");

//    _ceck_event_flag(&fd, stream);
	if(((stream->data[9] == MS_UART_CTL_CMD) || (stream->data[9] == MS_UART_QUERY_CMD))&&(gstEventFlag.stNet.ucMsgIdentify || gstEventFlag.stLocalNet.ucMsgIdentify || gstEventFlag.stSpeech.ucMsgIdentify)){
        if (0 != gstEventFlag.stNet.ucMsgIdentify)
        {
            if(-1 == (write(fd_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
                MS_ERR_TRACE("WRITE DEVICE ERROR");
                ret = MS_RESULT_FAIL;
            }else{
		    MS_DBG_TRACE("TRANS NET Message");
            }
            gstEventFlag.stNet.ucMsgIdentify = 0;
            gstEventFlag.stNet.iCmdFlag      = 0;
       }
       if (0 != gstEventFlag.stLocalNet.ucMsgIdentify)
       {
            if(-1 == (write(fd_local_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
                MS_ERR_TRACE("WRITE DEVICE ERROR");
                ret = MS_RESULT_FAIL;
            }else{
                MS_DBG_TRACE("TRANS localNET Message");
            }
            gstEventFlag.stLocalNet.ucMsgIdentify = 0;
            gstEventFlag.stLocalNet.iCmdFlag      = 0;
       }
       if (0 != gstEventFlag.stSpeech.ucMsgIdentify)
       {
            if(-1 == (write(fd_speech_in, (void*)stream, sizeof(T_MsTransStream)))){
                MS_ERR_TRACE("WRITE DEVICE ERROR");
                ret = MS_RESULT_FAIL;
            }else{
                MS_DBG_TRACE("TRANS SPEECH Message");
            }
            gstEventFlag.stSpeech.ucMsgIdentify = 0;
            gstEventFlag.stSpeech.iCmdFlag      = 0;
       }
       goto LAST;
	}
//    switch (fd)
//    {
//        case 0:
//            if(-1 == (write(fd_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
//                MS_ERR_TRACE("WRITE DEVICE ERROR");
//                ret = MS_RESULT_FAIL;
//            }else{
//		MS_DBG_TRACE("TRANS NET Message");
//            }
//            goto LAST;
//            //break;        //no break, go to LAST
//        case 1:
//            if(-1 == (write(fd_local_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
//                MS_ERR_TRACE("WRITE DEVICE ERROR");
//                ret = MS_RESULT_FAIL;
//            }else{
//                MS_DBG_TRACE("TRANS localNET Message");
//            }
//            goto LAST;
//            //break;        //no break, go to LAST
//        case 2:
//            if(-1 == (write(fd_speech_in, (void*)stream, sizeof(T_MsTransStream)))){
//                MS_ERR_TRACE("WRITE DEVICE ERROR");
//                ret = MS_RESULT_FAIL;
//            }else{
//                MS_DBG_TRACE("TRANS SPEECH Message");
//            }
//            goto LAST;
//            //break;        //no break, go to LAST
//        case 3: //no break
//        default:
//            break;
//    }

    if(-1 == (write(fd_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
        MS_ERR_TRACE("WRITE DEVICE ERROR");
        ret = MS_RESULT_FAIL;
        //goto LAST;
    }else{
	MS_DBG_TRACE("TRANS NET Message");
    }

    uint8_t cmd_type = stream->data[9];
	if (     (cmd_type == MS_UART_QUERY_CMD)
	      || (cmd_type == MS_UART_CTL_CMD)
	      || (cmd_type == MS_UART_REPORT_RUNNING_PARA_NOACK)
          || ((MS_EVENT_TO_MSPEECH_MIN < stream->event) && (MS_EVENT_TO_MSPEECH_MAX > stream->event)) /*MS_LOCALNET_EVENT_NETWORK_SETTING*/){
	    if(-1 == (write(fd_speech_in, (void*)stream, sizeof(T_MsTransStream)))){
	        MS_ERR_TRACE("WRITE DEVICE ERROR");
	        ret = MS_RESULT_FAIL;
	       // goto LAST;
	    }else{
		MS_DBG_TRACE("TRANS SPEECH Message");
	    }
	}

    if(-1 == (write(fd_local_net_in, (void*)stream ,sizeof(T_MsTransStream)))){
        MS_ERR_TRACE("READ DEVICE ERROR");
        ret = MS_RESULT_FAIL;
        //goto LAST;
    }else{
        MS_DBG_TRACE("TRANS LOCAL_NET Message");
    }

LAST:
    if(fd_speech_in >= 0)
        close(fd_speech_in);
    if(fd_net_in>=0)
        close(fd_net_in);
    if(fd_local_net_in>=0)
	    close(fd_local_net_in);
	return ret;
}

void ms_uart_send_start(void)
{
	T_MsTransStream stream;

	ms_hal_wlan_get_mac_address(ms_needed_stored_info.mac_addr);
	MS_TRACE("ms_needed_stored_info sn:");
	PRINT_BUF(ms_needed_stored_info.sn,sizeof(ms_needed_stored_info.sn));
	MS_TRACE("ms_needed_stored_info device type:");
	PRINT_BUF(ms_needed_stored_info.device_type,sizeof(ms_needed_stored_info.device_type));
	MS_TRACE("ms_needed_stored_info Mac address:");
	PRINT_BUF(ms_needed_stored_info.mac_addr,sizeof(ms_needed_stored_info.mac_addr));
	MS_TRACE("ms_needed_stored_info default wifi config mode:%d:",ms_needed_stored_info.e1_default_mode);
	MS_TRACE("ms_needed_stored_info boot_mode:%d:",ms_needed_stored_info.boot_mode);

	msm_write_config(MS_STORED_INFO,&ms_needed_stored_info,sizeof(ms_needed_stored_info));

//  memset(&stream, 0, sizeof(stream));
//  stream.event = MS_UART_EVENT_START_SUCCESS;
//	if(MS_RESULT_FAIL == ms_send_msg_to_msmart_fifo(&stream))
//	{
//		MS_ERR_TRACE("send msg to main queue fail");
//	}
//	else
//	{
//		MS_TRACE("uart started, %d %p", stream.size, stream.data);
//	}
}

void ms_printf_heap(void)
{

}

void ms_feed_watchdog_and_print_heap_info(void)
{
	static uint32_t timestamp = 0;

	if(ms_sys_api_ticks_compare_result(msm_timer_tickcount_get(), timestamp,  10000))
	{
	    printf("feed\n");
		timestamp = msm_timer_tickcount_get();
		ms_printf_heap();
	}
}

void ms_uart_state_machine(void)
{
	switch(uartStateMachine)
	{
		case MS_UART_SM_GET_SN:
		{
			ms_uart_fw_ask_for_sn(0);
			break;
		}
		case MS_UART_SM_GET_A0:
		{
			ms_uart_request_A0();
			break;
		}
		case MS_UART_SM_GET_E1:
		{
			ms_uart_request_E1();
			break;
		}

		case MS_UART_SM_START_OK:
		{
			//send started
			ms_uart_send_start();
			if(deamon_pid!=0){
			    kill(deamon_pid,SIG_SYS_EVENT_DEV_READY);
			}
			uartStateMachine = MS_UART_SM_IDLE;
			MS_DBG_TRACE("UART Start OK");
			break;
		}
		case MS_UART_SM_IDLE:
		{
			break;
		}
		case MS_UART_SM_SELFCHECK:
		{
			MS_TRACE("in self test mode, shouldn't see me!");

			break;
		}
		default:
			MS_TRACE("shouldn't see me!");
	}
}

void ms_uart_0x13_process(uint8_t *buff)
{
	int i = 0;
	status_t ret = 0;
	uint8_t data[20];
	memset(data, 0, 20);

	ret = ms_hal_wlan_get_mac_address(data);
	if (OK != ret)
	{
		MS_TRACE("get mac address error.");
		return;
	}
	for(i = 0; i < 6; i++)
	{
		data[MS_UART_MSG_DATA_BEGIN_BYTE+i] = data[5-i];//inverted sequence
	}
	ms_uart_ack(data, 6, buff);
}

void ms_uart_0x63_process(uint8_t *buff)
{
	uint8_t data[40];
	ms_sys_status_t *net_status;
	memset(data, 0, 40);
	net_status = (ms_sys_status_t *)&data[MS_UART_MSG_DATA_BEGIN_BYTE];

	ms_hal_read_msmart_config_info(MS_SYS_STATUS_PATH,(uint8_t *)net_status, sizeof(ms_sys_status_t));

	ms_uart_ack(data, sizeof(ms_sys_status_t), buff);
}
void ms_uart_0x61_process(uint8_t *buff)
{
	uint8_t data[44];
	uint8_t *msgbody;
	memset(data, 0, 44);

    unsigned int timezone = 0;
    time_t rawtime;
    time_t t1,t2;
    struct tm timeinfo;
	struct tm tm_utc;


	time(&rawtime);

localtime_r(&rawtime,&timeinfo);
	t1 = mktime(&timeinfo);

	 gmtime_r(&rawtime,&tm_utc);
	t2 = mktime(&tm_utc);
	 timezone = (t1 - t2) / 3600;
	msgbody = &data[MS_UART_MSG_DATA_BEGIN_BYTE];
	*msgbody++ = timeinfo.tm_sec;
	*msgbody++ = timeinfo.tm_min;
	*msgbody++ = timeinfo.tm_hour;
	*msgbody++ = timeinfo.tm_wday;
	*msgbody++ = timeinfo.tm_mday;
	*msgbody++ = timeinfo.tm_mon+1;
	*msgbody++ = timeinfo.tm_year+1900;
	*msgbody++ = timezone;
//	printf("time:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",timeinfo.tm_year,timeinfo.tm_mon,timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_wday,timezone,t1,t2);
//	printf("curtimeÂ£Âº%s::%s", asctime(&timeinfo),asctime(&tm_utc));
	ms_uart_ack(data,20, buff);
}

void ms_uart_0x6a_process(uint8_t *buff)
{
	uint8_t *data = NULL;
	uint8_t uart_reply[40];
	uint8_t *msg_act_data = NULL;
	uint8_t dev_id[13];

	memset(dev_id, 0, 13);
	memset(uart_reply, 0, 40);

	memset(ssid_0x6a, 0, MS_SSID_MAX_LENGTH);
	memset(pwd_0x6a, 0, MS_SSID_MAX_LENGTH);

	data = &uart_reply[MS_UART_MSG_DATA_BEGIN_BYTE];
	msg_act_data = &buff[MS_UART_MSG_DATA_BEGIN_BYTE];
	ssid_len_0x6a = msg_act_data[1];
	pwd_len_0x6a = msg_act_data[2];

	data[0] = 0;
	data[1] = 0;
	if((msg_act_data[0] != 0) && msg_act_data[0] != 1 && msg_act_data[0] != 2 )
	{
		data[0] = 1;
		data[1] = 4;
	}
	if((msg_act_data[1]>32) || (msg_act_data[1]<1))
	{
		data[0] = 1;
		data[1] = 2;
	}
	if(0 != msg_act_data[0] && ((msg_act_data[2]>32) || (msg_act_data[2]<8)))
	{
		data[0] = 1;
		data[1] = 3;
	}

	if(0 == data[0])
	{
		ms_0x6a_config_info = ENUM_MS_0X6A_CONFIG_INFO_SUCCESS;
		for(int i=0;i<ssid_len_0x6a;i++)
		{
			ssid_0x6a[i] = msg_act_data[3+i];
		}
		ssid_0x6a[ssid_len_0x6a] = '\0';
		for(int i=0;i<pwd_len_0x6a;i++)
		{
			pwd_0x6a[i] = msg_act_data[3+i+ssid_len_0x6a];
		}
		pwd_0x6a[pwd_len_0x6a] = '\0';
	}
	else
	{
		ms_0x6a_config_info = ENUM_MS_0X6A_CONFIG_INFO_ERROR;
	}

	ms_uart_ack(uart_reply, 2, buff);
}

void ms_uart_0x81_process(uint8_t *buff)
{
	uint8_t *data = NULL;
	uint8_t uart_reply[40];
	uint8_t *msg_act_data = NULL;
	uint8_t dev_id[13];

	ms_stored_info_t	ms_needed_stored_info;
	uint8_t temp_boot_mode;

	memset(&ms_needed_stored_info, 0, sizeof(ms_needed_stored_info));
	memset(dev_id, 0, 13);
	memset(uart_reply, 0, 40);

	data = &uart_reply[MS_UART_MSG_DATA_BEGIN_BYTE];
	msg_act_data = &buff[MS_UART_MSG_DATA_BEGIN_BYTE];

	data[0] = 0;
	if(-1 == ms_hal_read_msmart_config_info(MS_STORED_INFO,(uint8_t *)&ms_needed_stored_info, sizeof(ms_needed_stored_info)))
	{
		MS_ERR_TRACE("read msmart config info");
	}
	if((msg_act_data[0] == 1) && (ms_needed_stored_info.boot_mode != 0x02) )
	{
		temp_boot_mode = MS_BOOT_MODE_CONFIG_MS_AP;
		memset(ms_needed_stored_info.pwd, 0, sizeof(ms_needed_stored_info.pwd));
		memset(ms_needed_stored_info.ssid, 0, sizeof(ms_needed_stored_info.ssid));
		ms_needed_stored_info.pwd_len = 0;
		ms_needed_stored_info.ssid_len = 0;
		ms_needed_stored_info.boot_mode = temp_boot_mode;
		data[0] = 1;
	}
	if((msg_act_data[0] == 2) && (ms_needed_stored_info.ssid != 0) && (ms_needed_stored_info.boot_mode != 0x00))
	{
		temp_boot_mode = MS_BOOT_MODE_STA;
		//memcpy(ms_needed_stored_info.ssid, ssid_0x6a, ssid_len_0x6a);
		//memset(ms_needed_stored_info.pwd, pwd_0x6a, pwd_len_0x6a);

		MS_DBG_TRACE("ssid_len:%d pwd_len:%d", ms_needed_stored_info.ssid_len, ms_needed_stored_info.pwd_len);
		PRINT_BUF(ms_needed_stored_info.ssid, ms_needed_stored_info.ssid_len);
		MS_DBG_TRACE("...\r\n");
		PRINT_BUF(ms_needed_stored_info.pwd, ms_needed_stored_info.pwd_len);
		MS_DBG_TRACE("***\r\n");

		if(ENUM_MS_0X6A_CONFIG_INFO_SUCCESS == ms_0x6a_config_info)
		{
			MS_DBG_TRACE("switch to sta[0x6a]");
			if(ssid_len_0x6a)
			{
				ms_needed_stored_info.ssid_len = ssid_len_0x6a;
				ms_needed_stored_info.pwd_len = pwd_len_0x6a;
			}

			ms_needed_stored_info.boot_mode = temp_boot_mode;
			int i;
			printf("ms_needed_stored_info.ssid is");
			for(i=0;i<ssid_len_0x6a;i++)
			{
				ms_needed_stored_info.ssid[i] = ssid_0x6a[i];
				printf(" %x ",ms_needed_stored_info.ssid[i]);
			}
			ms_needed_stored_info.ssid[ssid_len_0x6a] = '\0';
			printf("ms_needed_stored_info.pwd is");
			for(i=0;i<pwd_len_0x6a;i++)
			{
				ms_needed_stored_info.pwd[i] = pwd_0x6a[i];
				printf(" %x ",ms_needed_stored_info.pwd[i]);
			}
			ms_needed_stored_info.pwd[pwd_len_0x6a] = '\0';

			data[0] = 2;
		}
		else if(ENUM_MS_0X6A_CONFIG_INFO_ERROR == ms_0x6a_config_info)
		{
			MS_DBG_TRACE("0x6a msg error");
		}
		else if(ENUM_MS_0X6A_CONFIG_INFO_NONE == ms_0x6a_config_info)
		{
			if(0 != ms_needed_stored_info.ssid_len)
			{
				MS_DBG_TRACE("switch to sta[own info]");
				ms_needed_stored_info.boot_mode = temp_boot_mode;
				data[0] = 2;
			}
		}
		else
		{
			MS_ERR_TRACE("shouldn't see me");
		}
	}

	ms_uart_ack(uart_reply, 1, buff);
}

void ms_uart_0x64_process(uint8_t *buff)
{
	uint8_t *data = NULL;
	uint8_t uart_reply[40];
	uint8_t *msg_act_data = NULL;
	uint8_t dev_id[13];
	uint8_t sev_flag = 0;

	ms_stored_info_t	ms_needed_stored_info;
	uint8_t temp_boot_mode = MS_BOOT_MODE_UNKNOWN;

    T_MsTransStream stStream;

	memset(&ms_needed_stored_info, 0, sizeof(ms_needed_stored_info));
	memset(dev_id, 0, 13);
	memset(uart_reply, 0, 40);

	data = &uart_reply[MS_UART_MSG_DATA_BEGIN_BYTE];
	msg_act_data = &buff[MS_UART_MSG_DATA_BEGIN_BYTE];

	data[4] = 0x03;
	data[5] = 0x03;

	if(-1 == ms_hal_read_msmart_config_info(MS_STORED_INFO,(uint8_t *)&ms_needed_stored_info, sizeof(ms_needed_stored_info)))
	{
		MS_DBG_TRACE("read msmart config info not exist");
		data[6] = 0x02;
	}

//	if(msg_act_data[20] == 0xf1)
//	{
//	    uint8_t i=0;
//	    uint32_t sum=0;
//	    for(i =0;i<13;i++)
//	    {
//                 sum+=msg_act_data[7+i];
//	    }
//              if(sum == 0)
//              {
//                 data[6] = 0x00;
//                 sev_flag = 1;
//              }
//	}

	if (0 == msg_act_data[6])/*to ap mode*/
	{
		temp_boot_mode = MS_BOOT_MODE_CONFIG_MS_AP;
		data[6] = 0x00;
	}
	else if (1 == msg_act_data[6])/*Common Mode*/
	{
		temp_boot_mode = MS_BOOT_MODE_CONFIG_MIX_SNIFFER;
		data[6] = 0x00;
	}
	else if (5 == msg_act_data[6])/*idle mode*/
	{
		temp_boot_mode = MS_BOOT_MODE_IDLE;
		data[6] = 0x00;
	}
	else if(8 == msg_act_data[6])/*msc*/
	{
		temp_boot_mode = MS_BOOT_MODE_CONFIG_MS_SNIFFER;
		data[6] = 0x00;
	}
	else
	{
		data[6] = 0x02;
	}

//	if(0 == data[6] ||sev_flag==1)
//	{
//	    if( sev_flag == 0)
//        {
//		memset(ms_needed_stored_info.pwd, 0, sizeof(ms_needed_stored_info.pwd));
//		memset(ms_needed_stored_info.ssid, 0, sizeof(ms_needed_stored_info.ssid));
//		ms_needed_stored_info.pwd_len = 0;
//		ms_needed_stored_info.ssid_len = 0;
//		ms_needed_stored_info.boot_mode = temp_boot_mode;
//		memset(ms_needed_stored_info.device_id, 0, sizeof(ms_needed_stored_info.device_id));
//
//		#if defined(MS_SST)
//		ms_hal_erase_license_operation_info();
//		#endif
//		ms_needed_stored_info.server_flag = SERVER_OFFICIAL;
//        }
//        else
//        {
//                ms_needed_stored_info.server_flag = SERVER_UNOFFICIAL;
//        }
//    }
    memset(ms_needed_stored_info.pwd, 0, sizeof(ms_needed_stored_info.pwd));
	memset(ms_needed_stored_info.ssid, 0, sizeof(ms_needed_stored_info.ssid));
	ms_needed_stored_info.pwd_len = 0;
	ms_needed_stored_info.ssid_len = 0;
	ms_needed_stored_info.boot_mode = temp_boot_mode;
	memset(ms_needed_stored_info.device_id, 0, sizeof(ms_needed_stored_info.device_id));
    msm_write_config(MS_STORED_INFO, (char *)&ms_needed_stored_info, sizeof(ms_needed_stored_info));
    usleep(10*1000);
#if 1
    MS_DBG_TRACE("send MS_LOCALNET_EVENT_NETWORK_SETTING event to speech, url: /oem/tts/network_config_start.mp3");
    memset(&stStream, 0, sizeof(stStream));
    stStream.event = MS_LOCALNET_EVENT_NETWORK_SETTING;
    //strncpy((char *)stStream.data, "/oem/tts/network_config_start.mp3", sizeof(stStream.data) - 1);
    //delete by 926 @20190619
    strncpy((char *)stStream.data, "/mnt/app/tts/network_config_start.mp3", sizeof(stStream.data) - 1);
    stStream.size  = strlen((const char *)stStream.data) + 1;
    stStream.msg_id = 0;
    ms_send_msg_to_msmart_fifo(&stStream);
#endif

    if(deamon_pid){
        kill(deamon_pid, SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE);
	}
	ms_uart_ack(uart_reply, 20, buff);
}

void ms_ai_0x64_process(uint8_t *buff)
{
		ms_stored_info_t	ms_needed_stored_info;
		uint8_t temp_boot_mode = MS_BOOT_MODE_UNKNOWN;

		memset(&ms_needed_stored_info, 0, sizeof(ms_needed_stored_info));

		if(-1 == ms_hal_read_msmart_config_info(MS_STORED_INFO,(uint8_t *)&ms_needed_stored_info, sizeof(ms_needed_stored_info)))
		{
			MS_DBG_TRACE("read msmart config info not exist");
			return;
		}

        temp_boot_mode = MS_BOOT_MODE_CONFIG_MS_AP;

		memset(ms_needed_stored_info.pwd, 0, sizeof(ms_needed_stored_info.pwd));
		memset(ms_needed_stored_info.ssid, 0, sizeof(ms_needed_stored_info.ssid));
		ms_needed_stored_info.pwd_len = 0;
		ms_needed_stored_info.ssid_len = 0;
		ms_needed_stored_info.boot_mode = temp_boot_mode;
		memset(ms_needed_stored_info.device_id, 0, sizeof(ms_needed_stored_info.device_id));

#if defined(MS_SST)
		ms_hal_erase_license_operation_info();
#endif
		ms_needed_stored_info.server_flag = SERVER_UNOFFICIAL;

		msm_write_config(MS_STORED_INFO, (char *)&ms_needed_stored_info, sizeof(ms_needed_stored_info));

		if(deamon_pid){
            kill(deamon_pid,SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE);
		}
	//	ms_uart_ack(uart_reply, 20, buff);
}

void ms_uart_0x82_process(uint8_t *buff)
{
	uint8_t uart_reply[20];

	memset(uart_reply, 0, 20);
	ms_uart_ack(uart_reply, 1, buff);

	MS_TRACE("reboot[uart]");
	system("reboot");
//	ms_send_important_event_to_main_queue(MS_SYS_EVENT_REBOOT, NULL);
}

void ms_uart_0x83_process(uint8_t *buff)
{
	uint8_t data[20];

	memset(data, 0, 20);
	ms_uart_ack(data, 1, buff);

	MS_TRACE("reset[uart]");
	system("recoverySystem");
}


void ms_uart_0x87_process(uint8_t *buff)
{
	uint8_t data[40];


	memset(data, 0, 40);
	ms_get_module_version(&data[10], 9);
	ms_uart_ack(data, 20, buff);
}

void ms_uart_syn_time_report(uint8_t *pbuf, uint8_t len)
{
	uint8_t buff_cache[40];
	memset(buff_cache, 0, 40);
	ms_uart_do_packet_para_t para;

	memcpy(&buff_cache[MS_UART_MSG_DATA_BEGIN_BYTE], pbuf, len);

	uint16_t data_len  = len;
	para.data = buff_cache;
	para.data_len = &data_len;
	para.device_type = gs_dev_type;
	para.msg_id = g_uart_msg_id++;
	para.cmd = MS_UART_SYNCHRONIZE_TIME;
	ms_uart_do_package(&para);
	ms_hal_hw_uart_write(g_serial_fd,buff_cache, data_len);
}


void ms_fw_uart_self_check_update_state(uint8_t fail_flag)
{

}

void ms_fw_uart_self_check_cmd_manage(ms_fw_uart_self_check_t *self_check_param, uint8_t *buff)
{

}


void ms_fw_uart_self_check_sm_handle(ms_fw_uart_self_check_t *self_check_param)
{
	switch(self_check_param->self_check_status)
	{
		case MS_SELF_DETECT_STATE_GET_SN:
		{
			break;
		}

		case MS_SELF_DETECT_STATE_SCAN_AP:
		{

			break;
		}

		case MS_SELF_DETECT_STATE_GET_SN_2S:
		{
			break;
		}

		case MS_SELF_DETECT_STATE_RESTART:
		{
			break;
		}

		case MS_SELF_DETECT_STATE_IDLE:
		{
			MS_TRACE("in idle@selfcheck");
			break;
		}
		default:
			break;
	}
}

ms_result_t ms_fw_uart_msg_verify(const uint8_t * const p_buffer)
{
	uint8_t i;
	uint8_t check_sum = 0;

	//check message length
	if(MS_UART_SN_ACQUIRE == p_buffer[MS_MSMART_UART_MSG_CMD_TYPE_BYTE])
	{
		if(p_buffer[MS_MSMART_UART_MSG_LEN_BYTE] != 42)
		{
			return MS_RESULT_FAIL;
		}
	}

	if((p_buffer[MS_MSMART_UART_HEAD_BYTE] == MS_MSMART_UART_HEAD) &&
		(p_buffer[MS_MSMART_UART_MSG_LEN_BYTE] >= MS_MSMART_UART_MSG_MIN_LEN))
	{
		ms_uart_msg_head_t *msg_buf = (ms_uart_msg_head_t *)p_buffer;

		if(msg_buf->msg_id != 0)
		{
			if((msg_buf->frame_crc == 0) || (msg_buf->frame_crc == (msg_buf->msg_length ^ msg_buf->dev_type)))
			{
			}
			else
			{
				return	MS_RESULT_FAIL;
			}
		}

		for(i = 1; i < (msg_buf->msg_length); i++)
		{
			check_sum += p_buffer[i];
		}

		check_sum = (~(check_sum)) + 1;
		if(check_sum == p_buffer[msg_buf->msg_length ])
		{
			return MS_RESULT_SUCCESS;
		}
	}

	return MS_RESULT_FAIL;
}
int ms_system(char *cmd)
{
	pid_t status;
	status = system(cmd);
	if (-1 == status){
		printf("system(\"%s\") error\n",cmd);
		return ERROR;
	}else{
		if (WIFEXITED(status)){
			if (0 == WEXITSTATUS(status)){
				printf("%s passed\n",cmd);
				return OK;
			}else{
				printf("%s return error: %d\n", cmd, WEXITSTATUS(status));
				return ERROR;
			}
		}else{
			printf("%s exec error\n",cmd);
			return ERROR;
		}
	}
}

void *factory_test_process(void *arg)
{
	uartStateMachine = MS_UART_SM_SELFCHECK;
	g_sn_0x14_got_flag = false;
	int count = 0;
	int ret;
	do{
		ms_uart_fw_ask_for_sn(0);
		usleep(100000);
		printf("send ask for sn\n");
		if(g_sn_0x14_got_flag){
			break;
		}
	}while(g_sn_0x14_got_flag != true && (count++ < 100));
	if(count >= 100){
		printf("get sn failed !!!\n");
		system("aplay /oem/factorytest/tts/sn_fail.wav");
		return;
	}else{
		//system("aplay /oem/factorytest/tts/sn_pass.wav");
		ms_ai_0x64_process(NULL);
	}
	//ret = ms_system("/oem/factorytest/ftest"); //delete by 926 220190626
	ret = ms_system("/mnt/app/factorytest/ftest");
	if(OK == ret){
		if(deamon_pid > 0){
			kill(deamon_pid,SIG_SYS_EVENT_START_MSPEECH);
		}
	}
	uartStateMachine = MS_UART_SM_IDLE;

	pthread_exit(NULL);

}


void ms_uart_0x14_process(uint8_t *buff)
{
	#if defined(WITH_FACTORY_TEST)
	printf("====================================\n");
	printf("factory test mode\n");
	printf("====================================\n");
	uint8_t aIp[4] = {0};
	if(deamon_pid){
		kill(deamon_pid,SIG_SYS_EVENT_STOP_MSPEECH);
	}
	//system("aplay /oem/factorytest/tts/ft.wav");
	system("aplay /mnt/app/factorytest/tts/ft.wav");
	#if 0
		int signal = 0;
		while(count++ < 3){
			/* wpa_cli scan_result return NULL sometime */
			signal = get_wifi_signal("Smart-AI5");
			if(signal){
				break;
			}
		}

		if(signal == 0  || signal > 75 ){
			printf("weak signal\n");
			return;
		}
		else{
			printf("normal signal\n");
		}
	#else
		pthread_t tid;
		int ret;

		ret = pthread_create(&tid, NULL, factory_test_process, NULL);
		if(ret < 0){
			//system("aplay /oem/factorytest/tts/sn_fail.wav");
			system("aplay /mnt/app/factorytest/tts/sn_fail.wav");
			return;
		}
		pthread_detach(tid);

	#endif
	#endif
}
extern int wakelock_fd;
ms_result_t ms_device2uart_cmd_process(uint8_t cmd_type, uint8_t *buff)
{
	switch(cmd_type)
	{
		case MS_UART_SN_ACQUIRE:
		{
			ms_uart_0x07_process(buff);
			break;
		}

		case MS_UART_AIRCONDITION_GET_INFO:
		{
			ms_uart_0x65_process(buff);
			break;
		}

		case MS_UART_DEV_SERVICE_QUERY:
		{
			ms_uart_0xE1_process(buff);
			break;
		}

		case MS_UART_GET_MAC:
		{
			ms_uart_0x13_process(buff);
			break;
		}

		case MS_UART_ENTER_SELF_DETECT_MODE:
		{
			ms_uart_0x14_process(buff);
			break;
		}

		case MS_UART_DEV_QUERY_NETWORK_STATE:
		{
			ms_uart_0x63_process(buff);
			break;
		}


		case MS_UART_AIRCONDITION_RESTORE:
		{
			ms_uart_0x64_process(buff);
			break;
		}


		case MS_UART_CONFIG_MODULE_ROUTER:
		{
			ms_uart_0x6a_process(buff);
			break;
		}

		case MS_UART_SWITCH_MODULE_MODE:
		{
			ms_uart_0x81_process(buff);
			break;
		}

		case MS_UART_REBOOT_WIFI:
		{
			ms_uart_0x82_process(buff);
			break;
		}
		case MS_UART_RESTORE_WIFI_CFG:
		case MS_UART_UNBIND:
		{
			ms_uart_0x83_process(buff);
			break;
		}


		case MS_UART_WIFI_VERSION_QUERY:
		{
			ms_uart_0x87_process(buff);
			break;
		}
		case MS_UART_EXT_AUDIO_PALY:
		{
            MS_DBG_TRACE("MS Playing command");
			break;
		}
		case MS_UART_EXT_WDG_REBOOT:
		{
			kill(deamon_pid,SIG_UART_EVENT_WDG);
			break;
		}
		default:
		    MS_DBG_TRACE("uart cmd %02x invalid", cmd_type);
	}
    return MS_RESULT_SUCCESS;
}

void ms_uart_common_data_process(T_MsTransStream *stream)
{
	ms_uart_msg_head_t *msg_buf = NULL;

	status_t ret = ms_valid_msg((uint8_t *)stream->data, stream->size);
    if(OK == ret){
	    msg_buf = (ms_uart_msg_head_t *)stream->data;
	    ms_hal_hw_uart_write(g_serial_fd, (uint8_t *)stream->data, msg_buf->msg_length+1);//Add The first 0xAA Sync code
    }else{
	MS_ERR_TRACE("Wrong uart message");
    }
}


void uart_up_msg_process(T_MsTransStream *stream)
{
	uint8_t cmd_type = stream->data[9];

	MS_TRACE("uart recv: 0x%02x", cmd_type);
	printf("uart recv: 0x%02x\n", cmd_type);
	PRINT_BUF(stream->data,stream->size);
	if(OK != ms_valid_msg(stream->data, stream->size))
	{
		MS_TRACE("uart check fail");
		printf("uart check fail\n");
		return ;
	}
	if((cmd_type == MS_UART_CTL_CMD)||
	   (cmd_type == MS_UART_RENT_CTL_CMD)||
	   (cmd_type == MS_UART_QUERY_CMD)||
	   (cmd_type == MS_UART_REPORT_RUNNING_PARA_NOACK)||
	   (cmd_type == MS_UART_REPORT_RUNNING_PARA_ACK)||
	   (cmd_type == MS_UART_REPORT_ERROR_NOACK)||
	   (cmd_type == MS_UART_QUERY_DEV_INFO)||
	   (cmd_type == MS_UART_SYNCHRONIZE_TIME)||
	   (cmd_type == MS_UART_REPORT_ERROR_ACK))
	{
		if(cmd_type == MS_UART_QUERY_DEV_INFO)
		{
		    ms_uart_0xA0_process(stream->data);
		}
		if(cmd_type == MS_UART_SYNCHRONIZE_TIME)/*0x61*/
		{
                    ms_uart_0x61_process(stream->data);
		}
		if((cmd_type == MS_UART_REPORT_RUNNING_PARA_NOACK)||
		   (cmd_type == MS_UART_REPORT_ERROR_NOACK)){

			stream->event = MS_UART_EVENT_REPORT_NOACK;
			ms_send_msg_to_msmart_fifo(stream);
		}
		if((cmd_type == MS_UART_REPORT_RUNNING_PARA_ACK)||
		   (cmd_type == MS_UART_REPORT_ERROR_ACK)){

			stream->event = MS_UART_EVENT_REPORT_ACK;
			ms_send_msg_to_msmart_fifo(stream);

		}
		   if((cmd_type == MS_UART_QUERY_CMD)||
		   (cmd_type == MS_UART_CTL_CMD))
		{
			stream->event = MS_SYS_EVENT_TRANSDATA;
			ms_send_msg_to_msmart_fifo(stream);
		}
	}else
	{
		ms_device2uart_cmd_process(stream->data[9], stream->data);/* handle cmd which is from device to wifi module */ //9: cmd byte.
	}
}

/*
    fd:  0-NET, 1-LOCAL_NET, 2-SPEECH
*/
void uart_down_msg_process(int fd, T_MsTransStream *stream)//handle with queue msg.
{
	switch((uint8_t)stream->event)
	{
		/************ lan -> uart ***************/
		/************ wlan -> uart ***************/
		case MS_SYS_EVENT_TRANSDATA:
		case MS_UART_EVENT_REPORT_ACK:
		{
			MS_DBG_TRACE("trans[src task: %d fd:%d len:%d]", stream->event, fd, stream->size);
            _set_event_flag(fd, stream);
			ms_uart_common_data_process(stream);
			break;
		}

		/************ wlan -> uart ***************/
		case MS_UART_EVENT_SYN_TIME:
		{
			ms_uart_syn_time_report((uint8_t *)stream->data, stream->size);
			break;
		}

		/************ main -> uart ***************/
		case MS_SYS_EVENT_SYS_STATUS_CHANGED:
		{
			ms_sys_status_t sys_status;
			CLR_STRUCT(&sys_status);
			ms_hal_read_msmart_config_info(MS_SYS_STATUS_PATH,(uint8_t *)&sys_status, sizeof(ms_sys_status_t));
			ms_uart_sys_status_report(&sys_status);//debug by xiejianjun
		    break;
		}
		/************ invalid -> uart ***************/
		default:
		{
		    MS_TRACE("main-queue-received-event-type: 0x%02x", (uint8_t)stream->event);
		    MS_TRACE("shouldn't see me!");
		    break;
		}
	}
}
