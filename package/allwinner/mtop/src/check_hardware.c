#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * get hardware spec value
 * @hardware:soc hardware name
 * @revision:
 * @chipid:soc diff-serial
 */
void get_hardware_name(char *hardware)
{
	const char *cpuinfo = "/proc/cpuinfo";
	char *data = NULL;
	size_t len = 0, limit = 2048;
	int fd, n;
	char *x, *hw, *rev, *id;

	fd = open(cpuinfo, O_RDONLY);
	if (fd < 0) {
		printf("Failed to open %s: %s (%d).\n", cpuinfo,
		       strerror(errno), errno);
		return;
	}

	for (;;) {
		x = realloc(data, limit);
		if (!x) {
			printf("Failed to allocate memory to read %p\n",
			       (void *)cpuinfo);
			goto done;
		}
		data = x;

		n = read(fd, data + len, limit - len);

		if (n < 0) {
			printf("Failed to reading %s: %s (%d)", cpuinfo,
			       strerror(errno), errno);
			goto done;
		}
		len += n;

		if (len < limit)
			break;

		/* We filled the buffer, so increase size and loop to read more */
		limit *= 2;
	}

	data[len] = 0;
	hw = strstr(data, "\nHardware");

	if (hw) {
		x = strstr(hw, ": ");
		if (x) {
			x += 2;
			n = 0;
			while (*x && *x != '\n') {
				if (!isspace(*x))
					hardware[n++] = tolower(*x);
				x++;
				if (n == 16)
					break;
			}
			hardware[n] = 0;
		}
	} else {
		int dt_fd;
		dt_fd = open("/proc/device-tree/model", O_RDONLY);
		if (dt_fd > 0) {
			read(dt_fd, hardware, 20);
			close(dt_fd);
		}
	}

done:
	close(fd);
	free(data);
}
