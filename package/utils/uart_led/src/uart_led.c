#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <log.h>
#include <app_uart.h>

#define APP_UART 			"/dev/ttyS0"
#define DEVICE_LED_UART_BAUD_RATE 4000000
/*#define DEVICE_LED_UART_BAUD_RATE 3500000*/

static int device_led_uart_fd;

/*diff --git a/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
           b/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
index 8eac871..daa64a3 100755
--- a/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
+++ b/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
@@ -703,6 +703,8 @@ int sunxi_clock_set_corepll(int frequency, int core_vol)
	reg_val |=  (0x02 << 16);
	writel(reg_val, CCM_CPU_L2_AXI_CTRL);
+	writel(0x0200000a, CCM_APB2_CLK_CTRL);
	return  0;
	}*/

/*--- a/drivers/tty/serial/sunxi-uart.c
+++ b/drivers/tty/serial/sunxi-uart.c
@@ -1044,12 +1044,13 @@ static void sw_uart_set_termios(
struct uart_port *port, struct ktermios *termios
		baud = uart_get_baud_rate(port, termios, old,
				port->uartclk / 16 / 0xffff,
				port->uartclk / 16);
	+	if(sw_uport->id == 0)
	+		baud = 3500000;
		sw_uart_check_baudset(port, baud);
		quot = uart_get_divisor(port, baud);
*/

/* For use baudrate 3500000
 * First enable uart use aphb2 clk source
 * default uart use internal 24M clk src
 * a. Modify clock.c add following line
 *		writel(0x0200000a, CCM_APB2_CLK_CTRL);
 *
 * b. Modify sunxi-uart.c to fit baudrate 3500000
 *    baudrate 4000000 no need to modify this;
 *
 * c. make kernel_menuconfig choose clk source
 *	(make kernel_menuconfig
 *	Device Drivers -->Common clock Framework
 *	--->SUNXI clock Configuration-->54545454)
 * */
void set_led_baud_3_5M()
{
	char data_str[13];

	/* show Red color*/
	data_str[0] = 0x3B;
	data_str[1] = 0x3B;
	data_str[2] = 0x3B;
	data_str[3] = 0x3B;

	data_str[4] = 0x04;
	data_str[5] = 0x04;
	data_str[6] = 0x04;
	data_str[7] = 0x04;

	data_str[8] = 0x3B;
	data_str[9] = 0x3B;
	data_str[10] = 0x3B;
	data_str[11] = 0x3B;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/* show Green color*/
	data_str[0] = 0x04;
	data_str[1] = 0x04;
	data_str[2] = 0x04;
	data_str[3] = 0x04;

	data_str[4] = 0x3B;
	data_str[5] = 0x3B;
	data_str[6] = 0x3B;
	data_str[7] = 0x3B;

	data_str[8] = 0x3B;
	data_str[9] = 0x3B;
	data_str[10] = 0x3B;
	data_str[11] = 0x3B;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/* show Blue color*/
	data_str[0] = 0x3B;
	data_str[1] = 0x3B;
	data_str[2] = 0x3B;
	data_str[3] = 0x3B;

	data_str[4] = 0x3B;
	data_str[5] = 0x3B;
	data_str[6] = 0x3B;
	data_str[7] = 0x3B;

	data_str[8] = 0x04;
	data_str[9] = 0x04;
	data_str[10] = 0x04;
	data_str[11] = 0x04;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/*White color*/
	data_str[0] = 0x04;
	data_str[1] = 0x04;
	data_str[2] = 0x04;
	data_str[3] = 0x04;

	data_str[4] = 0x04;
	data_str[5] = 0x04;
	data_str[6] = 0x04;
	data_str[7] = 0x04;

	data_str[8] = 0x04;
	data_str[9] = 0x04;
	data_str[10] = 0x04;
	data_str[11] = 0x04;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);
}

/*diff --git a/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
           b/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
index 8eac871..daa64a3 100755
--- a/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
+++ b/u-boot-2011.09/arch/arm/cpu/armv7/sun8iw8/clock.c
@@ -703,6 +703,8 @@ int sunxi_clock_set_corepll(int frequency, int core_vol)
	reg_val |=  (0x02 << 16);
	writel(reg_val, CCM_CPU_L2_AXI_CTRL);
+	writel(0x02000008, CCM_APB2_CLK_CTRL);
	return  0;
*/

/* For use baudrate 4000000
 * First enable uart use aphb2 clk source
 * default uart use internal 24M clk src
 * a. Modify clock.c add following line
 *		writel(0x02000008, CCM_APB2_CLK_CTRL);
 * b. make kernel_menuconfig choose clk source
 *	(make kernel_menuconfig
 *	Device Drivers -->Common clock Framework
 *	--->SUNXI clock Configuration-->66666666)
 *	*/

void set_led_baud_4M()
{
	char data_str[13];

	/*Green color*/
	data_str[0] = 0x08;
	data_str[1] = 0x08;
	data_str[2] = 0x08;
	data_str[3] = 0x08;
	data_str[4] = 0xEF;
	data_str[5] = 0xEF;
	data_str[6] = 0xEF;
	data_str[7] = 0xEF;
	data_str[8] = 0xEF;
	data_str[9] = 0xEF;
	data_str[10] = 0xEF;
	data_str[11] = 0xEF;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/*Red color*/
	data_str[0] = 0xEF;
	data_str[1] = 0xEF;
	data_str[2] = 0xEF;
	data_str[3] = 0xEF;
	data_str[4] = 0x08;
	data_str[5] = 0x08;
	data_str[6] = 0x08;
	data_str[7] = 0x08;
	data_str[8] = 0xEF;
	data_str[9] = 0xEF;
	data_str[10] = 0xEF;
	data_str[11] = 0xEF;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/*Blue color*/
	data_str[0] = 0xEF;
	data_str[1] = 0xEF;
	data_str[2] = 0xEF;
	data_str[3] = 0xEF;
	data_str[4] = 0xEF;
	data_str[5] = 0xEF;
	data_str[6] = 0xEF;
	data_str[7] = 0xEF;
	data_str[8] = 0x08;
	data_str[9] = 0x08;
	data_str[10] = 0x08;
	data_str[11] = 0xE8;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);

	/*white color*/
	data_str[0] = 0x08;
	data_str[1] = 0x08;
	data_str[2] = 0x08;
	data_str[3] = 0x08;
	data_str[4] = 0x08;
	data_str[5] = 0x08;
	data_str[6] = 0x08;
	data_str[7] = 0x08;
	data_str[8] = 0x08;
	data_str[9] = 0x08;
	data_str[10] = 0x08;
	data_str[11] = 0x08;
	app_uart_write(device_led_uart_fd, data_str, 12);
	usleep(2000000);
}

int main(int32_t argc, char** argv)
{
	int ret;
	FILE *file_dsc;

	log_info(">====Welcome to Uart LED Test=====<");

	device_led_uart_fd = app_uart_open(device_led_uart_fd, APP_UART);
	/* set for 4000000 baudrate*/
	app_uart_Init(device_led_uart_fd, DEVICE_LED_UART_BAUD_RATE, 0, 8, 1, 'N');

	/* set for 3500000 baudrate*/
	/*app_uart_Init(device_led_uart_fd, DEVICE_LED_UART_BAUD_RATE, 0, 6, 1, 'N');*/

	while(1) {
		set_led_baud_4M();
		usleep(80);
	}
	return 0;
}
