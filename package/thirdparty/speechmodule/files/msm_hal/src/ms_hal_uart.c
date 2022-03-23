/**
 * @file ms_hal_uart.c
 * @brief
 * @author Humble
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "ms_hal_uart.h"
#include "ms_common.h"
pthread_mutex_t mLock;
static int wake_asr_test = 0;

ms_hal_uart_result_t  ms_hal_hw_uart_open(const char* dev, int baudrate, int *serialfd)
{
    struct termios stermios;
    int fd = -1;
    int err;
	//if(!access("/data/.asr_test", F_OK)){	//delete by 926 @20190617
	if(!access("/mnt/app/.asr_test", F_OK)){
		printf("Enter develop asr_test mode\n");
		wake_asr_test = 1;
	}
	else {
		wake_asr_test = 0;
	}

    pthread_mutex_init(&mLock,NULL);
    speed_t rate = 0;

    if (baudrate == 9600)
        rate = B9600;
    else if (baudrate == 19200)
        rate = B19200;
    else if (baudrate == 38400)
        rate = B38400;
    else if (baudrate == 57600)
        rate = B57600;
    else if (baudrate == 115200)
        rate = B115200;
    else if (baudrate == 230400)
        rate = B230400;

    if (rate == 0) {
        MS_ERR_TRACE("serial baudrate (%d) not support, open fail!", baudrate);
        printf("serial baudrate (%d) not support, open fail!\n", baudrate);
        goto Fail;
    }

    fd = open(dev, O_RDWR|O_NONBLOCK|O_NOCTTY);

    if (fd < 0) {
        MS_ERR_TRACE("open serial dev (%s) error, fd = %d", dev, fd);
        printf("open serial dev (%s) error, fd = %d\n", dev, fd);
        goto Fail;
    } else {
        MS_DBG_TRACE("open serial dev (%s) baudrate (%d) succceed! fd = %d", dev, baudrate, fd);
        printf("open serial dev (%s) baudrate (%d) succceed! fd = %d\n", dev, baudrate, fd);
        tcflush(fd, TCIOFLUSH);
        if ((err = tcgetattr(fd, &stermios)) != 0) {
            MS_ERR_TRACE("tcgetattr (%d) = %d", fd, err);
            printf("tcgetattr (%d) = %d\n", fd, err);
            goto Fail;
        }
        stermios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
        stermios.c_oflag &= ~OPOST;
        stermios.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
        stermios.c_cflag &= ~(CSIZE|PARENB|PARODD|CSTOPB);
        stermios.c_cflag |= CS8;
        stermios.c_cflag &= ~CRTSCTS;

        tcflush(fd, TCIOFLUSH);
        if (cfsetispeed(&stermios, rate)) {
            MS_ERR_TRACE("cfsetispeed.. errno..");
            printf("cfsetispeed.. errno..\n");
            goto Fail;
        }
        tcsetattr(fd,TCSANOW, &stermios);
        MS_DBG_TRACE("serial dev setup finished..");
        printf("serial dev setup finished..\n");
    }

    *serialfd =fd;

    return MS_HAL_UART_RESULT_SUCCESS;

Fail:
    if (fd > 0){
        close(fd);
        fd = -1;
    }
    return MS_HAL_UART_RESULT_ERROR;
}

ms_hal_uart_result_t  ms_hal_hw_uart_close(int serialfd)
{
    if (serialfd > 0){
        close(serialfd);
        serialfd = -1;
        return MS_HAL_UART_RESULT_SUCCESS;
    }
    pthread_mutex_destroy(&mLock);
    return MS_HAL_UART_RESULT_ERROR;
}


int  ms_hal_hw_uart_write(int serialfd,uint8_t *buffer, int len )
{
    int ret = -1;

	if(wake_asr_test == 1)
		return 0;
    PRINT_BUF(buffer,len);
    pthread_mutex_lock(&mLock);
    if (serialfd > 0){
        ret = 0;
        if (buffer != NULL) {
            if(-1==(ret = write(serialfd, buffer, len))){
		MS_ERR_TRACE("UART write error");
            }
        }
    }
    pthread_mutex_unlock(&mLock);
    return ret;
}

int  ms_hal_hw_uart_read (int serialfd, uint8_t *buffer, int  len )
{
    int ret = -1;
    pthread_mutex_lock(&mLock);
    if (serialfd > 0){
        ret = 0;
        if (buffer != NULL) {
            ret = read(serialfd, buffer, len);
        }
    }
    pthread_mutex_unlock(&mLock);
    return ret;
}
