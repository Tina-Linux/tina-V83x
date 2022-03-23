#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include<errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mount.h>
#include<sys/stat.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <sys/vfs.h>
#include <time.h>
#include <sys/time.h>

#include "ap_uart.h"

#define DISPATCHER_MGR_THREAD_PRIORITY 25
static pthread_mutex_t dispatcher_mgr_lock;
pthread_t recv_thread_id;
pthread_t send_thread_id;
pthread_attr_t dispatcher_mgr_thread_attr;

#define  LCD2MCU_PACKAGE_MAX 32
#define  MCU2LCD_PACKAGE_MAX 60

#define  LCD2MCU_TYPE_PWM 0x01
#define  LCD2MCU_TYPE_RF  0x02

#define  LCD2MCU_STATUS_IDLE1  0x11
#define  LCD2MCU_STATUS_IDLE2  0x22
#define  LCD2MCU_STATUS_START  0x33
#define  LCD2MCU_STATUS_CHECK  0x44
#define  LCD2MCU_STATUS_OK     0x55
#define  LCD2MCU_STATUS_DROP   0x66

#define MSG_QUE_KEY 1024
/*define the struct of the mes_que*/
#define MSG_TYPE 17

#define DATA_LENGTH_MAX 60
#define PROTOCAL_LENGTH 4
#define BUFFER_SIZE 32

typedef struct
{
    char msg_type;
    unsigned char msg_len;
    unsigned char msg_buf[DATA_LENGTH_MAX + PROTOCAL_LENGTH - 3];
}msg_que;

typedef struct
{
    int port_fd;
    unsigned char buf[BUFFER_SIZE];
}recv_param;

unsigned char lcd_rx_data[MCU2LCD_PACKAGE_MAX];
unsigned char lcd_tx_data[LCD2MCU_PACKAGE_MAX];
unsigned char g_exit_flag = 0;

void  *tsk_run(void *arg)
{
    /*use to data send*/
    unsigned char data_send_length = 0;
    unsigned char data_send_array[32] = {0};
    unsigned char num = 0;

    /*use to data proc*/
    unsigned char package_data_ptr[DATA_LENGTH_MAX + PROTOCAL_LENGTH - 3] = {0};
    unsigned char data_ptr[DATA_LENGTH_MAX] = {0};
    unsigned char package_data_length = 0;

    /*proc the sub package*/
    unsigned char sub_package_num = 0;
    unsigned char sub_package_flag = 0;
    unsigned char sub_package_ID = 0;
    unsigned char sub_package_count = 0;
    unsigned char sub_package_buf[DATA_LENGTH_MAX * 8] = {0};
    int cur_time = 0;
    int last_time = 0;

	msg_que msg_que_info;
    char ret = 0;
    int msg_que_id;

    memset(&msg_que_info, 0, sizeof(msg_que_info));

    while (1)
    {
        /*judge the exit flag*/
        if(1 == g_exit_flag)
        {
            printf("int tsk_run,g_exit_flag is:%4d\n", g_exit_flag);
            break;
        }
		sleep(1);
        /*check the msg que is exist*/
        msg_que_id = msgget(MSG_QUE_KEY, IPC_EXCL);
        if (msg_que_id < 0)
        {
            printf("msg que is not exist!\n");
            continue;
        }
        else
        {
            printf("\nin tsk_run,msg_que_id is %4d\n", msg_que_id);
        }
        /*start to recv data from the msg que*/
        ret = msgrcv(msg_que_id, &msg_que_info, sizeof(msg_que_info), 0, 0);
        if (ret < 0)
        {
            printf("recv data from the msg que failed!\n");
            continue;
        }
        else
        {
            printf("recv data from the msg que success!\n");
        }/*end if*/

        /*get the cur time*/
        if (0 != last_time)
        {
            printf("*****last_time is %4d*****\n", last_time);
            cur_time = time((time_t*)NULL);
            printf("cur_time is :%4d\n", cur_time);
            if (cur_time - last_time > 10)
            {
                printf("cannot get the  complete package_data!!!\n");
                memset(sub_package_buf, 0, DATA_LENGTH_MAX * 8);
                sub_package_count = 0;
                last_time = 0;
                continue;
            }
        }/*end if*/

        package_data_length = msg_que_info.msg_len;
        memcpy(package_data_ptr, msg_que_info.msg_buf, msg_que_info.msg_len);
#if 1//DEBUG
        printf("package_data_length is %4d\n", package_data_length);
        unsigned char i = 0;
        while (i < package_data_length)
        {
            printf("%02x ", package_data_ptr[i++]);
        }
        printf("\n");
#endif
    }
}

int data_process(unsigned char *data_proc_ptr, int package_data_length)
{
    /*use msg_que to do the ipc*/
    msg_que msg_que_info;
    char ret = 0;
    int msg_que_id;

    memset(&msg_que_info, 0, sizeof(msg_que_info));
    msg_que_id = msgget((key_t)MSG_QUE_KEY, IPC_EXCL);
	if(msg_que_id < 0)
		return -1;

   /*get a complete package data and send it to the msg_que*/
 	msg_que_info.msg_type = MSG_TYPE;
    msg_que_info.msg_len = package_data_length;

#if DEBUG
   	printf("msg_que_info.msg_len is %4d\n",
                    msg_que_info.msg_len);
	unsigned char i = 0;
	while (i < package_data_length) {
		printf("%02x ", data_proc_ptr[i++]);
	}
	printf("\n");
#endif

 	memcpy(&(msg_que_info.msg_buf), data_proc_ptr,
                    package_data_length);
#if 0

 	/*send the msg*/
 	ret = msgsnd(msg_que_id, &msg_que_info, package_data_length, IPC_NOWAIT);
 	if (ret < 0)
  	{
  		printf("msg send failed!\n");
		return -1;
  	} else {
  		printf("send msg success! ret is %4d\n", ret);
 	}
#endif
	return ret;
}

#if 0
void uart_send_process(unsigned char *out, unsigned char *in, unsigned char size)
{
 	unsigned char i, j, sum;
  	unsigned int pic_id;
	unsigned int lcd_tx_total;

	if (lcd_tx_start)
		return;

  	if ((lcd_tx_time < 100) && (lcd_tx_ack == 0) && (lcd_tx_nak == 0))
		return;

  	lcd_tx_time = 0;

  	for (i = 0; i < LCD2MCU_PACKAGE_MAX; i++) {
    	lcd_tx_data[i] = 0x00;
  	}

  	if (lcd_tx_ack) {
      	lcd_tx_ack = 0;
      	lcd_tx_data[0] = LCD2MCU_HEAD1;
      	lcd_tx_data[1] = REPLY_OK;
      	uart_write(lcd_tx_data, 2);
   	}
   	else if (lcd_tx_nak) {
      	lcd_tx_nak = 0;
      	lcd_tx_data[0] = LCD2MCU_HEAD1;
      	lcd_tx_data[1] = REPLY_ERR;
      	lcd_tx_total = 2;
      	uart_write(lcd_tx_data, 2);
   	}
   	else if (lcd_tx_flag) {
  		switch(lcd_tx_flag) {
     		case CMD_BASE_DATA:
	 			lcd_basedata_tx_deal();
				break;
			default:
				break;
 		}
		lcd_tx_total = lcd_tx_data[2];

  		sum = 0;
  		for (i = 0; i < LCD2MCU_PACKAGE_MAX - 1; i++){
    		sum += lcd_tx_data[i];
		}
 		lcd_tx_data[LCD2MCU_PACKAGE_MAX-1] = sum;
  		uart_write(lcd_tx_data, LCD2MCU_PACKAGE_MAX);

 		lcd_tx_repeat_cnt++;
   		if (lcd_tx_repeat_cnt >= 3) {
        	lcd_tx_repeat_cnt = 0;
        	lcd_tx_flag = 0;
   		}
  }

}

void uart_recive_process(unsigned char *recv_data, unsigned int recv_data_len)
{
	unsigned int pic_id;
	unsigned char i, sum;

  	if(lcd_rx_finish==0)
  		return;

  	lcd_rx_finish=0;

    sum = 0;
   	for (i = 0; i < recv_data_len - 1; i++)
    {
    	sum += recv_data[i];
  	}

   	if (recv_data[recv_data_len - 1] != sum)
    {
      	return;
   	}

 	pic_id=((unsigned int)recv_data[3]<<8)+recv_data[4];

     if(recv_data[5]==0){
      	switch(pic_id) {
			case CMD_BASE_DATA:
             	break;
      		case CMD_POINT_CHECK_1:
				break;
      		case CMD_POINT_CHECK_2:
				break;
      		case CMD_TRY_RUN:
              	break;
      		case CMD_OPTION_SET:
              	break;
      		case CMD_ROOM_INFO:
				break;
      		case CMD_ADDRESS_CHG:
              	break;
      		case CMD_CONFIRM_RUN:
              	break;
      		case CMD_ROOM_ADR_REASIGN:
				break;
      		case CMD_ROOM_INIT:
				break;
      		case CMD_BOARD_CHECK:
				break;
      		case CMD_SELF_CHECK:
				break;
      		case CMD_PREHEART_OVER:
				break;
      		case CMD_IO_OUTPUT:
                  break;
      		case CMD_STANDARD_TIME:
				break;

      		case CMD_OFFDAY_1:
				break;
      		case CMD_OFFDAY_2:
				break;
      		case CMD_OFFDAY_3:
				break;

			case CMD_SCHEDULE_01:
				break;
			case CMD_SCHEDULE_02:
				break;
			case CMD_SCHEDULE_03:
				break;
			case CMD_SCHEDULE_04:
				break;
			case CMD_SCHEDULE_05:
				break;
			case CMD_SCHEDULE_06:
				break;
			case CMD_SCHEDULE_07:
				break;

   			case CMD_ERR_01:
				break;
      		case CMD_ERR_02:
        		break;
			case CMD_ERR_03:
				break;
			case CMD_ERR_04:
				break;
	    	case CMD_ERR_05:
				break;
	    	case CMD_ERR_06:
	 			break;
	    	case CMD_ERR_07:
				break;
	    	case CMD_ERR_08:
				break;
			case CMD_ERR_09:
				break;
	    	case CMD_ERR_0A:
				break;
      		default:
        		break;
      	}
     } else {
		printf("Recive write cmd from MCU !!!\n");
     }
}

void lcd_basedata_tx_deal(void)
{
 	  unsigned char i,j,i1,j1;

      lcd_tx_data[0]=LCD2MCU_HEAD1;
      lcd_tx_data[1]=LCD2MCU_HEAD2;
      lcd_tx_data[2]=0x20;
      lcd_tx_data[3]=0x10;
      lcd_tx_data[4]=0x01;
      lcd_tx_data[5]=0x00;

      if(system_status)lcd_tx_data[6]|=0x01;
      if(try_run_flag) lcd_tx_data[6]|=0x02;
      if(try_run_set)lcd_tx_data[6]=0x04;

      //mode_enable=0;
      lcd_tx_data[7]=mode_enable;
      lcd_tx_data[8]=system_mode;

      //wind_enable=0;
      lcd_tx_data[9]=wind_enable;
      lcd_tx_data[10]=system_wind;

      if((wind_enable<4)&&(system_wind==0x08))
      {
        lcd_tx_data[10]=0x14;
      }

      if((try_run_flag)||(try_run_set))
      {
        lcd_tx_data[11]=98;
        lcd_tx_data[12]=30;

        if(fre_machine)
			lcd_tx_data[13]=try_run_fre;
        else
			lcd_tx_data[13]=0;
      }
      else if(mode_enable==10)
      {
        lcd_tx_data[11]=99;
        lcd_tx_data[12]=0;
        i=pointcheck1_data[1];
        if(i>30)i=30;
        if(i<16)i=16;
        lcd_tx_data[13]=i<<1;
      }
      else if(system_mode==0x44)
      {
        lcd_tx_data[11]=90;
        lcd_tx_data[12]=35;
        lcd_tx_data[13]=dry_set;
      }
      else
      {
        lcd_tx_data[11] = tempset_max; if(lcd_tx_data[11] > 30)lcd_tx_data[11]=30;
        lcd_tx_data[12] = tempset_min; if(lcd_tx_data[12] < 16)lcd_tx_data[12]=16;
        lcd_tx_data[13] = tempset<<1;
        if(half_set)lcd_tx_data[13]|=0x01;

      }
      lcd_tx_data[14]=winddir_enable<<3;

      if(wind_mode==1)lcd_tx_data[14]|=0x01;
      if(wind_mode==2)lcd_tx_data[14]|=0x02;
      if(wind_board_set)lcd_tx_data[14]|=0x04;

          if(wind_mode==1)
          {
            i=1;j=1;
          }
          else
          {
            i=wind_board_status;j=wind_horizontal_status;
            if(a3d_wind_use)
            {
               i1=wind_board_angle;j1=wind_horizontal_angle;
            }
            else
            {
              i1=wind_board_angle;j1=wind_horizontal_angle;
            }
          }


      lcd_tx_data[15]&=0xf0;
      if(i)
      {
           lcd_tx_data[15]|=0x0e;
      }
      else
      {
           lcd_tx_data[15]|=(i1-1)<<1;

      }

      lcd_tx_data[15]&=0x0f;
      if(j)
      {
           lcd_tx_data[15]|=0xe0;
      }
      else
      {
           lcd_tx_data[15]|=(j1-1)<<5;
      }


         if(wind_mode==1)
          {
            i=1;j=1;
          }
          else
          {
            i=wind_board2_status;j=wind_horizontal2_status;
            i1=wind_board2_angle;j1=wind_horizontal2_angle;
          }

      lcd_tx_data[16]&=0xf0;
      if(i)
      {
           lcd_tx_data[16]|=0x0e;
      }
      else
      {
           lcd_tx_data[16]|=(i1-1)<<1;

      }

      lcd_tx_data[16]&=0x0f;
      if(j)
      {
           lcd_tx_data[16]|=0xe0;
      }
      else
      {
           lcd_tx_data[16]|=(j1-1)<<5;
      }

      if(center_control_onoff)lcd_tx_data[17]|=0x01;
      if(center_control_mode)lcd_tx_data[17]|=0x02;
      if(center_control_wind)lcd_tx_data[17]|=0x04;
      if(center_control_windboard)lcd_tx_data[17]|=0x08;
      if(center_control_tempset)lcd_tx_data[17]|=0x10;

      if((center_onoff_flag)||((center_control_onoff)&&(system_status)))
        lcd_tx_data[17]|=0x20;

      if(center_control_flag)lcd_tx_data[17]|=0x40;

      if(system_mode_canntchange)lcd_tx_data[18]|=0x01;
      if((system_wind_canntchange)||((ignore_flag&0x7f)==0x72))lcd_tx_data[18]|=0x02;

      if((tempset_canntchange)||((mode_enable==10)))lcd_tx_data[18]|=0x04;

      if(por_06_flag)lcd_tx_data[18]|=0x80;
      if(filter_flag)lcd_tx_data[19]|=0x01;
      if(heating_flag)lcd_tx_data[19]|=0x10;
      if(defrost_flag)lcd_tx_data[19]|=0x20;

      if(((heating_start)||(compress_preheat_flag))&&(option_h2==0)&&(system_status)&&((system_mode==0x10)||(system_mode==0x28)))lcd_tx_data[19]|=0x80;
      lcd_tx_data[20]=0;
      if(heat_used)lcd_tx_data[20]|=0x40;
      if(save_used)lcd_tx_data[20]|=0x20;
      if(health_used)lcd_tx_data[20]|=0x10;

       if((sleep_used)||(human_used)||(health_used)||(save_used)||(heat_used)||(human_sensor_used))
        lcd_tx_data[20]|=0x80;
      lcd_tx_data[21]=0;
      if((heat_flag)&&((heat_used)))lcd_tx_data[21]|=0x40;
      if((save_flag)&&(save_used))lcd_tx_data[21]|=0x20;
      if((health_flag)&&(health_used))lcd_tx_data[21]|=0x10;
      lcd_tx_data[22]=0;
      if(human_used)
      {
        switch(human_flag)
        {
        case 0:i=0;break;
        case 1:i=0x08;break;
        case 2:i=0x04;break;
        case 3:i=0x02;break;
        case 4:i=0x0a;break;
        case 5:i=0x06;break;
        default:i=0;break;
        }
        lcd_tx_data[22]=i;
        lcd_tx_data[22]|=0x10;
        if(radiatevt_flag)lcd_tx_data[22]|=0x01;
      }

      if(human_sensor_used)
      {
        lcd_tx_data[22]|=0x80;
        switch((human_sensor>>1)&0x03)
        {
        case 0x00:break;
        case 0x01:lcd_tx_data[22]|=0x20;break;
        case 0x02:lcd_tx_data[22]|=0x40;break;
        }

      }
      lcd_tx_data[23]=0;
      if(sleep_used)
      {
        switch(sleep_flag)
        {
        case 0:i=0;break;
        case 1:i=1;break;
        case 2:i=2;break;
        case 3:i=4;break;
        }
        lcd_tx_data[23]=i;
        lcd_tx_data[23]|=0x08;
      }


      if(human_sensor_used)
      {
        switch((human_sensor>>3)&0x07)
        {
        case 0x00:break;

        case 0x01:lcd_tx_data[23]|=0x10;break;
        case 0x02:lcd_tx_data[23]|=0x20;break;
        case 0x03:lcd_tx_data[23]|=0x30;break;
        case 0x04:lcd_tx_data[23]|=0x40;break;
        }


        if(human_sensor&0x01)lcd_tx_data[23]|=0x80;

      }

      if((pre_on_flag)&&(system_status==0))
      {
        lcd_tx_data[24]=pre_on_time<<1;
        lcd_tx_data[24]|=0x01;
      }
      if((pre_off_flag)&&(system_status))
      {
        lcd_tx_data[25]=pre_off_time<<1;
        lcd_tx_data[25]|=0x01;
      }
      lcd_tx_data[26]=total_machine<<4;

      if(schedule_used)lcd_tx_data[26]|=0x08;

      if((fre_machine)&&(system_mode!=0x08))lcd_tx_data[26]|=0x02;
      if(schedule_flag&0x80)lcd_tx_data[26]|=0x01;
      if((err_now_flag)&&(err_reset_flag==0))lcd_tx_data[27]|=0x01;
      if(err_reset_flag)lcd_tx_data[27]|=0x02;
      if(uart_connect_err_flag)lcd_tx_data[27]|=0x04;
      if(initialize_flag)lcd_tx_data[27]|=0x08;

      if(wakeup_flag)lcd_tx_data[27]|=0x20;
      wakeup_flag=0;

      if(line_control_double)lcd_tx_data[27]|=0x40;
      if(hh_flag)lcd_tx_data[27]|=0x80;

      lcd_tx_data[28]=ignore_flag;

      lcd_tx_data[29]=power_led_pwm;

      lcd_tx_data[30]=lcd_optionset;


      if(restart_flag)
	  	lcd_tx_data[31]=1;

      lcd_tx_data[32]=0;
}
#endif

void *data_recv(void *data_recv_info_ptr)
{
	recv_param *data_recv;
	data_recv = (recv_param *)data_recv_info_ptr;
    char port_fd = data_recv->port_fd;
    unsigned char *data_recv_buf_ptr = data_recv->buf;
#if DEBUG
    printf("port_fd is %4d\n", data_recv->port_fd);
#endif
    int pos = 0;
    int len = 0;
    unsigned char data_new_flag = 0;

    while(1)
    {
        /*judge the exit flag*/
        if (1 == g_exit_flag)
        {
            printf("in data_recv thread,g_exit_flag%4d\n", g_exit_flag);
            break;
        }

#if DEBUG
        printf("pos is %4dlen is %4d\n", pos, len);
#endif
		usleep(10000);
		len = uart_read(port_fd, &data_recv_buf_ptr[pos], (BUFFER_SIZE - pos));
        //len = read(port_fd, &data_recv_buf_ptr[pos], (BUFFER_SIZE - pos));
        printf("len is %d, data =%s\n", len, data_recv_buf_ptr);
        if (len > 0)
        {
            pos += len;
            data_new_flag = 1;
			printf("len is %d, data =%s\n", len, data_recv_buf_ptr);
            continue;
        }
        else if (0 == data_new_flag && 0 == len)
        {
            printf("no new data come......\n");
            continue;
        }

        /* receiving data */
        if (((0 == len) && (pos < BUFFER_SIZE) &&(1 == data_new_flag))
			|| (BUFFER_SIZE == pos))
        {
			printf("start to process\n");
            /*start to process data*/
            if (data_process(data_recv_buf_ptr, pos) < 0)
				printf(">>>data process failed\n");
			memset(data_recv_buf_ptr, 0, pos);
            data_new_flag = 0;
			pos = 0;
        }
    }
}

int dispatcher_mgr_init(void)
{
	int ret = 0;
	struct sched_param priorityParam;
	recv_param data_recv_info;
    /*create a msg_que*/
    msg_que msg_que_info;
    int msg_que_id;
    key_t key;

	printf("Entering %s init ...\n", __func__);

	pthread_attr_init(&dispatcher_mgr_thread_attr);
	pthread_attr_setschedpolicy(&dispatcher_mgr_thread_attr, SCHED_RR);
	priorityParam.sched_priority = DISPATCHER_MGR_THREAD_PRIORITY;
	pthread_attr_setschedparam(&dispatcher_mgr_thread_attr, &priorityParam);
	pthread_attr_setinheritsched(&dispatcher_mgr_thread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_mutex_init(&dispatcher_mgr_lock, NULL);

    /* create message queue */
	key = (key_t)MSG_QUE_KEY;
    msg_que_id = msgget(MSG_QUE_KEY, IPC_EXCL);
    if (msg_que_id <= 0) {
        /*create the msg_que*/
        printf("the msg que is not exist!\n");
        msg_que_id = msgget(key, IPC_CREAT | 0666);
        if (msg_que_id < 0) {
            printf("create the msg que failed!\n");
            return -1;
        } else {
            printf("create the msg que success,and the msg_que_id is:%4d\n",
                    msg_que_id);
        }
    }

	data_recv_info.port_fd = uart_open();
	ret = pthread_create(&recv_thread_id, &dispatcher_mgr_thread_attr, data_recv, &data_recv_info);
	if(ret != 0) {
		printf("create recive pthread failed!\n");
		ret = -3;
		goto errHdl;
	}

	ret = pthread_create(&send_thread_id, &dispatcher_mgr_thread_attr, tsk_run, NULL);
	if(ret != 0) {
		printf("create recive pthread failed!\n");
		ret = -3;
		goto errHdl;
	}

errHdl:
	return ret;
}

int dispatcher_mgr_uninit(void)
{
	int ret = 0;
	int msg_que_id = 0;

	printf("Entering %s uninit ...\n", __func__);

    /*free the related system resource*/

    msg_que_id = msgget(MSG_QUE_KEY, IPC_EXCL);
    if (msg_que_id < 0) {
        printf("msg que is not exist!\n");
        return 0;
    } else {
        if(msgctl(msg_que_id, IPC_RMID, 0) < 0) {
            printf("delete msg_que failed!\n");
            return -1;
        } else {
            printf("delete msg_que: %4d success\n",msg_que_id);
            return 0;
        }
    }

	pthread_attr_destroy(&dispatcher_mgr_thread_attr);
	pthread_cancel(recv_thread_id);
	pthread_mutex_destroy(&dispatcher_mgr_lock);

errHdl:
	return ret;
}

