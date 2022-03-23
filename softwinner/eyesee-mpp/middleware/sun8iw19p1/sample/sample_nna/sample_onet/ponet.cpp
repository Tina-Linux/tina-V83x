#include <stdio.h>

#include <ipu_runtime.h>
#include "ponet.h"
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

PONet::PONet()
{
	aw_ai_program_init();
    void* tmp_vaddr;
	void* tmp_paddr;
	dma_mem_alloc(0x800000,(&tmp_vaddr),(&tmp_paddr));
	gp_paddr = tmp_paddr;
	gp_vaddr = tmp_vaddr;
	ONET_IMG_DATA_IN_ADDR = (uint32_t)(gp_paddr);
	onet_WEIGHT_IN_ADDR = (uint32_t)(gp_paddr) + 0x00100000;
	onet_BAIS_IN_ADDR = (uint32_t)(gp_paddr) + 0x00240000;
	ONet_RELU_IN_ADDR = (uint32_t)(gp_paddr) + 0x00380000;
	onet_OUT_ADDR = (uint32_t)(gp_paddr) + 0x00400000;
	
}
PONet::~PONet()
{
    printf("~~PONet\n");
    dma_mem_free(gp_vaddr);
}

void PONet::loadinputdata(void* img_addr, uint32_t data_size)
{
    dma_loadin((char*)img_addr, data_size, ONET_IMG_DATA_IN_ADDR);
	in_addr.clear();
	in_addr.push_back(ONET_IMG_DATA_IN_ADDR);
}

void PONet::loaddata2(char* weight_path, char* bias_path, char* prelu_path)
{

	MEM_CTRL input_w_mem;
	MEM_CTRL input_bias_mem;
	MEM_CTRL input_prelu_mem;
	MEM_CTRL ouput_mem;
	MEM_CTRL real_ouput_mem;

	mem_alloc(&input_w_mem, 0x400000);
	mem_load_bin(&input_w_mem, weight_path);
	mem_alloc(&input_bias_mem, 0x10000);
	mem_load_bin(&input_bias_mem, bias_path);
	mem_alloc(&input_prelu_mem, 0x10000);
	mem_load_bin(&input_prelu_mem, prelu_path);

	//dump weight data
	int8_t* ptr_weight = (int8_t*)input_w_mem.data_addr;
	uint32_t f_offset = 0;
	uint32_t w_offset = 0;
	uint32_t weight0_size = ((uint32_t*)(ptr_weight + w_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight0_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight0_size;
	f_offset = f_offset + 4 + weight0_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight1_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + 4 + f_offset, weight1_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight1_size;
	f_offset = f_offset + 4 + weight1_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight2_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight2_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight2_size;
	f_offset = f_offset + 4 + weight2_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight3_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight3_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight3_size;
	f_offset = f_offset + 4 + weight3_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight4_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight4_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight4_size;
	f_offset = f_offset + 4 + weight4_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight5_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight5_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight5_size;
	f_offset = f_offset + 4 + weight5_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight6_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight6_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight6_size;
	f_offset = f_offset + 4 + weight6_size;
	w_offset = w_offset + (128 - w_offset % 128);

	uint32_t weight7_size = ((uint32_t*)(ptr_weight + f_offset))[0];
	dma_loadin(ptr_weight + f_offset + 4, weight7_size, onet_WEIGHT_IN_ADDR + w_offset);
	weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
	w_offset = w_offset + 4 + weight7_size;
	f_offset = f_offset + 4 + weight7_size;
	w_offset = w_offset + (128 - w_offset % 128);

	//dump bias data
	int8_t* ptr_bias = (int8_t*)input_bias_mem.data_addr;
	uint32_t b_offset = 0;
	uint32_t bf_offset = 0;
	uint32_t bias0_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias0_size, onet_BAIS_IN_ADDR);
	b_offset = 4 + bias0_size;//because int16 ,so 4->2
	bf_offset = bf_offset + 4 + bias0_size;
	b_offset = b_offset + (128 - b_offset % 128);
	bias_addr.push_back(onet_BAIS_IN_ADDR);

	uint32_t bias1_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias1_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias1_size;
	bf_offset = bf_offset + 4 + bias1_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias2_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias2_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias2_size;
	bf_offset = bf_offset + 4 + bias2_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias3_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias3_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias3_size;
	bf_offset = bf_offset + 4 + bias3_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias4_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias4_size, onet_BAIS_IN_ADDR + b_offset);
	
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias4_size;
	bf_offset = bf_offset + 4 + bias4_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias5_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	
	dma_loadin(ptr_bias + bf_offset + 4, bias5_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias5_size;
	bf_offset = bf_offset + 4 + bias5_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias6_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias6_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias6_size;
	bf_offset = bf_offset + 4 + bias6_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t bias7_size = ((uint32_t*)(ptr_bias + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_bias + bf_offset + 4, bias7_size, onet_BAIS_IN_ADDR + b_offset);
	bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + bias7_size;
	bf_offset = bf_offset + 4 + bias7_size;
	b_offset = b_offset + (128 - b_offset % 128);

	int8_t* ptr_prelu = (int8_t*)input_prelu_mem.data_addr;
	b_offset = 0;
	bf_offset = 0;
	uint32_t prelu0_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu0_size, ONet_RELU_IN_ADDR);
	b_offset = 4 + prelu0_size;//because int16 ,so 4->2
	bf_offset = bf_offset + 4 + prelu0_size;
	b_offset = b_offset + (128 - b_offset % 128);
	prelu_addr.push_back(ONet_RELU_IN_ADDR);

	uint32_t prelu1_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu1_size, ONet_RELU_IN_ADDR + b_offset);
	prelu_addr.push_back(ONet_RELU_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + prelu1_size;
	bf_offset = bf_offset + 4 + prelu1_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t prelu2_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu2_size, ONet_RELU_IN_ADDR + b_offset);
	prelu_addr.push_back(ONet_RELU_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + prelu2_size;
	bf_offset = bf_offset + 4 + prelu2_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t prelu3_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu3_size, ONet_RELU_IN_ADDR + b_offset);
	prelu_addr.push_back(ONet_RELU_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + prelu3_size;
	bf_offset = bf_offset + 4 + prelu3_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t prelu4_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu4_size, ONet_RELU_IN_ADDR + b_offset);
	
	prelu_addr.push_back(ONet_RELU_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + prelu4_size;
	bf_offset = bf_offset + 4 + prelu4_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t prelu5_size = ((uint32_t*)(ptr_prelu + bf_offset))[0] * sizeof(int16_t);
	dma_loadin(ptr_prelu + bf_offset + 4, prelu5_size, ONet_RELU_IN_ADDR + b_offset);
	prelu_addr.push_back(ONet_RELU_IN_ADDR + b_offset);
	b_offset = b_offset + 4 + prelu5_size;
	bf_offset = bf_offset + 4 + prelu5_size;
	b_offset = b_offset + (128 - b_offset % 128);

	uint32_t o_addr = onet_OUT_ADDR;
	uint32_t o_offset = 0;
	uint32_t o_size = 0;
	out_addr.push_back(o_addr + o_offset);

	o_size = 5 * 5 * 32 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 3 * 3 * 64 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);
}

void PONet::loaddata(char* data_path)
{
	MEM_CTRL data_mem;
	mem_alloc(&data_mem, 0x400000);
	size_t d_size = mem_load_bin(&data_mem, data_path);

	int32_t* ptr_data = (int32_t*)data_mem.data_addr;
	int8_t* ptr_d_data = (int8_t*)ptr_data + d_size;
	uint32_t d_offset = 0;
	int weight_idx = 0;
	int bias_idx = 0;
	int prelu_idx = 0;

	int32_t w_offset = 0;
	int32_t b_offset = 0;
	int32_t r_offset = 0;

	while (ptr_d_data > (int8_t*)(ptr_data))
	{
		int32_t dev_type = ptr_data[0];
		int32_t op_type = ptr_data[1];
		int32_t blob_id = ptr_data[2];
		int32_t scale = ptr_data[3];
		int32_t bits = ptr_data[4];

		cvt_scale.push_back(scale);
		cvt_bits.push_back(bits);

		switch (dev_type)
		{
		case 0://cpu
			break;
		case 1://gpu
			break;
		case 2://npu
			break;
		case 3://IPU
		{
				   //read weight and bias  
				   int8_t* ptr_cdata = (int8_t*)(&ptr_data[5]);
				   int count = 0;
				   ptr_data = (int32_t*)ptr_cdata;
				   int32_t weight_size = ptr_data[0];

				   weight_addr.push_back(onet_WEIGHT_IN_ADDR + w_offset);
				   w_offset = w_offset + weight_size;
				   w_offset = w_offset + (128 - w_offset % 128);

				   dma_loadin(ptr_cdata + 4, weight_size, weight_addr[weight_idx++]);
				   ptr_cdata = ptr_cdata + 4 + weight_size;
				   ptr_data = (int32_t*)ptr_cdata;

				   int32_t bias_size = ptr_data[0] * 2;
				   bias_addr.push_back(onet_BAIS_IN_ADDR + b_offset);
				   b_offset = b_offset + bias_size;
				   b_offset = b_offset + (128 - b_offset % 128);
				   

				   dma_loadin(ptr_cdata + 4, bias_size, bias_addr[bias_idx++]);

				   
				   ptr_cdata = ptr_cdata + 4 + bias_size;
				   ptr_data = (int32_t*)ptr_cdata;
				   
				   // prelu or relu or others
				   if ((op_type == 0) || (op_type == 1) || (op_type == 4)){
					   int32_t prelu_size = ptr_data[0] * 2;
					   prelu_addr.push_back(ONet_RELU_IN_ADDR + r_offset);
					   r_offset = r_offset + prelu_size;
					   r_offset = r_offset + (128 - r_offset % 128);
					   dma_loadin(ptr_cdata + 4, prelu_size, prelu_addr[prelu_idx++]);
					   ptr_cdata = ptr_cdata + 4 + prelu_size;
					   ptr_data = (int32_t*)ptr_cdata;
				   }
				   break;
		}	
		}
	}

	uint32_t o_addr = onet_OUT_ADDR;
	uint32_t o_offset = 0;
	uint32_t o_size = 0;
	out_addr.push_back(o_addr + o_offset);

	o_size = 5 * 5 * 32 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 3 * 3 * 64 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);

	o_size = 1 * 1 * 1024 + 1024 * 10;
	o_size = o_size + (128 - o_size % 128);
	o_offset = o_offset + o_size;
	out_addr.push_back(o_addr + o_offset);
}


struct timeval g_start,g_stop,g_diff; 

void PONet::run2(void* rtl_addr, uint32_t data_size)
{
	int i = 0;
	conv1(in_addr[0], weight_addr[0], bias_addr[0], prelu_addr[0],out_addr[0]);
	hw_reset();
	conv2(out_addr[0], weight_addr[1], bias_addr[1], prelu_addr[1], out_addr[1]);
	hw_reset();
	conv3(out_addr[1], weight_addr[2], bias_addr[2], prelu_addr[2], out_addr[2]);
	hw_reset();
    conv4(out_addr[2], weight_addr[3], bias_addr[3], prelu_addr[3], out_addr[3]);
	hw_reset();
	for(int i = 0;i<2;i++)
	{
		conv5(out_addr[3], weight_addr[4]+i*3*3*128*128, bias_addr[4]+i*128*2, out_addr[4]+i*128);
		hw_reset();
	}
	
    conv5_1(out_addr[4], weight_addr[5], bias_addr[5], out_addr[5]);
	hw_reset();
	conv5_2(out_addr[4], weight_addr[6], bias_addr[6], out_addr[6]);
	hw_reset();
	conv5_3(out_addr[4], weight_addr[7], bias_addr[7], out_addr[7]);
	hw_reset();
	
	int pt_size = 2;
	int pt_size1 = 4;
	int pt_size2 = 10;
	dma_loadout(out_addr[5], pt_size, rtl_addr);
	dma_loadout(out_addr[6], pt_size1, (uint8_t*)rtl_addr + pt_size);
	dma_loadout(out_addr[7], pt_size2, (uint8_t*)rtl_addr + pt_size + pt_size1);
}

extern "C" PREG_LIST ptr_greg_list;

void* PONet::run_config(void* rtl_addr, uint32_t data_size)
{
	int i = 0;
	conv_info = new REG_LIST[9];
	PREG_LIST ptr_reg = (PREG_LIST)conv_info;
	
	ptr_greg_list = &ptr_reg[0];
	ptr_greg_list->num = 0;
    conv1_info = ptr_greg_list;
	conv1(in_addr[0], weight_addr[0], bias_addr[0], prelu_addr[0],out_addr[0]);
	hw_reset();

	ptr_greg_list = &ptr_reg[1];
	ptr_greg_list->num = 0;
	conv2_info = ptr_greg_list;
	conv2(out_addr[0], weight_addr[1], bias_addr[1], prelu_addr[1], out_addr[1]);
	hw_reset();
	
	ptr_greg_list = &ptr_reg[2];
	ptr_greg_list->num = 0;
	conv3_info = ptr_greg_list;
	conv3(out_addr[1], weight_addr[2], bias_addr[2], prelu_addr[2], out_addr[2]);
	hw_reset();
  
	ptr_greg_list = &ptr_reg[3];
	ptr_greg_list->num = 0;
	conv4_info = ptr_greg_list;
	conv4(out_addr[2], weight_addr[3], bias_addr[3], prelu_addr[3], out_addr[3]);
	hw_reset();
      
	ptr_greg_list = &ptr_reg[4];
	ptr_greg_list->num = 0;
	conv501_info = ptr_greg_list;
	conv5(out_addr[3], weight_addr[4]+0*3*3*128*128, bias_addr[4]+0*128*2, out_addr[4]+0*128);
	hw_reset();
  
	ptr_greg_list = &ptr_reg[5];
	ptr_greg_list->num = 0;
	conv502_info = ptr_greg_list;
	conv5(out_addr[3], weight_addr[4]+1*3*3*128*128, bias_addr[4]+1*128*2, out_addr[4]+1*128);
	hw_reset();
	
    ptr_greg_list = &ptr_reg[6];
	ptr_greg_list->num = 0;
	conv51_info = ptr_greg_list;
	conv5_1(out_addr[4], weight_addr[5], bias_addr[5], out_addr[5]);
	hw_reset();
	
	ptr_greg_list = &ptr_reg[7];
	ptr_greg_list->num = 0;
	conv52_info = ptr_greg_list;
	conv5_2(out_addr[4], weight_addr[6], bias_addr[6], out_addr[6]);
	hw_reset();
  
	ptr_greg_list = &ptr_reg[8];
	ptr_greg_list->num = 0;
	conv53_info = ptr_greg_list;
	conv5_3(out_addr[4], weight_addr[7], bias_addr[7], out_addr[7]);
	hw_reset();

	int pt_size = 8;
	dma_loadout(out_addr[5], pt_size, rtl_addr);
	dma_loadout(out_addr[6], pt_size, (uint8_t*)rtl_addr + pt_size);
    return (void*)conv_info;
}

extern "C" PREG_LIST ptr_greg_list;
extern int test_flag;
void write_reg_info3(void* ptr_src_reg)
{
   PREG_LIST ptr_reg = (PREG_LIST)ptr_src_reg;
   FILE* fp = fopen("src_onet.txt","w");
   printf("ptr_reg->num = %d\n",ptr_reg->num);
   for(int i = 0;i<ptr_reg->num;i++)
   {
	   fprintf(fp,"addr: %8x\n",ptr_reg->addr[i]);
	   fprintf(fp,"val: %8x\n",ptr_reg->val[i]);
   }
   fclose(fp);
}

void PONet::run(void* rtl_addr, uint32_t data_size,void* config)
{
	int i = 0;
	conv_info = config;
	hw_reset();
	PREG_LIST ptr_reg = (PREG_LIST)conv_info;
	aw_ai_fast_start(conv1_info,1,1);
	hw_reset();
	aw_ai_fast_start(conv2_info,1,1);
	hw_reset();
	aw_ai_fast_start(conv3_info,1,1);
	hw_reset();
	aw_ai_fast_start(conv4_info,1,0);
	hw_reset();
	aw_ai_fast_start(conv501_info,1,0);
	hw_reset();
	aw_ai_fast_start(conv502_info,1,0);
	hw_reset();
	aw_ai_fast_start(conv51_info,1,0);
	hw_reset();
	aw_ai_fast_start(conv52_info,1,0);
	hw_reset();
	aw_ai_fast_start(conv53_info,1,0);
	hw_reset();

	int pt_size = 8;
	dma_loadout(out_addr[5], pt_size, rtl_addr);
	dma_loadout(out_addr[6], pt_size, (uint8_t*)rtl_addr + pt_size);
}

void PONet::conv1(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr)
{
  
	struct aw_ai_conv_op_desc conv_op;
	
	conv_op.first_layer = 1;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 48*3;
	conv_op.input_height = 48;
	conv_op.input_channel = 1;
     
	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3*3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 1;
	conv_op.kernel_num = 32;

	conv_op.conv_stride_x = 3;
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
	conv_op.scale = cvt_scale[0];
	conv_op.truncate = cvt_bits[0];
   
	conv_op.offset = 0;

	conv_op.output_data_addr = NULL;// out_addr
	conv_op.output_width = 46;
	conv_op.output_height = 46;
	conv_op.output_channel = 32;
	conv_op.data_format = 0;

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
	pool_op.input_addr = NULL;
	pool_op.input_width = 46;
	pool_op.input_height = 46;
	pool_op.input_channel = 32;
	pool_op.pad_num = -1;
	pool_op.pad_left = 0;
	pool_op.pad_top = 0;
	pool_op.pad_right = 1;
	pool_op.pad_bottom = 1;

	pool_op.stride_x = 2;
	pool_op.stride_y = 2;
	pool_op.pool_width = 3;
	pool_op.pool_height = 3;
	pool_op.output_width = 23;
	pool_op.output_height = 23;
	pool_op.output_channel = 32;
	pool_op.output_addr = out_addr;

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

void PONet::conv2(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 23;
	conv_op.input_height = 23;
	conv_op.input_channel = 32;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 32;
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
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scale[1];
	conv_op.truncate = cvt_bits[1];
	conv_op.offset = 0;

	conv_op.output_data_addr = NULL; 
	conv_op.output_width = 21;
	conv_op.output_height = 21;
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

	struct aw_ai_pool_op_desc pool_op;
	pool_op.pool_mode = POOL_MODE_MAX;
	pool_op.input_addr = NULL;
	pool_op.input_width = 21;
	pool_op.input_height = 21;
	pool_op.input_channel = 64;
	pool_op.pad_num = 0;
	pool_op.pad_left = 0;
	pool_op.pad_top = 0;
	pool_op.pad_right = 0;
	pool_op.pad_bottom = 0;

	pool_op.stride_x = 2;
	pool_op.stride_y = 2;
	pool_op.pool_width = 3;
	pool_op.pool_height = 3;
	pool_op.output_width = 10;
	pool_op.output_height = 10;
	pool_op.output_channel = 64;
	pool_op.output_addr = out_addr;
	
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

void PONet::conv3(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 10;
	conv_op.input_height = 10;
	conv_op.input_channel = 64;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 64;
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
	conv_op.mean_ry = 0; /* mean value for red in RGB or Y in YUV */
	conv_op.mean_gu = 0; /* mean value for green in RGB or U in YUV */
	conv_op.mean_bv = 0; /* mean value for blue in RGB or V in YUV */
	conv_op.mean_ax = 0;
	conv_op.pad_val = 0;
	
	/* output converter parameters, support truncate only */
	conv_op.cvt_enable = 1;
	conv_op.scale = cvt_scale[2];
	conv_op.truncate = cvt_bits[2];
	conv_op.offset = 0;

	conv_op.output_data_addr = NULL; 
	conv_op.output_width = 8;
	conv_op.output_height = 8;
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

	struct aw_ai_pool_op_desc pool_op;
	pool_op.pool_mode = POOL_MODE_MAX;
	pool_op.input_addr = NULL;
	pool_op.input_width = 8;
	pool_op.input_height = 8;
	pool_op.input_channel = 64;
	pool_op.pad_num = 0;
	pool_op.pad_left = 0;
	pool_op.pad_top = 0;
	pool_op.pad_right = 0;
	pool_op.pad_bottom = 0;

	pool_op.stride_x = 2;
	pool_op.stride_y = 2;
	pool_op.pool_width = 2;
	pool_op.pool_height = 2;
	pool_op.output_width = 4;
	pool_op.output_height = 4;
	pool_op.output_channel = 64;
	pool_op.output_addr = out_addr;
	
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


void PONet::conv4(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 4;
	conv_op.input_height = 4;
	conv_op.input_channel = 64;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 2;
	conv_op.kernel_height = 2;
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
	conv_op.scale = cvt_scale[3];
	conv_op.truncate = cvt_bits[3];
	conv_op.offset = 0;

	conv_op.output_data_addr = out_addr; 
	conv_op.output_width = 3;
	conv_op.output_height = 3;
	conv_op.output_channel = 128;
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


void PONet::conv5(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 3;
	conv_op.input_height = 3;
	conv_op.input_channel = 128;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 3;
	conv_op.kernel_height = 3;
	conv_op.kernel_channel = 128;
	conv_op.kernel_num = 128;//256

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
	conv_op.scale = cvt_scale[4];
	conv_op.truncate = cvt_bits[4];
	conv_op.offset = 0;

	conv_op.output_data_addr = out_addr; 
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


void PONet::conv5_1(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 256;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 256;
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
	conv_op.scale = cvt_scale[5];
	conv_op.truncate = cvt_bits[5];
	conv_op.offset = 0;

	conv_op.output_data_addr = out_addr; 
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


void PONet::conv5_2(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 256;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 256;
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
	conv_op.scale = cvt_scale[6];
	conv_op.truncate = cvt_bits[6];
	conv_op.offset = 0;

	conv_op.output_data_addr = out_addr; 
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

void PONet::conv5_3(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr)
{
	struct aw_ai_conv_op_desc conv_op;
	/* Performance parameters */
	conv_op.first_layer = 0;
	
	conv_op.input_data_addr = in_addr;
	conv_op.input_width = 1;
	conv_op.input_height = 1;
	conv_op.input_channel = 256;

	conv_op.kernel_data_addr = weight_addr;
	conv_op.kernel_width = 1;
	conv_op.kernel_height = 1;
	conv_op.kernel_channel = 256;
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
	conv_op.scale = cvt_scale[7];
	conv_op.truncate = cvt_bits[7];
	conv_op.offset = 0;

	conv_op.output_data_addr = out_addr; 
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

