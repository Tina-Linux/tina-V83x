#ifndef IPU_RUNTIME
#define IPU_RUNTIME

#define FORMAT_T_R8			0
#define FORMAT_T_R10			1
#define FORMAT_T_R12			2
#define FORMAT_T_R16			3
#define FORMAT_T_R16_I			4
#define FORMAT_T_R16_F			5
#define FORMAT_T_A16B16G16R16		6
#define FORMAT_T_X16B16G16R16		7
#define FORMAT_T_A16B16G16R16_F		8
#define FORMAT_T_A16Y16U16V16		9
#define FORMAT_T_V16U16Y16A16		10
#define FORMAT_T_A16Y16U16V16_F		11
#define FORMAT_T_A8B8G8R8		12
#define FORMAT_T_A8R8G8B8		13
#define FORMAT_T_B8G8R8A8		14
#define FORMAT_T_R8G8B8A8		15
#define FORMAT_T_X8B8G8R8		16
#define FORMAT_T_X8R8G8B8		17
#define FORMAT_T_B8G8R8X8		18
#define FORMAT_T_R8G8B8X8		19
#define FORMAT_T_A2B10G10R10		20
#define FORMAT_T_A2R10G10B10		21
#define FORMAT_T_B10G10R10A2		22
#define FORMAT_T_R10G10B10A2		23
#define FORMAT_T_A2Y10U10V10		24
#define FORMAT_T_V10U10Y10A2		25
#define FORMAT_T_A8Y8U8V8			26
#define FORMAT_T_V8U8Y8A8			27
#define FORMAT_T_Y8___U8V8_N444		28
#define FORMAT_T_Y8___V8U8_N444		29
#define FORMAT_T_Y10___U10V10_N444	30
#define FORMAT_T_Y10___V10U10_N444	31
#define FORMAT_T_Y12___U12V12_N444	32
#define FORMAT_T_Y12___V12U12_N444	33
#define FORMAT_T_Y16___U16V16_N444	34
#define FORMAT_T_Y16___V16U16_N444	35
#define FORMAT_FEATURE			36

#include <ipu_types.h>
struct aw_ai_conv_op_desc {
	/* Performance parameters */
	uint8_t first_layer;
	uint8_t data_format;
	/* The input cube dimension for CSC */
	uint32_t input_data_addr;
	uint16_t input_width;
	uint16_t input_height;
	uint16_t input_channel;

	uint32_t kernel_data_addr;
	uint16_t kernel_width;
	uint16_t kernel_height;
	uint16_t kernel_channel;
	uint16_t kernel_num;

	uint8_t bias_mode;//0->disable,1->layer,2->pre channel,3->pre elem
	uint32_t bias_data_addr;

	uint8_t conv_stride_x;
	uint8_t conv_stride_y;
	uint8_t pad_x_left;
	uint8_t pad_x_right;
	uint8_t pad_y_top;
	uint8_t pad_y_bottom;
	uint8_t dilation_x;
	uint8_t dilation_y;
	uint8_t reserved2[2];

	uint8_t mean_enable;
	int16_t mean_ry; /* mean value for red in RGB or Y in YUV */
	int16_t mean_gu; /* mean value for green in RGB or U in YUV */
	int16_t mean_bv; /* mean value for blue in RGB or V in YUV */
	int16_t mean_ax;
	int16_t pad_val;
	/*act*/
	uint8_t act_enbale; //0->disable,1->relu,2->prelu
	uint32_t act_data_addr;

	/*bn*/
	uint8_t bn_enbale; //0->disable,1->enable,BN和prelu不能同时用
	uint32_t bn_data_addr;

	/*eltwise*/
	uint8_t eltwise_enbale; //0->disable,1->enable,BN和prelu不能同时用
	uint32_t eltwise_data_addr;
	/* output converter parameters, support truncate only */
	uint8_t  cvt_enable;
	int16_t  scale;
	uint8_t  truncate;
	int32_t  offset;

	uint32_t output_data_addr;
	uint16_t output_width;
	uint16_t output_height;
	uint16_t output_channel;

	uint32_t reverse1;
	uint32_t reverse2;
};

struct aw_ai_element_wise_op_desc {
	/* Performance parameters */
	uint8_t data_format;
	uint8_t operation_type; //0-mul,1-add,2-mul+add

	uint32_t input1_data_addr;
	uint16_t input1_width;
	uint16_t input1_height;
	uint16_t input1_channel;

	uint32_t input2_data_addr;
	uint16_t input2_width;
	uint16_t input2_height;
	uint16_t input2_channel;

	uint32_t input3_data_addr;
	uint16_t input3_width;
	uint16_t input3_height;
	uint16_t input3_channel;

	int32_t add_operand;
	int32_t mul_operand;

	uint8_t  cvt_enable;//默认输入输出都是INT8，所以输出要转换量化。offset -> scale -> truncate(右移截断)
	int16_t  scale;
	uint8_t  truncate;
	int32_t  offset;

	uint32_t output_data_addr;
	uint16_t output_width;
	uint16_t output_height;
	uint16_t output_channel;
};

#define POOL_MODE_AVG		0
#define POOL_MODE_MAX		1
#define POOL_MODE_MIN		2

struct aw_ai_pool_op_desc {
	uint32_t input_addr;
	uint16_t input_width;
	uint16_t input_height;
	uint16_t input_channel;

	uint32_t output_addr;
	uint16_t output_width;
	uint16_t output_height;
	uint16_t output_channel;

	/* Algorithm parameters */
	uint8_t  pool_mode; /* ipu_pool_mode */
	uint8_t  pool_width; /* ipu_pool_width */
	uint8_t  pool_height; /* ipu_pool_height */

	uint8_t  stride_x;
	uint8_t  stride_y;

	uint8_t  pad_num;
	uint8_t  pad_left;
	uint8_t  pad_right;
	uint8_t  pad_top;
	uint8_t  pad_bottom;

	uint32_t reverse1;
	uint32_t reverse2;
};

struct ipu_ops_params
{
	unsigned int dev_type;
	unsigned int op_type;
	unsigned int blob_id;
	unsigned int cvt_scale;
	unsigned int bits;
	//===============================================
	int conv_input_shape_n;
	int conv_input_shape_h;
	int conv_input_shape_w;
	int conv_input_shape_c;
	int conv_kernel_shape_n;
	int conv_kernel_shape_h;
	int conv_kernel_shape_w;
	int conv_kernel_shape_c;
	int conv_stride_x;
	int conv_stride_y;
	int conv_pad;
	int conv_dilation;
	int conv_bn_enbale;
	int conv_bias_mode;
	int conv_act_mode;
	int conv_eltwise_enbale;
	int conv_output_shape_n;
	int conv_output_shape_h;
	int conv_output_shape_w;
	int conv_output_shape_c;
	int pool_input_shape_n;
	int pool_input_shape_h;
	int pool_input_shape_w;
	int pool_input_shape_c;
	int pool_kernel;
	int pool_pad;
	int pool_stride;
	int pool_output_shape_n;
	int pool_output_shape_h;
	int pool_output_shape_w;
	int pool_output_shape_c;
	//===============================================
	int in_addr;
	int weight_addr;
	int bias_addr;
	int prelu_addr;
	int out_addr;

	unsigned int weight_len;
	unsigned int bias_len;
	unsigned int prelu_len;
	void* ptr_conv_weight;
	void* ptr_conv_bias;
	void* ptr_prelu_weight;
};

struct ipu_ops
{
	int ipu_ops_num;
	struct ipu_ops_params* ptr_ops[100];
};


#ifdef __cplusplus 
extern "C" {
#endif
	extern int aw_ai_program_init();
    extern int aw_ai_program_deinit();
	extern int aw_ai_conv_program(struct aw_ai_conv_op_desc conv_op_desc);
	extern int aw_ai_pool_program(struct aw_ai_pool_op_desc pool_op_desc);
	extern int aw_ai_conv_pool_program(struct aw_ai_conv_op_desc conv_op_desc,struct aw_ai_pool_op_desc pool_op_desc);
	extern int aw_ai_conv_2_program(struct aw_ai_conv_op_desc conv_op_desc);
	extern int aw_ai_element_wise_program(struct aw_ai_element_wise_op_desc element_op_desc);
	extern int aw_ai_op_completion(unsigned int idx);

	extern void* aw_ai_read_info(void* ptr_info,uint8_t conv_sdp_flag,uint8_t pdp_flag);
	extern int aw_ai_fast_start2(void* ptr_info);
	extern int aw_ai_fast_start(void* ptr_info,int conv_sdp_enable,int pdp_enable);
	extern int aw_ai_free_info(void* ptr_info);

	extern int aw_ai_cvt_data_hwc2ipu(void* ptr_src, int height, int width, int channels, void* ptr_dst);
	extern int aw_ai_cvt_data_ipu2hwc(void* ptr_src, int height, int width, int channels, void* ptr_dst);
	extern void run_ops(struct ipu_ops_params* p_ipu_ops, int first_layer);

#ifdef __cplusplus 
}
#endif
#endif