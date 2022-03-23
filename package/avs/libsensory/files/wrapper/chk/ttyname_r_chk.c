#include <unistd.h>
#include "fortify.h"

int __ttyname_r_chk(int fd, char *name, size_t size, size_t nsize)
{
	if(nsize < size) __chk_fail();
	return ttyname_r(fd, name, size);
}