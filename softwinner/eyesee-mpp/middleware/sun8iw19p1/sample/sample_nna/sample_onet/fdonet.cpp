
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <linux/videodev2.h>
#include "ipu_runtime.h"
#include "hw_adaptor.h"
#include "mem_ctrl.h"
#include<sys/time.h>
#include "ponet.h"
#include <stdio.h>
#include <cstdio>

#ifndef nullptr
#define nullptr 				0
#endif


unsigned int run_onet1();
unsigned int run_onet();


unsigned int run_onet1()
{
	PONet nONet;
	hw_init();
	const char* y_path = "./prelu_onet/answer.bin";
	const char* img_path = "./prelu_onet/input_50.bin";

	MEM_CTRL y_d_mem;
	mem_alloc(&y_d_mem, 0x10000);
	mem_load_bin(&y_d_mem, y_path);

	MEM_CTRL input_d_mem;
	mem_alloc(&input_d_mem, 0x300000);
	mem_load_bin(&input_d_mem, img_path);

	//dump image data
	MEM_CTRL ouput_mem;
	mem_alloc(&ouput_mem, 128);

	char* d_ptr = (char*)input_d_mem.data_addr;
	unsigned int d_size = 0;
	char* y_ptr = (char*)y_d_mem.data_addr;
	
	y_ptr = y_ptr + 4;

	d_ptr += 4;
	float sub_total = 0.0;
	
	float max_f = 0.0;
	int error_count = 0;
	
	nONet.loaddata((char*)"./prelu_onet/data-20190723105355.bin");
	
	for (int i = 0; i < 192; i++){
		d_size = 6912;//48*48*3
		nONet.loadinputdata(d_ptr,d_size);
		d_ptr += d_size + 4;
		unsigned int compare_size = 2;
		float scale_factor = 28;
		int true_idx = 0;
		max_f = -100000;
		nONet.run2(ouput_mem.data_addr, ouput_mem.size);
		for (unsigned int j = 0; j < compare_size; j++)
		{
			signed char out = ((signed char*)ouput_mem.data_addr)[j];
			float out_f = ((float)out) / scale_factor;
			if (out_f > max_f)
			{
				max_f = out_f;
				true_idx = j;
			}
		}
		printf("\n[%d]::  idx = %d,answer %d,%d,%d\n", i, y_ptr[i], (signed char)true_idx, ((signed char*)ouput_mem.data_addr)[0], ((signed char*)ouput_mem.data_addr)[1]);
		if (y_ptr[i] != ((char)true_idx))
		{
			printf("===========error %d===========%d,%d\n", i, y_ptr[i], (char)true_idx);
			error_count++;
		}
		
	}
	printf("sub_total = %f,error_count = %d\n", sub_total, error_count);

	return 1;
}

unsigned int run_onet()
{
	PONet nONet;
	hw_init();
	const char* img_path = "./prelu_onet/input_50.bin";

	MEM_CTRL input_d_mem;
	mem_alloc(&input_d_mem, 0x300000);
	mem_load_bin(&input_d_mem, img_path);

	//dump image data
	MEM_CTRL ouput_mem;
	mem_alloc(&ouput_mem, 2000);

	unsigned int d_size = 6912; //48 * 48 *3;
	unsigned int size = 16; //output

	char* d_ptr = (char*)input_d_mem.data_addr;
	d_ptr += 4;

//	nONet.loaddata((char*)"./prelu_onet/data-20190723105355.bin");
	nONet.loaddata((char*)"./prelu_onet/data-20200111145138.bin");

	FILE* fp = NULL;
	fp = fopen("/mnt/extsd/onet.txt", "w");

	if (fp ==NULL){
		printf("Failed to open the file!\n");
	}
	else{
		printf("Successed to open the file!\n");
	}

	for (int i = 0; i < 192; i++){	
		nONet.loadinputdata(d_ptr,d_size);
		d_ptr += d_size + 4;

		nONet.run2(ouput_mem.data_addr, ouput_mem.size);

		for (unsigned int j = 0; j < size; j++){	
			signed char out = ((signed char*)ouput_mem.data_addr)[j];
			fprintf(fp, "%x\t", out);
			printf("%x\t", out);
		}
		fprintf(fp, "\n");
		printf("\n");
	}
	fclose(fp);
	fp =NULL;

	return 1;
}

int main(int argc, char **argv)
{
	run_onet();
	//run_onet1();
	
    return 1;
}
