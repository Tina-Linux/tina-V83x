#include <unistd.h>
#include "fortify.h"

char *__getcwd_chk(char *buf, size_t len, size_t buflen)
{
	if(buflen < len) __chk_fail();
	return getcwd(buf, len);
}