#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include "ap_uart.h"

#define TRANSFER_UART_PATH "/dev/ttyS0"

int uart_fd = -1;

void platform_uart_cfg(int fd, int speed)
{
    struct termios options;

    tcgetattr(fd, &options);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON |ISIG);

    options.c_lflag = 0;
    options.c_lflag &= ~ISIG;
    options.c_lflag &= ~ECHO;

    options.c_oflag = 0;
	options.c_oflag &= ~OPOST;

    options.c_iflag |= IGNPAR;
    options.c_iflag &= ~IXON;
    options.c_iflag &= ~ICRNL;
	options.c_iflag &= ~(ICRNL|IGNCR);

    cfsetospeed(&options, speed);

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);
	tcflush(fd, TCIOFLUSH);
}

void dump_uart_buf(void)
{
	int count = 0;
	char buf[256];
	int i;
	count = read(uart_fd, buf, 256);

	if (count > 0) {
		for (i = 0 ; i < count; i++) {
			printf("0x%-2x ", buf[i]);
		}
	}
}

int uart_open()
{
	uart_fd = open (TRANSFER_UART_PATH, O_RDWR | O_NONBLOCK | O_NDELAY | O_NOCTTY);
	if (uart_fd < 0) {
		printf("serial init failed\r\n");
		return -1;
	} else {
  		if (fcntl(uart_fd, F_SETFL, FNDELAY) < 0) {
    		printf("fcntl failed!/n");
 			return -1;
  		} else {
       		printf("fcntl=%d/n",fcntl(uart_fd, F_SETFL,FNDELAY));
    	}
   		if (0 == isatty(STDIN_FILENO)) {
			printf("standard input is not a terminal device/n");
			return -1;
  		} else {
           printf("isatty success!/n");
    	}
		platform_uart_cfg(uart_fd, B115200);
	}

	return uart_fd;
}

ssize_t safe_read(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
	int data_flag =0;
    char *ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
        	if(data_flag)
				break;
			else {
            	nread = 0;
				return nread;
			}
        }
        else {
        	if (nread == 0)
            	break;

			nleft -= nread;
			ptr += nread;
			data_flag = 1;
			printf("nread1 =%d,nleft1=%d\n", nread, nleft);
        }
    }
    return (n-nleft);
}

ssize_t safe_write(int fd, const void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0)
    {
    if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0&&errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

int uart_read(int fd, char *r_buf, size_t len)
{
	int ret;
    ssize_t cnt = 0;
    fd_set rfds;
    struct timeval time;

    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);

    time.tv_sec = 0;
    time.tv_usec = 100000;

    ret = select(fd+1, &rfds, NULL, NULL, &time);
    switch(ret)
    {
        case -1:
            fprintf(stderr,"select error!\n");
            return -1;
        case 0:
            fprintf(stderr,"time over!\n");
            return 0;
        default:
            cnt = safe_read(fd, r_buf, len);
            if(cnt == -1)
            {
                fprintf(stderr,"read error!\n");
                return -1;
            }
			printf("--cnt =%d\n", cnt);
            return cnt;
    }
}

int uart_write(int fd, const char *w_buf, ssize_t len)
{
    ssize_t cnt = 0;

    cnt = safe_write(fd, w_buf, len);
    if(cnt == -1)
    {
        fprintf(stderr,"write error!\n");
        return -1;
    }
    return cnt;
}

void uart_close(int fd)
{
	if(fd)
    	close(fd);
}

