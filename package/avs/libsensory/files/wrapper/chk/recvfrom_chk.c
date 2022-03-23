#include <sys/socket.h>
#include "fortify.h"

ssize_t __recvfrom_chk(int fd, void *restrict buf, size_t len, size_t buflen, int flags, struct sockaddr *restrict addr, socklen_t *restrict alen)
{
	if(buflen < len) __chk_fail();
	return recvfrom(fd, buf, len, flags, addr, alen);
}