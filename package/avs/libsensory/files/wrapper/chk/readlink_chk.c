#include <unistd.h>
#include "fortify.h"

ssize_t __readlink_chk(const char *restrict path, char *restrict buf, size_t size, size_t bufsize)
{
	if(bufsize < size) __chk_fail();
	return readlink(path, buf, size);
}