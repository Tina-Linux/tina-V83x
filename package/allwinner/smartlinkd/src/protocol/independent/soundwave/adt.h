#ifndef _ADT_H_
#define _ADT_H_


/* frequency type selector */
typedef enum
{
	/*低频， 2K~5K */
	FREQ_TYPE_LOW =0,
	/*中频, 8K~12K*/
	FREQ_TYPE_MIDDLE,
	/*高频， 16K~20K*/
	FREQ_TYPE_HIGH

}freq_type_t;

/*macros of return valule of decoder*/
/* 解码出错 */
#define RET_DEC_ERROR -1
/* 解码正常返回 */
#define RET_DEC_NORMAL 0
/* 解码还未结束，不能获取到解码结果 */
#define RET_DEC_NOTREADY 1
/* 解码结束 */
#define RET_DEC_END 2


/* definition of decoder config paramters */
typedef struct
{
	/* 支持的最大字符串长度（字节数） */
	int max_strlen;
	/* 采样率 */
	int sample_rate;
	/* 频率范围选择 */
	freq_type_t freq_type;
	/* 每个分组传输的字节数 */
	int group_symbol_num;
	/* 是否采用纠错码 */
	int error_correct;
	/* 纠错码的纠错能力 */
	int error_correct_num;
}config_decoder_t;

/*
	描述：	创建解码器
	参数：	decode_config: 参数结构体（指针）
	返回值：	解码器句柄，NULL表示创建失败
*/
void* decoder_create(config_decoder_t* decode_config );

/*
	描述：	复位解码器
	参数：	handle：		解码器句柄
	返回值：	无

*/
void decoder_reset(void* handle);
/*
	描述：	获取每帧数据量（样本数）
	参数：	handle：		解码器句柄
	返回值：	每帧数据量（样本数，每个样本为16bit）

*/
int decoder_getbsize(void* handle);
/*
	描述：	向解码器填充数据
	参数：	handle：		解码器句柄
			pcm：		数据buffer，需保证含有的样本点数等于decoder_getbsize的返回值
	返回值：	同解码返回值的宏定义
*/
int decoder_fedpcm(void* handle, short* pcm);
/*
	描述：	获取解码结果
	参数：	handle：	解码器句柄
			str：	解码结果buffer
	返回值：	同解码返回值的宏定义
*/
int decoder_getstr(void* handle, unsigned char* str);
/*
	描述：释放解码器句柄
	参数：handle：	解码器句柄
	返回值：无
*/
void decoder_destroy(void* handle);

/* definition of encoder config paramters */
typedef struct
{
	/* 支持的最大字符串长度（字节数） */
	int max_strlen;
	/* 采样频率 */
	int sample_rate;
	/* 频率范围选择 */
	freq_type_t freq_type;
	/* 每个分组传输的字节数 */
	int group_symbol_num;
	/* 是否采用纠错编码 */
	int error_correct;
	/* 纠错编码的纠错能力（字节数）*/
	int error_correct_num;
}config_encoder_t;

/* macros of return value of encoder */
/* 编码正常返回 */
#define		RET_ENC_NORMAL 0
/* 编码结束 */
#define		RET_ENC_END 1
/* 编码出错 */
#define		RET_ENC_ERROR -1

/*
	描述：	释放编码器句柄
	参数：	handle：		编码器句柄
	返回值：	无
*/
void encoder_destroy(void* handle);
/*
	描述：	创建编码器句柄
	参数：	config：		参数结构体（指针）
	返回值：	编码器句柄，为NULL表示创建失败
*/
void* encoder_create(config_encoder_t* config);
/*
	描述：	复位编码器
	参数：	handle：		编码器句柄
	返回值：	无

*/
void encoder_reset(void* handle);
/*
	描述：	获取帧缓存大小（字节单位），用于外部分配帧缓存buffer
	参数：	handle：		编码器句柄
	返回值：	帧缓存大小（字节单位）

*/
int encoder_getoutsize(void* handle);
/*
	描述：	获取帧数据
	参数：	handle：		编码器句柄
			outpcm：		帧数据buffer（外部分配）
	返回值：	同编码器返回值的宏定义
*/
int encoder_getpcm(void* handle, short* outpcm);
/*
	描述：	设置待编码的字符串，字符串需以空字符'\0'结尾
	参数：	handle：		编码器句柄
			input：		以空字符结尾的字符串
			返回值		同编码器返回值的宏定义
*/
int encoder_setinput(void* handle, unsigned char* input);

#endif
