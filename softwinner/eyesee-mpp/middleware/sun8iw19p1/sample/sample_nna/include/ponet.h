#ifndef PONET_H
#define PONET_H
#include <vector>
using namespace std;

class PONet
{
public:
	PONet();
	~PONet();
public:
	void loadinputdata(void* img_addr, uint32_t data_size);
	void loaddata(char* data_path);
	void loaddata2(char* weight_path, char* bias_path, char* prelu_path);
	void run(void* rtl_addr, uint32_t data_size,void* config);
	void* run_config(void* rtl_addr, uint32_t data_size);
	void run2(void* rtl_addr, uint32_t data_size);
    uint32_t ONET_IMG_DATA_IN_ADDR;
private:
	void* gp_vaddr;
	void* gp_paddr;
	uint32_t onet_WEIGHT_IN_ADDR;
	uint32_t onet_BAIS_IN_ADDR;
	uint32_t ONet_RELU_IN_ADDR;
	uint32_t onet_OUT_ADDR;

	uint32_t* reg_list;

	void* conv_info;
    void* conv1_info;
	void* conv2_info;
	void* conv3_info;
	void* conv4_info;
	void* conv501_info;
	void* conv502_info;
	void* conv51_info;
	void* conv52_info;
	void* conv53_info;

	vector<int32_t> cvt_scale;
	vector<int32_t> cvt_bits;
	vector<uint32_t> in_addr;
	vector<uint32_t> weight_addr;
	vector<uint32_t> bias_addr;
	vector<uint32_t> prelu_addr;
	vector<uint32_t> out_addr;
	void conv1(uint32_t img_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr);
	void conv2(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr);
	void conv3(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr);
	void conv4(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t out_addr);
	void conv5(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr);
	void conv5_1(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr);
	void conv5_2(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr);
	void conv5_3(uint32_t in_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t out_addr);
};
#endif
