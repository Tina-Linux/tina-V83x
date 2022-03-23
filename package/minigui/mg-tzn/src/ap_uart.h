#ifndef	__AP_UART_H__
#define	__AP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

int uart_open(void);

int uart_read(int fd, char *r_buf, size_t len);

int uart_write(int fd, const char *w_buf, ssize_t len);

void uart_close(int fd);

#ifdef __cplusplus
}
#endif

#endif /*__AP_UART_H__*/

