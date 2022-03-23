void a_crash(void)
{
	exit(-1);
}

void __chk_fail(void)
{
	a_crash();
	for(;;);
}