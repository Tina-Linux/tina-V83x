#include <unistd.h>
#include "fortify.h"

size_t __confstr_chk(int name, char *buf, size_t len, size_t buflen)
{
	if(buflen < len) __chk_fail();
	return confstr(name, buf, len);
}