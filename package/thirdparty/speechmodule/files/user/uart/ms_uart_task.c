/*
 * Copyright (c) 2017 M-SMART of Midea Group.
 * All rights reserved.
 *
 * File Name		: ms uart task
 * Introduction	:
 *
 * Current Version	: v0.1
 * Author			: Humble
 * Create Time	: 2018/11
 * Change Log		: create this file
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */
#include <ms_common.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "poll.h"
#include "ms_uart_process.h"

#define MS_UART_MONITOR_THREADS_TIME	(60 * 1000) // 1 min

/******************************************************************
				GLOBAL	VARABLE	 DEFINATE
*******************************************************************/
int deamon_pid = 0;
extern int g_serial_fd;
uint8_t soft_version[9]={MS_HW_PLATFORM, MS_HW_BIG_VERSION, MS_HW_SMALL_VERSION, MS_HW_FUNC_VERSION, MS_SW_VERSION_Y, MS_SW_VERSION_M, MS_SW_BIG_VERSION, MS_SW_SMALL_VERSION, MS_SW_FUNC_VERSION};
ms_stored_info_t ms_needed_stored_info;
// ringbuffer globle var
static uint8_t ms_uart_ring_buff[512];
static uint16_t uart_recv_total_len=0;
#define MS_FW_UART_BUF_LENGTH				255
#define MS_MIX_UART_BUG_LEN                 10
pthread_rwlock_t rwlock; //读写锁，用来保护 ms_uart_ring_buff

#ifdef HARD_VAD
int wakelock_fd = 0;
#endif

void ms_get_module_version(unsigned char *p_in, int len)
{
	if(len > 9)
	{
		memcpy(p_in, soft_version, sizeof(soft_version));
	}
	else
	{
		memcpy(p_in, soft_version, len);
	}
}

static void ms_printf_sw_version(void)
{
      uint8_t week=0;

//	MS_TRACE("\r\n**************************************");
	MS_TRACE("\r\n*************|  ATHENA  |*************");
//	MS_TRACE("**************************************");

       MS_TRACE("*** sw  :");

      //soft_version[5] = (ms_needed_stored_info.server_flag == SERVER_UNOFFICIAL) ? 0x00 : MS_SW_VERSION_5;

	for(week=0;week<9;week++)
	{
            printf("%02x.",soft_version[week]);
	}
	printf("\n");

	MS_TRACE("*** build: %s %s",  __DATE__, __TIME__);
	MS_TRACE("**************** ^_^ *****************\r\n");
}

static int write_version_file(){
    char version[30] = {0};
    snprintf(version,sizeof(version),"%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
								MS_HW_PLATFORM,
								MS_HW_CATEGORY,
								MS_HW_BIG_VERSION,
								MS_HW_SMALL_VERSION,
								MS_HW_FUNC_VERSION,
								MS_SW_VERSION_Y,
								MS_SW_VERSION_M,
								MS_SW_BIG_VERSION,
								MS_SW_SMALL_VERSION,
								MS_SW_FUNC_VERSION);
	printf("version:%s\n",version);
    //if(-1 == msm_write_config("/data/cfg/module_version",version, sizeof(version))) //delete by 926 @20190619
    if(-1 == msm_write_config("/mnt/app/cfg/module_version",version, sizeof(version)))
    {
		MS_TRACE("stored info fail");
    }
    return 0;
}
//ringbuffer function : start
static void ms_fill_to_ring_buf(uint8_t *buf,uint16_t len)
{
    static uint16_t offset = 0;
    uint16_t len_tp=0;

    pthread_rwlock_wrlock(&rwlock);//请求写锁

    uart_recv_total_len += len;

    if((offset+len) > sizeof(ms_uart_ring_buff))
    {
        len_tp = sizeof(ms_uart_ring_buff)-offset;
        memcpy(ms_uart_ring_buff+offset,buf,len_tp);
        offset = 0;
        memcpy(ms_uart_ring_buff, buf+len_tp, len-len_tp);

        offset +=(len-len_tp);
    }
    else
    {
        memcpy(ms_uart_ring_buff+offset,buf,len);
        offset +=len;
    }
    pthread_rwlock_unlock(&rwlock);//解锁
}

static int8_t ms_get_date_from_ring_buf(uint8_t *in_buf,uint16_t in_len)
{
   static uint16_t offset=0;
   uint16_t len_tp = 0;

   pthread_rwlock_rdlock(&rwlock);//请求读锁

   if(in_len > uart_recv_total_len)
   {
     pthread_rwlock_unlock(&rwlock);//解锁
     return -1;
   }
   else
   {
        uart_recv_total_len -= in_len;

        if(in_len+offset > sizeof(ms_uart_ring_buff))
        {
            len_tp = sizeof(ms_uart_ring_buff)-offset;
            memcpy(in_buf,ms_uart_ring_buff+offset,len_tp);
            offset = 0;
            memcpy(in_buf+len_tp,ms_uart_ring_buff,in_len-len_tp);
            offset += (in_len-len_tp);
        }
        else
        {
            memcpy(in_buf,ms_uart_ring_buff+offset,in_len);
            offset += in_len;
        }
   }
   pthread_rwlock_unlock(&rwlock);//解锁

   return 0;
}

static int8_t ms_uart_getpack(uint8_t *buff,uint16_t *in_len)
{
    uint16_t len_tp = 0;
    static uint8_t step=0;
    static uint32_t start_time=0;
    static uint8_t buf_tp[2]={0,0};

    while(1)
    {
        switch(step)
        {
            case 0:
                    while(1)
                    {
                        buf_tp[0]=0;
                        buf_tp[1]=0;
                        if(ms_get_date_from_ring_buf(buf_tp,1) != 0)
                        {
				*in_len = 0;
                            return -1;
                        }

                        if(*buf_tp != 0xaa)
                        {
                            continue;
                        }
                        else
                        {
                            step = 1;
                            break;
                        }
                   }
                   break;

            case 1:
                    if(ms_get_date_from_ring_buf(buf_tp+1,1) != 0)
                    {
                        step = 0;
                        return -1;
                    }
                    else
                    {
                        if(*(buf_tp+1) < 10 ||*(buf_tp+1) > *in_len )
                        {
                             step = 0;
                            continue;
                        }
                        else
                        {
                            step = 2;
                            start_time = msm_timer_get_systime_ms();
                            break;
                        }
                    }
                    break;

            case 2:
                     if(ms_get_date_from_ring_buf(buff+2,buf_tp[1]-1) != 0)
                     {
                        //if(compare_time(start_time,ms_hal_os_tickcount_get(), 1000) == 1)
                        if((msm_timer_get_systime_ms()-start_time) > 1000)
                        {
				// timeout (1s)
				// printf("uart wait...\r\n");
                            step = 0;
                            return -1;
                        }
                        else
                        {
				// printf("compare return\r\n");
                            return 1;
                        }
                     }
                     else
                     {
                         step = 0;
                         buff[0]=buf_tp[0];
                         buff[1]=buf_tp[1];
                         *in_len = buf_tp[1] + 1;
                        return 0;
                     }
                    break;

            default:
                    break;

        }
   }

    return 0;
}


//数据下行线程
void *uart_down_msg(void){

	int fd_net_out = -1;
	int fd_localnet_out = -1;
	int fd_maispeech_out = -1;
    int ne, nevents, ret, fd;

    int mEpollFd = epoll_create(10);
    struct epoll_event events[10];
    struct epoll_event  ev;
	int readcnt;
	T_MsTransStream  transtream;

	MS_TRACE("uart down msg start ...");
	printf("uart down msg start ...\n");

    memset(events,0,sizeof(events));
	/*
	Because pipes provide only a single unidirectional channel (not a separate bidirectional channel for each client like a socket),
	they are normally used when you have just one process that needs to send data to only one other process.
	When the writer closes the pipe, POLLHUP (hangup) tells the reader that the pipe is closed and it can finish processing and terminate.

	It is possible to use a pipe with multiple writers but you will need to be careful if the messages
	could be larger than PIPE_BUF or 512 bytes. Otherwise, because it is only a single channel,
	the messages from multiple writers writing at the same time could be interleaved.
	Also because it is a single channel you won't be able to tell whether a long message is a single write from
	one client or multiple writes from multiple clients, unless you have some convention
	like one line (terminated by a newline) per client message.

	POLLHUP indicates that the last writer has closed the pipe, and persists until another process opens
	the pipe for writing or it is closed by all readers. If you don't want this, open the pipe with
	O_RDWR instead of O_RDONLY so that the pipe will stay open. This works because then there
	will always be a writer (your program) as long as you have it open.
	*/

	if(-1 == (fd_net_out = open(MSMART_NET_OUT,O_RDWR | O_NONBLOCK))){
		MS_ERR_TRACE("/tmp/msmart_net_out OPEN ERROR");
		printf("/tmp/msmart_net_out OPEN ERROR\n");
		goto ERR3;
	}
	ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
	ev.data.fd = fd_net_out;
	while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_net_out, &ev) < 0 && errno == EINTR);

	if(-1 == (fd_localnet_out = open(MSMART_LOCAL_NET_OUT,O_RDWR|O_NONBLOCK))){
		MS_ERR_TRACE("/tmp/msmart_local_net_out OPEN ERROR");
		printf("/tmp/msmart_local_net_out OPEN ERROR\n");
		goto ERR2;
	}
	ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
	ev.data.fd = fd_localnet_out;
	while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_localnet_out, &ev) < 0 && errno == EINTR);

	if(-1 == (fd_maispeech_out = open(MAISPEECH_OUT,O_RDWR|O_NONBLOCK))){
		MS_ERR_TRACE("/tmp/maipseech_out OPEN ERROR");
		printf("/tmp/maipseech_out OPEN ERROR\n");
		goto ERR1;
	}
	ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
	ev.data.fd = fd_maispeech_out;
	while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_maispeech_out, &ev) < 0 && errno == EINTR);

	while(1){
		nevents = epoll_wait(mEpollFd, events, 1, 2000);  // 2s timeout
		if (nevents < 0) {
			//if (errno != EINTR){
				MS_TRACE("epoll_wait() unexpected error: %s", strerror(errno));
				printf("epoll_wait() unexpected error: %s", strerror(errno));
			//}
		continue;
        }

		for (ne = 0; ne < nevents; ne++) {
			if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
				#if 0
				fd = events[ne].data.fd;
				if(fd == fd_net_out){
					MS_TRACE("fd_net_out EPOLLERR or EPOLLHUP after epoll_wait() !?");
					close(fd_net_out);
					while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_net_out, NULL) < 0 && errno == EINTR);
					if(-1 == (fd_net_out = open(MSMART_NET_OUT,O_RDONLY | O_NONBLOCK))){
						MS_ERR_TRACE("/tmp/msmart_net_out OPEN ERROR");
						goto ERR2;
					}
					ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
					ev.data.fd = fd_net_out;
					while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_net_out, &ev) < 0 && errno == EINTR);

				}
				else if (fd == fd_localnet_out) {
					MS_TRACE("fd_localnet_out EPOLLERR or EPOLLHUP after epoll_wait() !?");
					close(fd_localnet_out);
					while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_localnet_out, &ev) < 0 && errno == EINTR);
					if(-1 == (fd_localnet_out = open(MSMART_LOCAL_NET_OUT,O_RDONLY|O_NONBLOCK))){
						MS_ERR_TRACE("/tmp/msmart_local_net_out OPEN ERROR");
						goto ERR1;
					}
					ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
					ev.data.fd = fd_localnet_out;
					while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_localnet_out, &ev) < 0 && errno == EINTR);
				}
				else if (fd == fd_maispeech_out) {
					MS_TRACE("fd_maispeech_out EPOLLERR or EPOLLHUP after epoll_wait() !?");
					close(fd_maispeech_out);
					while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_maispeech_out, &ev) < 0 && errno == EINTR);
					if(-1 == (fd_maispeech_out = open(MAISPEECH_OUT,O_RDONLY|O_NONBLOCK))){
						MS_ERR_TRACE("/tmp/maipseech_out OPEN ERROR");
						goto ERR0;
		}
					ev.events  = EPOLLIN |EPOLLERR|EPOLLHUP;
					ev.data.fd = fd_maispeech_out;
					while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd_maispeech_out, &ev) < 0 && errno == EINTR);
				}
				#endif
			}
			else if ((events[ne].events & EPOLLIN)) {
				fd = events[ne].data.fd;
				readcnt = read(fd, &transtream, sizeof(struct T_MsTransStream));
				if(readcnt < 0){
					MS_ERR_TRACE("READ DEVICE ERROR");
					printf("READ DEVICE ERROR\n");
					goto ERR0;
				}
				else {
					MS_TRACE("UART DOWN MSG fd:%d,len: %d  CONTENT:",fd, transtream.size);
					printf("UART DOWN MSG fd:%d,len: %d  CONTENT:\n",fd, transtream.size);
					PRINT_BUF(transtream.data, transtream.size);
					if(transtream.event == MS_UART_EVENT_AP_MODE){
						ms_ai_0x64_process(&transtream.data);
					}
					else{
						if(fd == fd_net_out){
							uart_down_msg_process(0, &transtream);
					}
						else if (fd == fd_localnet_out) {
							uart_down_msg_process(1, &transtream);
				}
						else if (fd == fd_maispeech_out) {
							uart_down_msg_process(2, &transtream);
            }
        }
	}
			}
		}
	}

ERR0:
	while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_maispeech_out, NULL) < 0 && errno == EINTR);
    if(fd_maispeech_out > 0) {
		close(fd_maispeech_out);
	}
ERR1:
	while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_localnet_out, NULL) < 0 && errno == EINTR);
	if(fd_localnet_out > 0){
	    close(fd_localnet_out);
	}
ERR2:
	while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd_net_out, NULL) < 0 && errno == EINTR);
    if(fd_net_out > 0) {
		close(fd_net_out);
	}
ERR3:
	return;
}

//串口读线程
void uart_read_msg_from_hw(void)
{
    struct epoll_event events[1];
    int ne, nevents, ret, fd;
    struct epoll_event  ev;
    int mEpollFd = epoll_create(1);

    int fd_rd;
    char data[MS_FW_UART_BUF_LENGTH];

    MS_TRACE("uart read start ...");
    printf("uart read start ...\n");

	//if(0 != (ret =ms_hal_hw_uart_open("/dev/ttyS1", 9600,&g_serial_fd))){ //delete by 926 220190619
	if(0 != (ret =ms_hal_hw_uart_open("/dev/ttyS2", 9600,&g_serial_fd))){
		MS_ERR_TRACE("UART OPEN ERROR");
		printf("UART OPEN ERROR");
		exit(-1);
	}

	if (g_serial_fd > 0){

        fcntl(g_serial_fd, F_SETFL, fcntl(g_serial_fd, F_GETFL) | O_NONBLOCK);
        ev.events  = EPOLLIN;
        ev.data.fd = g_serial_fd;
        while (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, g_serial_fd, &ev) < 0 && errno == EINTR);

        while (1){
            nevents = epoll_wait(mEpollFd, events, 1, 200);  // 20ms timeout
            if (nevents < 0) {
                if (errno != EINTR){
                    MS_TRACE("epoll_wait() unexpected error: %s", strerror(errno));
                    printf("epoll_wait() unexpected error: %s", strerror(errno));
                }
				usleep(50 * 1000);
                continue;
            }

            for (ne = 0; ne < nevents; ne++) {
                if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                    MS_TRACE("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                    printf("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                    break;
                }
                if ((events[ne].events & EPOLLIN) != 0) {
                    fd = events[ne].data.fd;
                    if (fd == g_serial_fd) {
                        while (1) {
				memset(data,0,sizeof(data));
                            ret = read(fd, data, MAX_UART_LEN);
                            if (ret < 0) {
                                if (errno == EINTR) {
                                    continue;
                                }
                                if (errno != EWOULDBLOCK) {
                                    MS_ERR_TRACE("error while read serial port socket: %d", strerror(errno));
                                    printf("error while read serial port socket: %d", strerror(errno));
                                }
                                break;
                            }
				MS_DBG_TRACE("uart hardware read buffer:");
				PRINT_BUF(data,ret);
                            ms_fill_to_ring_buf(data,ret);
                        }
                    } else {
                        MS_ERR_TRACE("epoll_wait() returned unkown fd %d ?", fd);
                        printf("epoll_wait() returned unkown fd %d ?", fd);
                    }
                }
            }
        }
    }

    while (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, g_serial_fd, NULL) < 0 && errno == EINTR);

    MS_TRACE("serial stoped");
    printf("serial stoped");
    ms_hal_hw_uart_close(g_serial_fd);
}
void init_oprate_license_area(){
	unsigned char init_bin[2048] = {0xFF};
	if(0 == access(MSMART_OP_LICENSE_FILE,F_OK)){
        MS_TRACE("LICENSE BACKUP AREA exsit, Do not need to create");
        printf("LICENSE BACKUP AREA exsit, Do not need to create\n");
	}else{
	    msm_flash_save_license_operation_info(init_bin,2048);
	}
}

void uart_up_msg(){
    int size;
    T_MsTransStream  transtream;
    MS_TRACE("uart up msg start ...");
    printf("uart up msg start ...\n");

	/* factory test mode */
	 if(-1 == deamon_pid){
		 memset(transtream.data, 0 ,sizeof(transtream.data));
		 transtream.size = 12;
		 char temp[] = {0xaa, 0x0b, 0xb1, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x76};
		 memcpy(transtream.data, temp, 12);
		 uart_up_msg_process(&transtream);
	 }

    while(1){
	usleep(50*1000);
        memset(transtream.data, 0 ,sizeof(transtream.data));
        size = MS_FW_UART_BUF_LENGTH;

        if(uart_recv_total_len>0)
        {
#ifdef HARD_VAD
            ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_LOCK);
#endif
		ms_uart_getpack(transtream.data,&size);
	    if(size>=MS_MIX_UART_BUG_LEN){
		MS_DBG_TRACE("Msmart uart buffer[len:%d]:", size);
		PRINT_BUF(transtream.data,size);
                transtream.size = size;
                uart_up_msg_process(&transtream);
	    }
#ifdef HARD_VAD
            ms_sent_wakelock_event(wakelock_fd,MS_UART_EVENT_WAKE_UNLOCK);
#endif
        }

    }
}

int main(int argc,char **argv){
	int ret =0;
	pthread_t uart_tid;
	pthread_t up_tid;
	pthread_t down_tid;

	printf("msmart_uart\n");
	pthread_rwlock_init(&rwlock, NULL);

    if(argc == 2){
        sscanf(argv[1],"%d\n",&deamon_pid);
    }

    init_oprate_license_area();
    printf("FPID:%d\n",deamon_pid);
    write_version_file();
#ifdef HARD_VAD
    wakelock_fd = init_wakelock_client();
#endif

	ms_printf_sw_version();
	MS_TRACE("UART Process begin");

	if(-1==(msm_read_config(MS_STORED_INFO,(char*)&ms_needed_stored_info,sizeof(ms_needed_stored_info)))){
		MS_TRACE("The globe configure file is not exist,and we will create it later:%s",strerror(errno));
	}

    mkfifo(MSMART_NET_OUT, 0777);
    mkfifo(MSMART_LOCAL_NET_OUT, 0777);
    mkfifo(MAISPEECH_OUT, 0777);
    mkfifo(MSMART_NET_IN, 0777);
    mkfifo(MSMART_LOCAL_NET_IN, 0777);
    mkfifo(MAISPEECH_IN, 0777);
    mkfifo(MSMART_WAKE_SOCKET_FILE, 0777);


    sleep(1);
	if(0 != (ret = pthread_create(&uart_tid,NULL,(void*)&uart_read_msg_from_hw,NULL))){
		MS_ERR_TRACE("uart_down_msg pthread create fail");
		exit(-1);
	}
	if(0 != (ret = pthread_create(&up_tid,NULL,(void*)&uart_up_msg,NULL))){
		MS_ERR_TRACE("uart_up_msg pthread create fail");
		exit(-1);
	}
	if(0 != (ret = pthread_create(&down_tid,NULL,(void*)&uart_down_msg,NULL))){
		MS_ERR_TRACE("uart_down_msg pthread create fail");
		exit(-1);
	}
	while(1){
		ms_uart_state_machine();
		sleep(1);
	}

#ifdef HARD_VAD
	close_wakelock_client(wakelock_fd);
#endif
	pthread_rwlock_destroy(&rwlock);
	pthread_join(uart_tid, NULL);
	pthread_join(up_tid, NULL);
	pthread_join(down_tid, NULL);
	return 0;
}
