#ifndef __APP_UART_H__
#define __APP_UART_H__

int app_uart_open(int fd,const char* port);
int app_uart_write(int fd, char *send_buf,int data_len);
int app_uart_read(int fd, char *rcv_buf,int data_len,uint32_t timeout_msec);
int app_uart_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);

#endif

