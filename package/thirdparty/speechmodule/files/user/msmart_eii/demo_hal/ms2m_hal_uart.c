/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_uart.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/11
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#include "stdio.h"
#include "string.h"
#include "fcntl.h"
#include "termios.h"
#include "unistd.h"

#include "ms2m_hal_chip.h"
#include "ms2m_hal_socket.h"
#include "ms2m_hal_uart.h"
#include "ms_common.h"

//#define	UART_READ_LEN    256
#define UART_DEV_FILE "/dev/ttyUSB0"

static char *gs_port = NULL;

static int gs_fd = -1;

typedef enum
{
    MS_HAL_BAUDRATE_300,
    MS_HAL_BAUDRATE_1200,
    MS_HAL_BAUDRATE_2400,
    MS_HAL_BAUDRATE_4800,
    MS_HAL_BAUDRATE_9600,
    MS_HAL_BAUDRATE_19200,
    MS_HAL_BAUDRATE_38400,
    MS_HAL_BAUDRATE_57600,
    MS_HAL_BAUDRATE_115200,
}ms_hal_uart_baudrate_t;

typedef enum
{
    MS_HAL_DATA_WIDTH_5BIT,
    MS_HAL_DATA_WIDTH_6BIT,
    MS_HAL_DATA_WIDTH_7BIT,
    MS_HAL_DATA_WIDTH_8BIT,
    MS_HAL_DATA_WIDTH_9BIT,
}ms_hal_uart_data_width_t;

typedef enum
{
    MS_HAL_NO_PARITY,
    MS_HAL_ODD_PARITY,
    MS_HAL_EVEN_PARITY,
}ms_hal_uart_parity_t;

typedef enum
{
    MS_HAL_STOP_BITS_1,
    MS_HAL_STOP_BITS_2,
}ms_hal_uart_stop_bits_t;

typedef enum
{
    MS_HAL_FLOW_CONTROL_DISABLED,
    MS_HAL_FLOW_CONTROL_CTS,
    MS_HAL_FLOW_CONTROL_RTS,
    MS_HAL_FLOW_CONTROL_CTS_RTS,
} ms_hal_uart_flow_control_t;

typedef enum
{
    MS_HAL_UART_COMM, //for home appliance communication
    MS_HAL_UART_DEBUG, //for uart debug
    MS_HAL_UART_DEV_MAX,
}ms_hal_uart_device_t;

typedef struct ms_hal_uart_config_s
{
    ms_hal_uart_baudrate_t baudrate;
    ms_hal_uart_data_width_t data_width;
    ms_hal_uart_parity_t parity;
    ms_hal_uart_stop_bits_t stop_bits;
    ms_hal_uart_flow_control_t flow_control;
}ms_hal_uart_config_t;

typedef struct termios ms_hal_serial_format_t;

static void _get_serial_format(const ms_hal_uart_config_t uart_cfg, ms_hal_serial_format_t *serial_format)
{
    switch(uart_cfg.baudrate)
    {
    case MS_HAL_BAUDRATE_300:
        cfsetspeed(serial_format, B300);
        break;
    case MS_HAL_BAUDRATE_1200:
        cfsetspeed(serial_format, B1200);
        break;
    case MS_HAL_BAUDRATE_2400:
        cfsetspeed(serial_format, B2400);
        break;
    case MS_HAL_BAUDRATE_4800:
        cfsetspeed(serial_format, B4800);
        break;
    case MS_HAL_BAUDRATE_9600:
        cfsetspeed(serial_format, B9600);
        break;
    case MS_HAL_BAUDRATE_19200:
        cfsetspeed(serial_format, B19200);
        break;
    case MS_HAL_BAUDRATE_38400:
        cfsetspeed(serial_format, B38400);
        break;
    case MS_HAL_BAUDRATE_57600:
        cfsetspeed(serial_format, B57600);
        break;
    case MS_HAL_BAUDRATE_115200:
        cfsetspeed(serial_format, B115200);
        break;
    default:
        cfsetspeed(serial_format, B9600);
        break;
    }

    serial_format->c_cflag &= ~CSIZE;
    serial_format->c_cflag |= CLOCAL;
    serial_format->c_cflag |= CREAD;

    switch(uart_cfg.data_width)
    {
    case MS_HAL_DATA_WIDTH_5BIT:
        serial_format->c_cflag |= CS5;
        break;
    case MS_HAL_DATA_WIDTH_6BIT:
        serial_format->c_cflag |= CS6;
        break;
    case MS_HAL_DATA_WIDTH_7BIT:
        serial_format->c_cflag |= CS7;
        break;
    case MS_HAL_DATA_WIDTH_8BIT:
        serial_format->c_cflag |= CS8;
        break;
    default:
        serial_format->c_cflag |= CS8;
        break;
    }

    switch(uart_cfg.parity)
    {
    case MS_HAL_NO_PARITY:
        serial_format->c_cflag &= ~PARENB;
        serial_format->c_iflag &= ~INPCK;
        break;
    case MS_HAL_ODD_PARITY:
        serial_format->c_cflag |= PARENB;
        serial_format->c_cflag |= PARODD;
        serial_format->c_iflag |= (INPCK | ISTRIP);
        break;
    case MS_HAL_EVEN_PARITY:
        serial_format->c_cflag |= PARENB;
        serial_format->c_cflag &= ~PARODD;
        serial_format->c_iflag |= (INPCK | ISTRIP);
        break;
    default:
        serial_format->c_cflag &= ~PARENB;
        serial_format->c_iflag &= ~INPCK;
        break;
    }

    switch(uart_cfg.stop_bits)
    {
    case MS_HAL_STOP_BITS_1:
        serial_format->c_cflag &= ~CSTOPB;
        break;
    case MS_HAL_STOP_BITS_2:
        serial_format->c_cflag |= CSTOPB;
        break;
    default:
        serial_format->c_cflag &= ~CSTOPB;
        break;
    }

    serial_format->c_cflag &= ~CRTSCTS; //No ctl flow

    serial_format->c_oflag &= ~OPOST;
    serial_format->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);

    serial_format->c_iflag &= ~ (INLCR | ICRNL | IGNCR);
    serial_format->c_oflag &= ~(ONLCR | OCRNL);

    serial_format->c_cc[VINTR]    = 0;       /**//* Ctrl-c */
    serial_format->c_cc[VQUIT]     = 0;   /**//* Ctrl- */
    serial_format->c_cc[VERASE]    = 0;   /**//* del */
    serial_format->c_cc[VKILL]    = 0;   /**//* @ */
    serial_format->c_cc[VEOF]     = 0;   /**//* Ctrl-d */
    serial_format->c_cc[VTIME]    = 1;   /**//*  */
    serial_format->c_cc[VMIN]     = 0;   /**//*  */
    serial_format->c_cc[VSWTC]    = 0;   /**//* '' */
    serial_format->c_cc[VSTART]   = 0;   /**//* Ctrl-q */
    serial_format->c_cc[VSTOP]    = 0;   /**//* Ctrl-s */
    serial_format->c_cc[VSUSP]    = 0;   /**//* Ctrl-z */
    serial_format->c_cc[VEOL]     = 0;   /**//* '' */
    serial_format->c_cc[VREPRINT] = 0;   /**//* Ctrl-r */
    serial_format->c_cc[VDISCARD] = 0;   /**//* Ctrl-u */
    serial_format->c_cc[VWERASE]  = 0;   /**//* Ctrl-w */
    serial_format->c_cc[VLNEXT]   = 0;   /**//* Ctrl-v */
    serial_format->c_cc[VEOL2]    = 0;   /**//* '' */
//    serial_format->c_cc[VTIME] = 0;
//    serial_format->c_cc[VMIN] = 1;
}
//---------------------------------------------------------------------------------------------------------------//

#if 0
static MS2M_STATUS _uart_close(const ms_hal_uart_device_t uart_dev)
{
    MS2M_STATUS ret = M_OK;

    if(uart_dev == MS_HAL_UART_COMM)
    {
        hal_uart_deinit(HAL_UART_1);
        gs_comm_uart_handle = -1;
    }
    else if(uart_dev == MS_HAL_UART_DEBUG)
    {
        hal_uart_deinit(HAL_UART_0);
        gs_comm_uart_handle = -1;
    }
    else
    {
        hal_uart_deinit(HAL_UART_1);
        gs_comm_uart_handle = -1;
        hal_uart_deinit(HAL_UART_0);
        gs_debug_uart_handle = -1;
    }

    return ret;
}
#endif
//----------------------------------------------------------------//

int ms2m_hal_uart_write(unsigned char *buffer, unsigned int len)
{
    unsigned long now_send_time_tick = 0;
    static unsigned long last_send_time_tick = 0;
    ssize_t ret = 0;

    if(gs_fd == -1)
    {
        return -1;
    }

    if((NULL == buffer) || (len <= 0))
        return 0;

    now_send_time_tick = ms2m_hal_get_ticks();
    while(M_ERROR == ms2m_hal_ticks_compare(last_send_time_tick, now_send_time_tick, 50))
    {
        ms2m_hal_msleep(5);
        now_send_time_tick = ms2m_hal_get_ticks();
    }
    ret = write(gs_fd, buffer, len);
    if(ret != len)
    {
        tcflush(gs_fd,TCOFLUSH);
    }
    last_send_time_tick = ms2m_hal_get_ticks();
    return ret;
}

int ms2m_hal_uart_read(unsigned char *buffer, unsigned int len)
{
    int recv_len = -1;
    int retval = -1;
    ms2m_fd_set readfds;
    struct timeval tv;

    tv.tv_sec = 0;//set the rcv wait time
    tv.tv_usec = 70000;//100000us = 0.1s

    if(gs_fd == -1)
    {
        return -1;
    }

    if((NULL == buffer) || (len <= 0))
    {
        return -1;
    }

    ms2m_hal_fd_clean(&readfds);
    ms2m_hal_fd_set(gs_fd, &readfds);

    retval = ms2m_hal_select(gs_fd + 1, &readfds, NULL, NULL, &tv);
    if((retval > 0)&&(ms2m_hal_fd_is_set(gs_fd, &readfds)))
    {
        recv_len = read(gs_fd, buffer, len);
    }
    return recv_len;
}

MS2M_STATUS ms2m_hal_uart_init(void)	//initial the uart.
{
    ms_hal_uart_config_t uart_init_params;
    ms_hal_serial_format_t sUartCfg;

    if(gs_port == NULL)
    {
        gs_fd = open(UART_DEV_FILE, O_RDWR|O_NOCTTY);
        if(-1 == gs_fd)
        {
            MS_ERR_TRACE("cannot open %s\r\n",UART_DEV_FILE);
            return M_ERROR;
        }
    }
    else
    {
        gs_fd = open(gs_port, O_RDWR|O_NOCTTY);
        if(-1 == gs_fd)
        {
            MS_ERR_TRACE("cannot open %s\r\n",gs_port);
            return M_ERROR;
        }
    }

    memset(&sUartCfg,0,sizeof(ms_hal_serial_format_t));
    memset(&uart_init_params, 0, sizeof(ms_hal_uart_config_t));

    uart_init_params.baudrate	= MS_HAL_BAUDRATE_9600;
    uart_init_params.data_width = MS_HAL_DATA_WIDTH_8BIT;
    uart_init_params.flow_control = MS_HAL_FLOW_CONTROL_DISABLED;
    uart_init_params.parity	= MS_HAL_NO_PARITY;
    uart_init_params.stop_bits	= MS_HAL_STOP_BITS_1;

    _get_serial_format(uart_init_params, &sUartCfg);

    tcsetattr(gs_fd, TCSANOW, &sUartCfg);
    tcflush(gs_fd, TCIFLUSH);

    return M_OK;
}

void ms2m_hal_uart_port_set(char *port)
{
    gs_port = port;
}
