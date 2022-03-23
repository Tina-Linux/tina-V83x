#include <unistd.h>
#include "fortify.h"

int __getgroups_chk(int count, gid_t *list, size_t listlen)
{
	if(count*sizeof(gid_t) > listlen) __chk_fail();
	return getgroups(count, list);
}