/*
 * crash-worker-test -- testcase to generate crashes
 *
 * Coryright 2017, Allwinnertech Co., Ltd
 *
 * Author:  Jackie <huangshr@allwinnnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void do_fault(void)
{
	char *ptr;
	printf("inside do fault");

	ptr = NULL;
	*ptr = 'x';
	printf("%c\n", *ptr);
}

void delay_then_fault(int count)
{
	int i;

	for (i = count; i > 0; i--) {
		printf("%d", i);
		printf(i > 1 ? "," : "...\n");
		fflush(stdout);
		sleep(1);
	}

	do_fault();
}

int main(int argc, char **argv)
{
	int count = 5;

	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0 ||
		    strcmp(argv[1], "--help") == 0) {
			printf("Usage fault-test [<delay>]\n");
			exit(0);
		}
		count = atoi(argv[1]);
	}
	printf("Doing a segfault in %d second.", count);
	delay_then_fault(count);
	return 0;
}
