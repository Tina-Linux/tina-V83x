#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <asm/types.h>
#include <sys/time.h>

#ifdef __SUNXI_DISPLAY2__
#include <arm_neon.h>
#endif

#include "yuv_rotate.h"

#if defined(__SUNXI_DISPLAY2__) && (!defined(CONF_KERNEL_VERSION_4_4)) && (!defined(CONF_KERNEL_VERSION_4_9))
void nv_rotage90(unsigned int width,
		unsigned int height, unsigned char* src_addr,
		unsigned char* dst_addr)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;

	unsigned int row_bytes = width;
	unsigned int col_bytes = height;
	unsigned int row_bytes_7 = width*7;
	unsigned int row_bytes_3 = width*3;

	unsigned char* y_src_line0 = src_addr;
	unsigned char* y_src_line1 = y_src_line0 + row_bytes;
	unsigned char* y_src_line2 = y_src_line1 + row_bytes;
	unsigned char* y_src_line3 = y_src_line2 + row_bytes;
	unsigned char* y_src_line4 = y_src_line3 + row_bytes;
	unsigned char* y_src_line5 = y_src_line4 + row_bytes;
	unsigned char* y_src_line6 = y_src_line5 + row_bytes;
	unsigned char* y_src_line7 = y_src_line6 + row_bytes;

	unsigned char* uv_src_line0 = src_addr + width*height;
	unsigned char* uv_src_line1 = uv_src_line0 + row_bytes;
	unsigned char* uv_src_line2 = uv_src_line1 + row_bytes;
	unsigned char* uv_src_line3 = uv_src_line2 + row_bytes;

	unsigned char* y_dst_line0 = dst_addr + col_bytes - 8;
	unsigned char* uv_dst_line0 = dst_addr + width*height + col_bytes - 8;

	unsigned char* y_dst_line0_static = y_dst_line0;
	unsigned char* uv_dst_line0_static = uv_dst_line0;

	for (i = 0; i < height; i += 8) {
		for (j = 0; j < width; j += 8) {
			asm volatile
			(
				"mov			 r9,    %9			\n\t"
				 "pld		 [%0,#64]               \n\t"
				 "pld		 [%1,#64]               \n\t"
				 "pld		 [%2,#64]               \n\t"
				 "pld		 [%3,#64]               \n\t"
				 "pld		 [%4,#64]               \n\t"
				 "pld		 [%5,#64]               \n\t"
				 "pld		 [%6,#64]               \n\t"
				 "pld		 [%7,#64]               \n\t"
				"vld1.u8		 {d0},  [%0]!			\n\t"
				"vld1.u8		 {d1},  [%1]!			\n\t"
				"vld1.u8		 {d2},  [%2]!			\n\t"
				"vld1.u8		 {d3},  [%3]!			\n\t"
				"vld1.u8		 {d4},  [%4]!			\n\t"
				"vld1.u8		 {d5},  [%5]!			\n\t"
				"vld1.u8		 {d6},  [%6]!			\n\t"
				"vld1.u8		 {d7},  [%7]!			\n\t"

				"vtrn.8			 d1,  d0				\n\t"
				"vtrn.8			 d3,  d2				\n\t"
				"vtrn.8			 d5,  d4				\n\t"
				"vtrn.8			 d7,  d6				\n\t"

				"vtrn.16		 d3,  d1				\n\t"
				"vtrn.16		 d2,  d0				\n\t"
				"vtrn.16		 d7,  d5				\n\t"
				"vtrn.16		 d6,  d4				\n\t"
				"vtrn.32		 d7,  d3				\n\t"
				"vtrn.32		 d5,  d1				\n\t"
				"vtrn.32		 d6,  d2				\n\t"
				"vtrn.32		 d4,  d0				\n\t"

			"vst1.u8		 {d7},  [%8], r9			\n\t"
			"vst1.u8		 {d6},  [%8], r9			\n\t"
			"vst1.u8		 {d5},  [%8], r9			\n\t"
			"vst1.u8		 {d4},  [%8], r9			\n\t"
			"vst1.u8		 {d3},  [%8], r9		\n\t"
			"vst1.u8		 {d2},  [%8], r9			\n\t"
			"vst1.u8		 {d1},  [%8], r9			\n\t"
			"vst1.u8		 {d0},  [%8], r9		\n\t"

			: "+r" (y_src_line0),
			  "+r" (y_src_line1),
			  "+r" (y_src_line2),
			  "+r" (y_src_line3),
			  "+r" (y_src_line4),
			  "+r" (y_src_line5),
			  "+r" (y_src_line6),
			  "+r" (y_src_line7),
			  "+r" (y_dst_line0),
			  "+r" (col_bytes)
			:
			: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "r9"
		    );
			asm volatile
			(
				"mov r8, %5			\n\t"

				"pld		 [%0,#64]				\n\t"
				"pld		 [%1,#64]				\n\t"
				"pld		 [%2,#64]				\n\t"
				"pld		 [%3,#64]				\n\t"
				"vld1.u8		 {d8}, [%0]!			\n\t"
				"vld1.u8		 {d9}, [%1]!			\n\t"
				"vld1.u8		 {d10}, [%2]!			\n\t"
				"vld1.u8		 {d11}, [%3]!			\n\t"

				"vtrn.16		 d9, d8				\n\t"
				"vtrn.16		 d11, d10			\n\t"
				"vtrn.32		 d11, d9			\n\t"
				"vtrn.32		 d10, d8			\n\t"

				"vst1.u8		 {d11}, [%4], r8			\n\t"
			"vst1.u8		 {d10}, [%4], r8			\n\t"
			"vst1.u8		 {d9}, [%4], r8			\n\t"
			"vst1.u8		 {d8}, [%4], r8			\n\t"

				: "+r" (uv_src_line0),
			  "+r" (uv_src_line1),
			  "+r" (uv_src_line2),
			  "+r" (uv_src_line3),
			  "+r" (uv_dst_line0),
			  "+r" (col_bytes)
			:
			: "cc", "memory", "d8", "d9", "d10", "d11", "r8"
		    );
		}

		y_src_line0 = y_src_line7;
		y_src_line1 += row_bytes_7;
		y_src_line2 += row_bytes_7;
		y_src_line3 += row_bytes_7;
		y_src_line4 += row_bytes_7;
		y_src_line5 += row_bytes_7;
		y_src_line6 += row_bytes_7;
		y_src_line7 += row_bytes_7;

		uv_src_line0 = uv_src_line3;
		uv_src_line1 += row_bytes_3;
		uv_src_line2 += row_bytes_3;
		uv_src_line3 += row_bytes_3;

		y_dst_line0 = y_dst_line0_static - i - 8;
		uv_dst_line0 = uv_dst_line0_static - i - 8;
	}
}

void nv_rotage270(unsigned int width,
		unsigned int height, unsigned char* src_addr,
		unsigned char* dst_addr)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;

	unsigned int row_bytes = width;
	unsigned int col_bytes = height;

	unsigned int row_bytes_7 = width*7;
	unsigned int row_bytes_3 = width*3;

	unsigned char* y_src_line0 = src_addr;
	unsigned char* y_src_line1 = y_src_line0 + row_bytes;
	unsigned char* y_src_line2 = y_src_line1 + row_bytes;
	unsigned char* y_src_line3 = y_src_line2 + row_bytes;
	unsigned char* y_src_line4 = y_src_line3 + row_bytes;
	unsigned char* y_src_line5 = y_src_line4 + row_bytes;
	unsigned char* y_src_line6 = y_src_line5 + row_bytes;
	unsigned char* y_src_line7 = y_src_line6 + row_bytes;

	unsigned char* uv_src_line0 = src_addr + width*height;
	unsigned char* uv_src_line1 = uv_src_line0 + row_bytes;
	unsigned char* uv_src_line2 = uv_src_line1 + row_bytes;
	unsigned char* uv_src_line3 = uv_src_line2 + row_bytes;

	unsigned char* y_dst_line0 = dst_addr + col_bytes*(row_bytes - 8);
	unsigned char* uv_dst_line0 = dst_addr + width*height + col_bytes*(row_bytes - 4)/2;

	unsigned char* y_dst_line0_static = y_dst_line0;
	unsigned char* uv_dst_line0_static = uv_dst_line0;

	for (i = 0; i < height; i += 8) {
		for (j = 0; j < width; j += 8) {
			asm volatile
			(
				"mov			 r9,	%9			\n\t"
				 "pld		 [%0,#64]               \n\t"
				 "pld		 [%1,#64]               \n\t"
				 "pld		 [%2,#64]               \n\t"
				 "pld		 [%3,#64]               \n\t"
				 "pld		 [%4,#64]               \n\t"
				 "pld		 [%5,#64]               \n\t"
				 "pld		 [%6,#64]               \n\t"
				 "pld		 [%7,#64]               \n\t"

				"vld1.u8		 {d0},  [%0]!			\n\t"
				"vld1.u8		 {d1},  [%1]!			\n\t"
				"vld1.u8		 {d2},  [%2]!			\n\t"
				"vld1.u8		 {d3},  [%3]!			\n\t"
				"vld1.u8		 {d4},  [%4]!			\n\t"
				"vld1.u8		 {d5},  [%5]!			\n\t"
				"vld1.u8		 {d6},  [%6]!			\n\t"
				"vld1.u8		 {d7},  [%7]!			\n\t"

				"vtrn.8			 d0,  d1							\n\t"
				"vtrn.8			 d2,  d3							\n\t"
				"vtrn.8			 d4,  d5							\n\t"
				"vtrn.8			 d6,  d7							\n\t"

				"vtrn.16		 d1,  d3							\n\t"
				"vtrn.16		 d0,  d2							\n\t"
				"vtrn.16		 d5,  d7							\n\t"
				"vtrn.16		 d4,  d6							\n\t"

				"vtrn.32		 d3,  d7							\n\t"
				"vtrn.32		 d1,  d5							\n\t"
				"vtrn.32		 d2,  d6							\n\t"
				"vtrn.32		 d0,  d4							\n\t"

			"vst1.u8		 {d7},  [%8], r9			\n\t"
			"vst1.u8		 {d6},  [%8], r9			\n\t"
			"vst1.u8		 {d5},  [%8], r9			\n\t"
			"vst1.u8		 {d4},  [%8], r9			\n\t"
			"vst1.u8		 {d3},  [%8], r9		\n\t"
			"vst1.u8		 {d2},  [%8], r9			\n\t"
			"vst1.u8		 {d1},  [%8], r9			\n\t"
			"vst1.u8		 {d0},  [%8], r9		\n\t"

			: "+r" (y_src_line0),
			  "+r" (y_src_line1),
			  "+r" (y_src_line2),
			  "+r" (y_src_line3),
			  "+r" (y_src_line4),
			  "+r" (y_src_line5),
			  "+r" (y_src_line6),
			  "+r" (y_src_line7),
			  "+r" (y_dst_line0),
			  "+r" (col_bytes)
			:
			: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "r9"
		    );

			asm volatile
			(
				"mov			 r8,    %5			\n\t"

				"pld		 [%0,#64]               \n\t"
				"pld		 [%1,#64]               \n\t"
				"pld		 [%2,#64]               \n\t"
				"pld		 [%3,#64]               \n\t"

				"vld1.u8		 {d8},  [%0]!			\n\t"
				"vld1.u8		 {d9},  [%1]!			\n\t"
				"vld1.u8		 {d10}, [%2]!			\n\t"
				"vld1.u8		 {d11}, [%3]!			\n\t"

				"vtrn.16		 d8,  d9							\n\t"
				"vtrn.16		 d10,  d11							\n\t"

				"vtrn.32		 d9,  d11							\n\t"
				"vtrn.32		 d8,  d10							\n\t"

				"vst1.u8		 {d11}, [%4], r8			\n\t"
			"vst1.u8		 {d10}, [%4], r8			\n\t"
			"vst1.u8		 {d9},  [%4], r8			\n\t"
			"vst1.u8		 {d8},  [%4], r8			\n\t"

				: "+r" (uv_src_line0),
			  "+r" (uv_src_line1),
			  "+r" (uv_src_line2),
			  "+r" (uv_src_line3),
			  "+r" (uv_dst_line0),
			  "+r" (col_bytes)
			:
			: "cc", "memory", "d8", "d9", "d10", "d11", "r8"
		    );

			y_dst_line0 -= 16*col_bytes;
			uv_dst_line0 -= 8*col_bytes;
		}

		y_src_line0 = y_src_line7;
		y_src_line1 += row_bytes_7;
		y_src_line2 += row_bytes_7;
		y_src_line3 += row_bytes_7;
		y_src_line4 += row_bytes_7;
		y_src_line5 += row_bytes_7;
		y_src_line6 += row_bytes_7;
		y_src_line7 += row_bytes_7;

		uv_src_line0 = uv_src_line3;
		uv_src_line1 += row_bytes_3;
		uv_src_line2 += row_bytes_3;
		uv_src_line3 += row_bytes_3;

		y_dst_line0 = y_dst_line0_static + i + 8;

		uv_dst_line0 = uv_dst_line0_static + i + 8;
	}
}

#else
void nv_rotage90(unsigned int width, unsigned int height,
	unsigned char* src_addr, unsigned char* dst_addr)
{
#define NV21 0
#define YU12 1

	unsigned int i = 0;
	unsigned int j = 0;

	unsigned int k = 0;

	unsigned int format = NV21;
	unsigned int row_bytes = width;
	unsigned int col_bytes = height;
	unsigned char* y_src_line0 = src_addr;
	unsigned char* uv_src_line0 = src_addr + width*height;


	unsigned char* y_dst_line0 = dst_addr + col_bytes - 1;
	unsigned char* y_dst_line1 = y_dst_line0 + col_bytes;
	unsigned char* y_dst_line2 = y_dst_line1 + col_bytes;
	unsigned char* y_dst_line3 = y_dst_line2 + col_bytes;
	unsigned char* y_dst_line4 = y_dst_line3 + col_bytes;
	unsigned char* y_dst_line5 = y_dst_line4 + col_bytes;
	unsigned char* y_dst_line6 = y_dst_line5 + col_bytes;
	unsigned char* y_dst_line7 = y_dst_line6 + col_bytes;

	unsigned char* uv_dst_line0 = NULL;
	unsigned char* uv_dst_line0_v = NULL;
	unsigned char* uv_dst_line0_v_static = NULL;

	if (format == YU12) {
		uv_dst_line0 = dst_addr + width*height + col_bytes/2 - 1;
		uv_dst_line0_v = dst_addr + width*height*5/4 + col_bytes/2 - 1;

		uv_dst_line0_v_static = uv_dst_line0_v;
	} else if (format == NV21) {
		uv_dst_line0 = dst_addr + width*height + col_bytes - 2;
	}

	unsigned char* y_dst_line0_static = y_dst_line0;
	unsigned char* uv_dst_line0_static = uv_dst_line0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			*(y_dst_line0) = *(y_src_line0++);
			y_dst_line0 += height;
		}
		y_dst_line0 = y_dst_line0_static - i - 1;
	}

	if (format == NV21) {
		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0) = *(uv_src_line0++);
				*(uv_dst_line0+1) = *(uv_src_line0++);
				uv_dst_line0 += height;
			}
			uv_dst_line0 = uv_dst_line0_static - 2*(i+1);
		}
	} else if (format == YU12) {
		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0) = *(uv_src_line0++);
				uv_dst_line0 += height/2;
			}
			uv_dst_line0 = uv_dst_line0_static - (i+1);
		}

		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0_v) = *(uv_src_line0++);
				uv_dst_line0_v += height/2;
			}
			uv_dst_line0_v = uv_dst_line0_v_static - (i+1);
		}
	}
}

void nv_rotage270(unsigned int width, unsigned int height,
	unsigned char* src_addr, unsigned char* dst_addr)
{
#define NV21 1
#define YU12 0

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;

	unsigned int format = NV21;
	unsigned int row_bytes = width;
	unsigned int col_bytes = height;

	unsigned char* y_src_line0 = src_addr;
	unsigned char* uv_src_line0 = src_addr + width*height;

	unsigned char* y_dst_line0 = dst_addr + col_bytes*(row_bytes - 1);
	unsigned char* y_dst_line1 = y_dst_line0 + col_bytes;
	unsigned char* y_dst_line2 = y_dst_line1 + col_bytes;
	unsigned char* y_dst_line3 = y_dst_line2 + col_bytes;
	unsigned char* y_dst_line4 = y_dst_line3 + col_bytes;
	unsigned char* y_dst_line5 = y_dst_line4 + col_bytes;
	unsigned char* y_dst_line6 = y_dst_line5 + col_bytes;
	unsigned char* y_dst_line7 = y_dst_line6 + col_bytes;

	unsigned char* uv_dst_line0 = NULL;
	unsigned char* uv_dst_line0_v = NULL;
	unsigned char* uv_dst_line0_v_static = NULL;

	if (format == YU12) {
		uv_dst_line0 = dst_addr + width*height;
		uv_dst_line0_v = dst_addr + width*height*5/4;

		uv_dst_line0_v_static = uv_dst_line0_v;
	} else if (format == NV21) {
		uv_dst_line0 = dst_addr + width*height + col_bytes*(row_bytes - 1)/2;
	}

	unsigned char* y_dst_line0_static = y_dst_line0;
	unsigned char* uv_dst_line0_static = uv_dst_line0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			*(y_dst_line0) = *(y_src_line0++);
			y_dst_line0 -= height;
		}
		y_dst_line0 = y_dst_line0_static+ i + 1;
	}

	if (format == NV21) {
		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0) = *(uv_src_line0++);
				*(uv_dst_line0+1) = *(uv_src_line0++);
				uv_dst_line0 -= height;
			}
			uv_dst_line0 = uv_dst_line0_static + 2*(i+1);
		}
	} else if (format == YU12) {
		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0) = *(uv_src_line0++);
				uv_dst_line0 += height/2;
			}
			uv_dst_line0 = uv_dst_line0_static + (i+1);
		}

		for (i = 0; i < height/2; i++) {
			for (j = 0; j < width/2; j++) {
				*(uv_dst_line0_v) = *(uv_src_line0++);
				uv_dst_line0_v += height/2;
			}
			uv_dst_line0_v = uv_dst_line0_v_static + (i+1);
		}
	}
}
#endif
