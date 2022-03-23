#define _GNU_SOURCE
#include <sys/select.h>
#include "fortify.h"

long __fdelt_chk(long d)
{
	if(0 < d || d >= FD_SETSIZE) __chk_fail();
	return d/NFDBITS;
}