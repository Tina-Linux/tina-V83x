#ifndef __DISP_TEST_HEAD_H
#define __DISP_TEST_HEAD_H

#include <stdio.h>

struct fb {
	int fd;
	unsigned int w;
	unsigned int h;
	unsigned int bpp;
	unsigned int bpp_byte;
	unsigned int mem_size;
	int b_map;
	void *mem;
};

#define PR_DISP_TEST_HEAD "[disptest]: "
#define __DISP_TEST_DEBUG
#ifdef __DISP_TEST_DEBUG
#define DISP_TEST_OK(x) {printf("\x1b[32m");printf(PR_DISP_TEST_HEAD);printf("OK   ");printf x;printf("\e[0m\n");}
#define DISP_TEST_FAIL(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf("FAIL ");printf x;printf("\e[0m\n");}
#define DISP_TEST_WARN(x) {printf("\x1b[33m");printf(PR_DISP_TEST_HEAD);printf("WARN ");printf x;printf("\e[0m\n");}
#define DISP_TEST_DBG(x) {printf x;}
#define DISP_TEST_PUT(x) {printf x;}
#define DISP_TEST_INFO(x) {printf(PR_DISP_TEST_HEAD);printf("INFO ");printf x;}
#define DISP_TEST_ERROR(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf x;printf("\e[0m\n");}
#else
#define DISP_TEST_OK(x) {printf("\x1b[32m");printf(PR_DISP_TEST_HEAD);printf("OK   ");printf x;printf("\e[0m\n");}
#define DISP_TEST_FAIL(x) {printf("\x1b[31m");printf(PR_DISP_TEST_HEAD);printf("FAIL ");printf x;printf("\e[0m\n");}
#define DISP_TEST_WARN(x) {printf("\x1b[33m");printf(PR_DISP_TEST_HEAD);printf("WARN ");printf x;printf("\e[0m\n");}
#define DISP_TEST_DBG(x) do{}while(0)
#define DISP_TEST_PUT(x) {printf x;}
#define DISP_TEST_INFO(x) {printf x;}
#endif

#endif
