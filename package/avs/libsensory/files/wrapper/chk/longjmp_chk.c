#include <setjmp.h>

void __longjmp_chk(jmp_buf env, int val)
{
	return longjmp(env, val);
}