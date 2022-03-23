#include <unistd.h>
#include "fortify.h"

ssize_t __read_chk(int fd, void *buf, size_t count, size_t buflen)
{
	if(buflen < count) __chk_fail();
	return read(fd, buf, count);
}