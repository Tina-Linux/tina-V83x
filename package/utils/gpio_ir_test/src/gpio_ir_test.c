#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/lirc.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define NEC_NBITS		32
#define NEC_UNIT		562500  /* ns */
#define NEC_HEADER_PULSE	(16 * NEC_UNIT) /*9ms*/
#define NECX_HEADER_PULSE	(8  * NEC_UNIT) /* Less common NEC variant */
#define NEC_HEADER_SPACE	(8  * NEC_UNIT) /*4.5ms*/
#define NEC_REPEAT_SPACE	(4  * NEC_UNIT) /*2.25ms*/
#define NEC_BIT_PULSE		(1  * NEC_UNIT)
#define NEC_BIT_0_SPACE		(1  * NEC_UNIT)
#define NEC_BIT_1_SPACE		(3  * NEC_UNIT)
#define	NEC_TRAILER_PULSE	(1  * NEC_UNIT)
#define	NEC_TRAILER_SPACE	(10 * NEC_UNIT) /* even longer in reality */

#define NS_TO_US(nsec)		((nsec) / 1000)

#define GPIO_IR_RAW_BUF_SIZE 128
#define DEFAULT_CARRIER_FREQ 38000
#define DEFAULT_DUTY_CYCLE   33

uint32_t rx_raw_buf[GPIO_IR_RAW_BUF_SIZE];
uint32_t tx_raw_buf[GPIO_IR_RAW_BUF_SIZE];

static int fd;
struct pollfd poll_fds[1];
static int int_exit;
pthread_t tid;

static void print_usage(const char *argv0)
{	printf("usage: %s [options]\n", argv0);
	printf("\n");
	printf("gpio ir receive test:\n");
	printf("\tgpio_ir_test rx\n");
	printf("\n");
	printf("gpio ir send test:\n");
	printf("\tgpio_ir_test tx <code>\n");
	printf("\n");
	printf("gpio ir loop test, rx&tx:\n");
	printf("\tgpio_ir_test loop\n");
	printf("\n");
}

static int nec_modulation_byte(uint32_t *buf, uint8_t code)
{
	int i = 0;
	uint8_t mask = 0x01;

	while (mask) {
		/*low bit first*/
		if (code & mask) {
			/*bit 1*/
			*(buf + i) = LIRC_PULSE(NS_TO_US(NEC_BIT_PULSE));
			*(buf + i + 1) = LIRC_SPACE(NS_TO_US(NEC_BIT_1_SPACE));
		} else {
			/*bit 0*/
			*(buf + i) = LIRC_PULSE(NS_TO_US(NEC_BIT_PULSE));
			*(buf + i + 1) = LIRC_SPACE(NS_TO_US(NEC_BIT_0_SPACE));
		}
		mask <<= 1;
		i += 2;
	}
	return i;
}

static int nec_ir_encode(uint32_t *raw_buf, uint32_t key_code)
{
	uint8_t address, not_address, command, not_command;
	uint32_t *head_p, *data_p, *stop_p;

	address	= (key_code >> 24) & 0xff;
	not_address = (key_code >> 16) & 0xff;
	command	= (key_code >>  8) & 0xff;
	not_command = (key_code >>  0) & 0xff;

	/*head bit*/
	head_p = raw_buf;
	*(head_p) = LIRC_PULSE(NS_TO_US(NEC_HEADER_PULSE));
	*(head_p + 1) = LIRC_SPACE(NS_TO_US(NEC_HEADER_SPACE));

	/*data bit*/
	data_p = raw_buf + 2;
	nec_modulation_byte(data_p,  address);

	data_p += 16;
	nec_modulation_byte(data_p,  not_address);

	data_p += 16;
	nec_modulation_byte(data_p,  command);

	data_p += 16;
	nec_modulation_byte(data_p,  not_command);

	/*stop bit*/
	stop_p = data_p + 16;
	*(stop_p) = LIRC_PULSE(NS_TO_US(NEC_TRAILER_PULSE));
	*(stop_p + 1) = LIRC_SPACE(NS_TO_US(NEC_TRAILER_SPACE));

	 /*return the total size of nec protocal pulse*/
	return (NEC_NBITS + 2) * 2;

}

void *ir_recv_thread(void *arg)
{
	int size = 0, size_t = 0;
	int i = 0;
	int dura;
	int ret;
	int total = 0;

	poll_fds[0].fd = fd;
	poll_fds[0].events = POLLIN | POLLERR;
	poll_fds[0].revents = 0;
	while (!int_exit) {
		ret = poll(poll_fds, 1, 100);
		if (!ret) {
			//printf("time out\n");
			printf("\n--------------------\n");
			total = 0;
		} else {
			if (poll_fds[0].revents == POLLIN) {
				size = read(fd, (char *)(rx_raw_buf),
						GPIO_IR_RAW_BUF_SIZE);
				size_t = size / sizeof(uint32_t);
				for (i = 0; i < size_t; i++) {
					dura = rx_raw_buf[i] & 0xffffff;
					printf("%d ", dura);
					if ((total++) % 8 == 0)
						printf("\n");
				}
			}
		}
	}

	return NULL;
}

void gpio_ir_test_close(int sig)
{	/* allow the stream to be closed gracefully */
	signal(sig, SIG_IGN);
	int_exit = 1;
	close(fd);
}

int main(int argc, char **argv)
{
	int ret;
	int size = 0, size_t = 0;
	int i = 0;
	int duty_cycle, carrier_freq;
	int key_code = 0;
	int err = 0;

	/* catch ctrl-c to shutdown cleanly */
	signal(SIGINT, gpio_ir_test_close);

	if (argc < 2) {
		print_usage(argv[0]);
		return -1;
	}

	fd = open("/dev/lirc0", O_RDWR);
	if (fd < 0) {
		printf("can't open lirc0, check driver!\n");
		return 0;
	}
	printf("lirc0 open succeed.\n");

	if (!strcmp(argv[1], "rx")) {
		err = pthread_create(&tid, NULL, (void *)ir_recv_thread, NULL);
		if (err != 0) {
			printf("create pthread error: %d\n", __LINE__);
			goto OUT;
		}

		do {
			usleep(1000);
		} while (!int_exit);

	} else if (!strcmp(argv[1], "tx")) {
		if (argc < 3) {
			fprintf(stderr, "No data passed\n");
			goto OUT;
		}

		if (sscanf(argv[2], "%x", &key_code) != 1) {
			fprintf(stderr, "no input data: %s\n", argv[2]);
			goto OUT;
		}

		carrier_freq = DEFAULT_CARRIER_FREQ;
		if (ioctl(fd, LIRC_SET_SEND_CARRIER, &carrier_freq)) {
			fprintf(stderr,
				"lirc0: could not set carrier freq: %s\n",
				strerror(errno));
			goto OUT;
		}

		duty_cycle = DEFAULT_DUTY_CYCLE;
		if (ioctl(fd, LIRC_SET_SEND_DUTY_CYCLE, &duty_cycle)) {
			fprintf(stderr,
				"lirc0: could not set carrier duty: %s\n",
				strerror(errno));
			goto OUT;
		}

		printf("irtest: send key code : 0x%x\n", key_code);

		size = nec_ir_encode(tx_raw_buf, key_code);
		/*dump the raw data*/
		for (i = 0; i < size; i++) {
			printf("%d ", *(tx_raw_buf + i) & 0x00FFFFFF);
			if ((i + 1) % 8 == 0)
				printf("\n");
		}
		printf("\n");

		size_t = size * sizeof(uint32_t);
		ret = write(fd, (char *)tx_raw_buf, size_t);
		if (ret > 0)
			printf("irtest: send %d bytes ir raw data\n\n", ret);
	} else if (!strcmp(argv[1], "loop")) {

		/*code: 0x13*/
		key_code = 0x04fb13ec;
		err = pthread_create(&tid, NULL, (void *)ir_recv_thread, NULL);
		if (err != 0) {
			printf("create pthread error: %d\n", __LINE__);
			goto OUT;
		}

		carrier_freq = DEFAULT_CARRIER_FREQ;
		if (ioctl(fd, LIRC_SET_SEND_CARRIER, &carrier_freq)) {
			fprintf(stderr,
				"lirc0: could not set carrier freq: %s\n",
				strerror(errno));
			goto OUT;
		}

		duty_cycle = DEFAULT_DUTY_CYCLE;
		if (ioctl(fd, LIRC_SET_SEND_DUTY_CYCLE, &duty_cycle)) {
			fprintf(stderr,
				"lirc0: could not set carrier duty: %s\n",
				strerror(errno));
			goto OUT;
		}
		/*echo 50ms transmit one frame */
		do {
			size = nec_ir_encode(tx_raw_buf, key_code);
			size_t = size * sizeof(uint32_t);
			printf("send key code : 0x%x, %d\n", key_code, i++);
			write(fd, (char *)tx_raw_buf, size_t);
			usleep(100*1000);
		} while (!int_exit);
	}

OUT:
	close(fd);
	return 0;
}
