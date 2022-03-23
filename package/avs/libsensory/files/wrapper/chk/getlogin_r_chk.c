#include <unistd.h>
#include "fortify.h"

int __getlogin_r_chk(char *name, size_t size, size_t maxlen)
{
	if(size < maxlen) __chk_fail();
	return getlogin_r(name, size);
}