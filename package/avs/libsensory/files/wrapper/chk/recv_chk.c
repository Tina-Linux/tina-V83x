#include <sys/socket.h>
#include "fortify.h"

ssize_t __recv_chk(int fd, void *buf, size_t len, size_t buflen, int flags)
{
	if(buflen < len) __chk_fail();
	return recv(fd, buf, len, flags);
}