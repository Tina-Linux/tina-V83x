#include <unistd.h>
#include "fortify.h"

int __gethostname_chk(char *name, size_t len, size_t maxlen)
{
	if(len > maxlen) __chk_fail();
	return gethostname(name, len);
}