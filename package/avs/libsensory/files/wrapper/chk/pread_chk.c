#include <unistd.h>
#include "fortify.h"

ssize_t __pread_chk(int fd, void *buf, size_t size, off_t ofs, size_t bufsize)
{
	if(bufsize < size) __chk_fail();
	return pread(fd, buf, size, ofs);
}

LFS64_2(__pread_chk, __pread64_chk);