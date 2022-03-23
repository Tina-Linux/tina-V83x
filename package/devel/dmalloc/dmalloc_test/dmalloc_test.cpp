#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#define DMALLOC 1

#if DMALLOC
#include "dmalloc.h"
#endif

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Invalid parameters\n");
		printf("usage: dmalloc_test <1|2|3|4|5>\n");
		printf("       1 - overflow\n");
		printf("       2 - use after free\n");
		printf("       3 - double free\n");
		printf("       4 - memory leak\n");
		printf("       5 - check unfreed pointers since the mark\n");
		return -1;
	}

	printf("argv[0]: %s, argv[1]: %s\n", argv[0], argv[1]);

#if DMALLOC
	dmalloc_debug_setup("debug=0x4f47d03,log=/tmp/dmalloc_test.log");

	/* get the current dmalloc position */
	unsigned long mark;
	if (!strcmp(argv[1], "5"))
		mark = dmalloc_mark();
#endif

	int *p1=new int;
	int *p2=new int[20];
	int *p3=(int*)malloc(sizeof(int));
	int *p4=(int*)malloc(sizeof(int));
	char *p5=(char*)malloc(256);

	printf("p1: 0x%p; p2: 0x%p; p3: 0x%p; p4: 0x%p; p5: 0x%p\n", p1, p2, p3, p4, p5);

	/* 1. overflow */
	if (!strcmp(argv[1], "1"))
		memset(p5, 0, 256+1);

	/* 2. use after free */
	delete p1;
	if (!strcmp(argv[1], "2"))
		*p1=10;

#if DMALLOC
	/*
	* log unfreed pointers that have been added to
	* the heap since mark
	*/
	if (!strcmp(argv[1], "5"))
		dmalloc_log_changed(mark,
			1 /* log unfreed pointers */,
			0 /* do not log freed pointers */,
			1 /* log each pnt otherwise summary */);
#endif

	/* 3. double free */
	free(p3);
	if (!strcmp(argv[1], "3"))
		free(p3);

	printf("dmalloc test end\n");

	/* 4. memory leak p2,p4,p5 */
	if (strcmp(argv[1], "4"))
		while(1);

	return 0;
}
