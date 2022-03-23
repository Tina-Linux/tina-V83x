#include <stdio.h>

#include <ipu_runtime.h>
#include "rnet.h"
#include "hw_adaptor.h"
#include "mem_ctrl.h"

#include <math.h>
#ifdef __ARM__
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <sys/ioctl.h>
#include <unistd.h>
#else
#include "win_platform.h"
#endif

extern int timeval_subtract(struct timeval* diff, struct timeval* start, struct timeval* stop);

RNet::RNet() {
	aw_ai_program_init();

	void* tmp_paddr;
	void* tmp_vaddr;

	dma_mem_alloc(0x800000, (&tmp_vaddr), (&tmp_paddr));

	gp_paddr = tmp_paddr;
	gp_vaddr = tmp_vaddr;

	PNET_IMG_DATA_IN_ADDR = (uint32_t)(gp_paddr);
	PNET_WEIGHT_IN_ADDR = (uint32_t)(gp_paddr)+0x00100000;
	PNET_BIAS_IN_ADDR = (uint32_t)(gp_paddr)+0x00240000;
	PNET_RELU_IN_ADDR = (uint32_t)(gp_paddr)+0x00380000;
	PNET_OUTPUT_ADDR = (uint32_t)(gp_paddr)+0x00400000;
}

RNet::~RNet() {
	printf("~~RNet\n");
	dma_mem_free(gp_vaddr);
}

void RNet::load_input_data(void* img_addr, uint32_t data_size) {
	dma_loadin((char*)img_addr, data_size, PNET_IMG_DATA_IN_ADDR);
	input_addrs.clear();
	input_addrs.push_back(PNET_IMG_DATA_IN_ADDR);
}

void RNet::load_data(char* data_path) {
	MEM_CTRL data_mem;
	mem_alloc(&data_mem, 0x400000);
	size_t d_size = mem_load_bin(&data_mem, data_path);

	int32_t* ptr_data = (int32_t*)data_mem.data_addr;
	int8_t* ptr_d_data = (int8_t*)ptr_data + d_size;

	int weight_idx = 0;
	int bias_idx = 0;
	int prelu_idx = 0;

	int32_t w_offset = 0;
	int32_t b_offset = 0;
	int32_t r_offset = 0;

	while (ptr_d_data > (int8_t*)(ptr_data)) {
		int32_t dev_type = ptr_data[0];
		int32_t op_type = ptr_data[1];
	    int32_t blob_type = ptr_data[2];
		int32_t scale = ptr_data[3];
		int32_t bits = ptr_data[4];

		cvt_scales.push_back(scale);
		cvt_bits.push_back(bits);

		switch (dev_type)
		{
		case 0:   //cpu
			break;
		case 1:   //gpu
			break;
		case 2:	  //npu
			break;
		case 3:   //IPU
		{
			//read weights and biases
			int8_t* ptr_cdata = (int8_t*)(&ptr_data[5]);
			ptr_data = (int32_t*)ptr_cdata;
			int32_t weight_size = ptr_data[0];

			weight_addrs.push_back(PNET_WEIGHT_IN_ADDR + w_offset);
			w_offset += weight_size;
			w_offset += 128 - w_offset % 128;

			dma_loadin(ptr_cdata + 4, weight_size, weight_addrs[weight_idx++]);
			ptr_cdata += 4 + weight_size;
			ptr_data = (int32_t*)ptr_cdata;

			int32_t bias_size = ptr_data[0] * 2;
			bias_addrs.push_back(PNET_BIAS_IN_ADDR + b_offset);
			b_offset += bias_size;
			b_offset += 128 - b_offset % 128;

			dma_loadin(ptr_cdata + 4, bias_size, bias_addrs[bias_idx++]);
			ptr_cdata += 4 + bias_size;
			ptr_data = (int32_t*)ptr_cdata;

			// prelu or relu or others
			if ((op_type == 0) || (op_type == 1) || (op_type == 4)) {
				int32_t prelu_size = ptr_data[0] * 2;
				prelu_addrs.push_back(PNET_RELU_IN_ADDR + r_offset);

				r_offset += prelu_size;
				r_offset += 128 - r_offset % 128;

				dma_loadin(ptr_cdata + 4, prelu_size, prelu_addrs[prelu_idx++]);
				ptr_cdata += 4 + prelu_size;
				ptr_data = (int32_t*)ptr_cdata;
			}
			break;

		}
		}
	}

	uint32_t out_addr = PNET_OUTPUT_ADDR;
	uint32_t out_offset = 0;
	uint32_t out_size = 0;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 5 * 5 * 10 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 3 * 3 * 16 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 1 * 1 * 1024 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 1 * 1 * 1024 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 1 * 1 * 1024 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);

	out_size = 1 * 1 * 1024 + 1024 * 10;
	out_size += 128 - out_size % 128;
	out_offset += out_size;
	output_addrs.push_back(out_addr + out_offset);
}

void RNet::run(void* rtl_addr, uint32_t data_size) {

	conv1(input_addrs[0], weight_addrs[0], bias_addrs[0], prelu_addrs[0], output_addrs[0]);	
	hw_reset();
	conv2(output_addrs[0], weight_addrs[1], bias_addrs[1], prelu_addrs[1], output_addrs[1]);
	hw_reset();
	conv3(output_addrs[1], weight_addrs[2], bias_addrs[2], prelu_addrs[2], output_addrs[2]);
	hw_reset();
	conv4(output_addrs[2], weight_addrs[3], bias_addrs[3], prelu_addrs[3], output_addrs[3]);
	hw_reset();

	conv4_1(output_addrs[3], weight_addrs[4], bias_addrs[4], output_addrs[4]);
	hw_reset();
	conv4_2(output_addrs[3], weight_addrs[5], bias_addrs[5], output_addrs[5]);
	hw_reset();
	conv4_3(output_addrs[3], weight_addrs[6], bias_addrs[6], output_addrs[6]);
	hw_reset();

	int pt_size = 2;
	int pt_size1 = 4;
	int pt_size2 = 10;
	dma_loadout(output_addrs[4], pt_size, rtl_addr);
	dma_loadout(output_addrs[5], pt_size1, (uint8_t*)rtl_addr + pt_size);
	dma_loadout(output_addrs[6], pt_size2, (uint8_t*)rtl_addr + pt_size + pt_size1);
}

void RNet::conv1(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* performance parameters */
	conv_op.first_layer = 1;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 24 * 3;
	conv_op.input_height = 24;
	conv_op.input_channel = 1;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3 * 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 1;
	conv_op.kernel_num = 28;

	conv_op.conv_stride_x = 3;
	conv_op.conv_stride_y = 1;

	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0;    /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0;    /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0;    /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.mean_gu = 0;

	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scales[0];
	conv_op.truncate = cvt_bits[0];

	conv_op.offset = 0;

	conv_op.output_data_addr = 0;
	conv_op.output_width = 22;
	conv_op.output_height = 22;
	conv_op.output_channel = 28;
	conv_op.data_format = 0;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 2;
	conv_op.act_data_addr = prelu_addr;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;

	struct aw_ai_pool_op_desc pool_op;
	pool_op.pool_mode = POOL_MODE_MAX;
	pool_op.input_addr = 0;
	pool_op.input_width = 22;
	pool_op.input_height = 22;
	pool_op.input_channel = 28;
	pool_op.pad_num = -1;
	pool_op.pad_left = 0;
	pool_op.pad_top = 0;
	pool_op.pad_right = 1;
	pool_op.pad_bottom = 1;

	pool_op.stride_x = 2;
	pool_op.stride_y = 2;
	pool_op.pool_width = 3;
	pool_op.pool_height = 3;
	pool_op.output_width = 11;
	pool_op.output_height = 11;
	pool_op.output_channel = 28;
	pool_op.output_addr = output_addr;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;
	pool_op.reverse1 = 0;
	pool_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);
	unsigned int id2 = aw_ai_pool_program(pool_op);

	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_POOL_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
	aw_ai_op_completion(id2);
}

void RNet::conv2(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 11;
	conv_op.input_height = 11;
	conv_op.input_channel = 28;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 28;
	conv_op.kernel_num = 48;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;

	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0;    /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0;    /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0;    /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.mean_gu = 0;

	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scales[1];
	conv_op.truncate = cvt_bits[1];

	conv_op.offset = 0;

	conv_op.output_data_addr = 0;
	conv_op.output_width = 9;
	conv_op.output_height = 9;
	conv_op.output_channel = 48;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 2;
	conv_op.act_data_addr = prelu_addr;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	struct aw_ai_pool_op_desc pool_op;
	pool_op.pool_mode = POOL_MODE_MAX;
	pool_op.input_addr = 0;
	pool_op.input_width = 9;
	pool_op.input_height = 9;
	pool_op.input_channel = 48;
	pool_op.pad_num = 0;
	pool_op.pad_left = 0;
	pool_op.pad_top = 0;
	pool_op.pad_right = 0;
	pool_op.pad_bottom = 0;

	pool_op.stride_x = 2;
	pool_op.stride_y = 2;
	pool_op.pool_width = 3;
	pool_op.pool_height = 3;
	pool_op.output_width = 4;
	pool_op.output_height = 4;
	pool_op.output_channel = 48;
	pool_op.output_addr = output_addr;
	
	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;
	pool_op.reverse1 = 0;
	pool_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);
	unsigned int id2 = aw_ai_pool_program(pool_op);

	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_POOL_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
	aw_ai_op_completion(id2);
}

void RNet::conv3(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 4;
	conv_op.input_height = 4;
	conv_op.input_channel = 48;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 2;
	conv_op.kernel_height = 2;
	conv_op.kernel_channel = 48;
	conv_op.kernel_num = 64;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;

	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0;    /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0;    /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0;    /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.mean_gu = 0;

	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scales[2];
	conv_op.truncate = cvt_bits[2];

	conv_op.offset = 0;

	conv_op.output_data_addr = output_addr;
	conv_op.output_width = 3;
	conv_op.output_height = 3;
	conv_op.output_channel = 64;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 2;
	conv_op.act_data_addr = prelu_addr;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);

	unsigned int i = 0;
	unsigned int b = 0;	
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
}

void RNet::conv4(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 3;
	conv_op.input_height = 3;
	conv_op.input_channel = 64;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 64;
	conv_op.kernel_num = 128;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;
	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scales[3];
	conv_op.truncate = cvt_bits[3];

	conv_op.offset = 0;

	conv_op.output_data_addr = output_addr; 
	conv_op.output_width = 1;
	conv_op.output_height = 1;
	conv_op.output_channel = conv_op.kernel_num;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 1;
	conv_op.act_data_addr = 1;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;
	unsigned int id1 = aw_ai_conv_program(conv_op);
	
	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
}

void RNet::conv4_1(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 128;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 128;
	conv_op.kernel_num = 2;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;
	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;

	conv_op.scale = cvt_scales[4];
	conv_op.truncate = cvt_bits[4];
	conv_op.offset = 0;

	conv_op.output_data_addr = output_addr;
	conv_op.output_width = 1;
	conv_op.output_height = 1;
	conv_op.output_channel = 2;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 0;
	conv_op.act_data_addr = 1;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);
	
	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
}

void RNet::conv4_2(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 128;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 128;
	conv_op.kernel_num = 4;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;
	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;

	conv_op.scale = cvt_scales[5];
	conv_op.truncate = cvt_bits[5];
	conv_op.offset = 0;

	conv_op.output_data_addr = output_addr;
	conv_op.output_width = 1;
	conv_op.output_height = 1;
	conv_op.output_channel = 4;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 0;
	conv_op.act_data_addr = 1;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);
	
	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
}

void RNet::conv4_3(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr) {
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = input_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 128;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 128;
	conv_op.kernel_num = 10;

	conv_op.conv_stride_x = 1;
	conv_op.conv_stride_y = 1;
	conv_op.pad_x_left = 0;
	conv_op.pad_x_right = 0;
	conv_op.pad_y_top = 0;
	conv_op.pad_y_bottom = 0;
	conv_op.dilation_x = 1;
	conv_op.dilation_y = 1;

	conv_op.mean_enable = 0;
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;

	conv_op.scale = cvt_scales[6];
	conv_op.truncate = cvt_bits[6];
	conv_op.offset = 0;

	conv_op.output_data_addr = output_addr;
	conv_op.output_width = 1;
	conv_op.output_height = 1;
	conv_op.output_channel = 10;
	conv_op.data_format = FORMAT_FEATURE;

	conv_op.bias_mode = 2;
	conv_op.bias_data_addr = bias_addr;

	conv_op.act_enbale = 0;
	conv_op.act_data_addr = 1;

	conv_op.bn_enbale = 0;
	conv_op.bn_data_addr = 1;

	conv_op.eltwise_enbale = 0;
	conv_op.eltwise_data_addr = 1;

	conv_op.reverse1 = 0;
	conv_op.reverse2 = 0;

	unsigned int id1 = aw_ai_conv_program(conv_op);
	
	unsigned int i = 0;
	unsigned int b = 0;
	do {
		usleep(5);
		b = readl_sig();
		if (b == CONV_ACT_SIG)
			break;
		usleep(5);
	} while (i < 1000000);

	aw_ai_op_completion(id1);
}
