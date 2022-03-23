
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
#include "rnet.h"
#include <stdio.h>
#include <cstdio>

#ifndef nullptr
#define nullptr 				0
#endif


unsigned int run_rnet(){

	RNet rNet;
	hw_init();
	const char* img_path = "./prelu_rnet/rnet_192_imgs.bin";

	MEM_CTRL input_d_mem;
	mem_alloc(&input_d_mem, 0x300000);
	mem_load_bin(&input_d_mem, img_path);

	//dump image data
	MEM_CTRL ouput_mem;
	mem_alloc(&ouput_mem, 2000);
		
	unsigned int size = 16; //output
	unsigned int d_size = 0;

	char* d_ptr = (char*)input_d_mem.data_addr;

	d_ptr += 4;
	
	rNet.load_data((char*)"./prelu_rnet/data-20200111145654.bin");

	FILE* fp = NULL;
	fp = fopen("/mnt/extsd/rnet.txt", "w");
	if (fp ==NULL){
		printf("Failed to open the file!\n");
	}
	else{
		printf("Successed to open the file!\n");
	}
	
	for (int i = 0; i < 192; i++){
		
		d_size = 1728;//24*24*3
		rNet.load_input_data(d_ptr,d_size);
		d_ptr += d_size + 4;

		rNet.run(ouput_mem.data_addr, ouput_mem.size);
		for (unsigned int j = 0; j < size; j++){
			
			signed char out = ((signed char*)ouput_mem.data_addr)[j];
			fprintf(fp, "%x\t", out);
		//	printf("%x\t", out);
	
		}
		fprintf(fp, "\n");
	//	printf("\n");
	}
	fclose(fp);
	fp =NULL;

	return 1;
}

int main(int argc, char **argv)
{
	run_rnet();

    return 1;
}
