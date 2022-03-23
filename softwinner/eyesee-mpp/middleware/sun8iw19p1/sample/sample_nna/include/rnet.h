#ifndef RNET_H
#define RNET_H
#include <vector>

using namespace std;

class RNet {

public:
	RNet();
	~RNet();

public:
	void load_input_data(void* img_addr, uint32_t data_size);
	void load_data(char* data_path);
	void run(void* rtl_addr, uint32_t data_size);

	uint32_t PNET_IMG_DATA_IN_ADDR;
private:
	void* gp_vaddr;
	void* gp_paddr;

	uint32_t PNET_WEIGHT_IN_ADDR;
	uint32_t PNET_BIAS_IN_ADDR;
	uint32_t PNET_RELU_IN_ADDR;
	uint32_t PNET_OUTPUT_ADDR;


	vector<uint32_t> cvt_scales;
	vector<uint32_t> cvt_bits;
	vector<uint32_t> input_addrs;
	vector<uint32_t> weight_addrs;
	vector<uint32_t> bias_addrs;
	vector<uint32_t> prelu_addrs;
	vector<uint32_t> output_addrs;

	void conv1(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr);
	void conv2(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr);
	void conv3(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr);
	void conv4(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t prelu_addr, uint32_t output_addr);
	void conv4_1(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr);
	void conv4_2(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr);
	void conv4_3(uint32_t input_addr, uint32_t weight_addr, uint32_t bias_addr, uint32_t output_addr);
};
#endif