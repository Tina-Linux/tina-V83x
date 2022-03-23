#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>
#include <stdint.h>
#include "log.h"
#include "app_uart.h"
#define LOGTAG "app_uart"

#define FALSE  -1
#define TRUE   0

/*******************************************************************
* Name：               UART0_Open
* Function：           Open uart and return file discript
* Input param：        fd: file discript port: port number(ttyS0,ttyS1,ttyS2)
* Output param：       ture:1，false:0
*******************************************************************/
int app_uart_open(int fd,const char* port)
{
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd) {
		perror("Can't Open Serial Port");
		return(FALSE);
	}
	/*set uart blocked*/
	if (fcntl(fd, F_SETFL, 0) < 0) {
		log_e("fcntl failed!\n");
		return(FALSE);
	} else {
		log_raw("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}
	/*Test whether terminal device*/
	if (0 == isatty(STDIN_FILENO)) {
		log_raw("standard input is not a terminal device\n");
		/*return(FALSE);*/
	} else {
		log_raw("isatty success!\n");
	}
	log_raw("fd->open=%d\n",fd);

	return fd;
}

/*******************************************************************
* Name：               UART0_Close
* Function：           Close uart
* Input param：        fd:file discript port: serial number(ttyS0,ttyS1,ttyS2)
* Output parm：        void
*******************************************************************/
void UART0_Close(int fd)
{
	close(fd);
}

/*******************************************************************
* Name：			UART0_Set
* Function:			Set data width, stop bit and verify bit
* Input param:		fd: uart file discript
*					speed	transfer speed
*					flow_ctrl	data control
*					databits	data width 7 or 8
*					stopbits	stop bit 1 or 2
*					parity		verify type N,E,O,,S
*Output param:		true:1, false:0
*******************************************************************/
int UART0_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
	int i;
	int status;
	int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
	struct termios options;

    /* tcgetattr(fd,&options)
     * get the object parameters, then save to options
     * lso can test the configuration correct or not
     * correct return 0, else return 1
    */
	if (tcgetattr( fd,&options) !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	cfmakeraw(&options);
	/*printf("cfmakeraw-------------\n");*/
	/*set uart input and output bodartate*/
	for (i= 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	/*Modify control mode, asure no other procce occupy serial */
	options.c_cflag |= CLOCAL;
	/*Modiy control mode, enable data read from serial */
	options.c_cflag |= CREAD;
	/*set data flow control */
	switch(flow_ctrl) {
		case 0 :/*not use flow control*/
              /*options.c_cflag &= ~CRTSCTS;*/
			options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | CRTSCTS);
			/* c_iflag(termios) need to config, if not config, while transfer 0x0d, 0x11, 0x13,
			 * it will recognatized as special char ctrl, close ICRNL and IXON Option will reslove it.
			 */
			break;
		case 1 :/*hardware flow ctrl*/
			options.c_cflag |= CRTSCTS;
			break;
		case 2 :/*software flow ctrl*/
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
	}
	/*set data width
	 * mask other flags*/
	options.c_cflag &= ~CSIZE;
	switch (databits) {
		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}
	/*set verify flag*/
	switch (parity) {
		case 'n':
		case 'N': /*set no odd/even*/
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':/*set odd/even*/
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':/*set even*/
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's':
		case 'S':/*set space*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}
    /*set stop bit*/
	switch (stopbits) {
		case 1:
			options.c_cflag &= ~CSTOPB; break;
		case 2:
			options.c_cflag |= CSTOPB; break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}

	/*modify output mode, raw data*/
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	/*options.c_lflag &= ~(ISIG | ICANON);*/

	/*set waitting time and minimal recevice char*/
	options.c_cc[VTIME] = 1; /* read one char wait 1*(1/10)s */
	options.c_cc[VMIN] = 1; /* read no less than 1 */

    /* if data overflow, still recive data, not read any more*/
	tcflush(fd,TCIFLUSH);

	/*active configuration*/
	if (tcsetattr(fd,TCSANOW,&options) != 0) {
		perror("com set error!\n");
		return (FALSE);
	}
	return (TRUE);
}

/*******************************************************************
* Name：			UART0_Init()
* Function:			serial init
* Input Param: 		fd:
*					speed: uart speed
*					flow_ctrl
*					databits
*					stopbits
*					parity
*
* Output param：	true:1, flase:0
*******************************************************************/
int app_uart_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int err;
    /*set uart data frame */
    if (UART0_Set(fd, speed, flow_ctrl, databits, stopbits, parity) == FALSE) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*******************************************************************
* Name:				UART0_Recv
* Function:			Revice data
* Input Param:		fd
*					rcv_buf
*					data_len: the length of one frame
* Output param:		true 1, false 0
*******************************************************************/
int app_uart_read(int fd, char *rcv_buf,int data_len,uint32_t timeout_msec)
{
	int len,fs_sel;
	fd_set fs_read;

	struct timeval time;

	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);
	if (UINT32_MAX == timeout_msec) {
		fs_sel = select(fd + 1, &fs_read, NULL, NULL, NULL);
	} else {
		time.tv_sec = timeout_msec / 1000;
		timeout_msec -= time.tv_sec * 1000;
		time.tv_usec = timeout_msec * 1000;

		/*use select to multiple communication*/
		fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
	}
	if (fs_sel&& FD_ISSET(fd, &fs_read)) {
		len = read(fd,rcv_buf,data_len);
		return len;
	} else {
		return FALSE;
	}
}

/********************************************************************
* Name: 		UART0_Send
* Function:		Send Data
* Input Param:	fd: file description
*				send_buf: buffer
*				data_len: date length
* Output Param：ok return 1，else return 0
*******************************************************************/
int app_uart_write(int fd, char *send_buf,int data_len)
{
	int i, len = 0;
 	/*for (i = 0; i < data_len; ++i) {
		printf("%02X ", ((uint8_t*)send_buf)[i]);
	}*/
	printf("\r\n");
    len = write(fd,send_buf,data_len);
    if (len == data_len )
        return len;
    else {
        tcflush(fd,TCOFLUSH);
        return FALSE;
    }
}

