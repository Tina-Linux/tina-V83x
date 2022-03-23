#ifndef __MS_HAL_UART_H__
#define	__MS_HAL_UART_H__

#include <ms_common.h>
//hw uart recv buffer len
#define	UART_READ_LEN    512

typedef enum
{
	MS_HAL_UART_RESULT_SUCCESS = 0,
	MS_HAL_UART_RESULT_ERROR   = 1,
}ms_hal_uart_result_t;

/**
* @Function	: ms_hal_hw_uart_open
* @return	MS_HAL_UART_RESULT_SUCCESS/MS_HAL_UART_RESULT_ERROR
**/
ms_hal_uart_result_t  ms_hal_hw_uart_open(const char* dev, int baudrate, int *serialfd);

/**
* @Function	: ms_hal_hw_uart_close
* @return	MS_HAL_UART_RESULT_SUCCESS/MS_HAL_UART_RESULT_ERROR
**/
ms_hal_uart_result_t  ms_hal_hw_uart_close(int serialfd);

/**
* @Function	: write data to uart
* @return	Actually written data bytes
**/
int  ms_hal_hw_uart_write(int serialfd,uint8_t *buffer, int len );

/**
* @Function	: read data from uart
* @return	Actually read data bytes
**/
int  ms_hal_hw_uart_read (int serialfd, uint8_t *buffer, int  len );
#endif
