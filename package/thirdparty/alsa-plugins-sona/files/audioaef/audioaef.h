#ifndef _AUDIOAEF_H
#define _AUDIOAEF_H

#ifdef WIN32
#define DLL_API _declspec(dllexport)
#else
#define DLL_API
#endif
/*
定义-1
typedef enum _Biquad_Filter_Type_
{
	BiquadFilter_AP1 = 0,
	BiquadFilter_LP1 ,
	BiquadFilter_HP1 ,
	BiquadFilter_LS1 ,
	BiquadFilter_HS1 ,
	BiquadFilter_LP2 ,
	BiquadFilter_HP2 ,
	BiquadFilter_LS2 ,
	BiquadFilter_HS2 ,
	BiquadFilter_PEAK ,
	BiquadFilter_NOTCH,
	BiquadFilter_AP2 ,
	BiquadFilter_BP2,
	BiquadFilter_BP2_0GAIN
} eBiquadFilterType;
*/
/*定义-2
typedef enum _XOVER_SLOPE
{
	XOVER_SLOPE_6 = 0,
	XOVER_SLOPE_12,
	XOVER_SLOPE_18,
	XOVER_SLOPE_24,
	XOVER_SLOPE_30,
	XOVER_SLOPE_36,
	XOVER_SLOPE_48
}eXoverSlope;
*/

typedef int (*savefileCb)(char*filename,void*private_data);

#ifdef __cplusplus
extern "C" {
#endif
	//fs,算法系统采样率
	//conf_file, 音效配置文件；
	// outbits，process输出数据每个样点bit数(有符号数)，支持16，24； 与输入数据的bit数无关
	//maxFrameLen,允许的最大输入帧长
	DLL_API void* audioaef_create(int infs, int outfs, char*conf_file, int outBits, int maxFrameLen);
	//同上,配置从字符串buffer中读取不从文件读取
	DLL_API void* audioaef_create_fromBuffer(int infs, int outfs, char*buffer, int outBits, int maxInFrameLen);
	DLL_API int audioaef_refreshConfig_fromBuffer(void*ins,char*buffer);
	//关闭音效处理并且释放资源
	DLL_API int audioaef_destroy(void*ins);
	//修改采样率
	DLL_API int audioaef_set_fs(void*ins,int infs, int outfs);
	//设置接收网络保存配置的回调函数，如无则不相应网络保存命令
	DLL_API int audioaef_set_saveFileCb(void*ins,savefileCb func,void*private_data);

	//相应网络保存配置指令，file被忽略
	DLL_API int aduioaef_net_saveconfig(void*ins,char*file);
	//相应网络保存配置指令，file被忽略，size为buffer内存大小,成功返回写入buffer大小（含字符串结束符）
	DLL_API int audioaef_save_config2Buffer(void*ins,char*buffer, int size);
	//根据输入帧长计算最大输出帧长；
	//如果输入输出采样率不同，则process每次返回结果会不固定长度，此接口返回最大长度，便于buffer和播放管理
	DLL_API int audioaef_getMaxOutFrame(void*ins,int inFrameLen);
	//对输入语音做流程中的算法处理，数据格式（左右通道交错形式）
	//outdata输出数据，长度可根据输入参数计算所得，也可以从函数返回值获取；
	//channels : 输入数据通道数 1 or 2
	//bitspersample: 16 or 24,24bit格式每个sample占据32bit内存空间
	//outChans：输出同道数
	//return ： outdata 数据字节长度
	DLL_API int audioaef_process(void*ins,char* indata, int lenOfBytes, int channels,int bitspersample, char*outdata,int outChans);

	//设置第二个输出音频流采样率，比如输出16K采样的数据用于aec参考信号
	//如果需要，紧接着audioaef_init之后调用
	//return 输出数据每帧最大采样点数(单声道)
	DLL_API int audioaef_create_secondOutStream(void*ins,int fs);
	//设置第二个输出音频使能开关
	DLL_API int audioaef_enable_secondOutStream(void*ins,int enble);
	//返回第二个音频流，必须在audioaef_process之后紧接着调用；
	//return 输出数据字节数
	DLL_API int audioaef_get_secondOutStream(void*ins,char*outdata);

	DLL_API int audioaef_save_config(void*ins,char*file);

	//net-server init for audio config api
	//aefIns 网络配置对象为aefIns的音效实例
	DLL_API void* audioRPC_init(void*aefIns); //return RPC handle
	DLL_API void audioRPC_close(void* handle);

	//如下为音效参数设置相关API接口,接口中有相关参数运算，故参数如下接口的调用与process接口使用是不同的线程处理，便于音频数据的流程性
	//设置虚拟低音整体模块功能使能
	DLL_API int audioaef_virtbass_enable(void*ins,int enable);
	DLL_API int audioaef_get_virtbass_enable(void*ins);
	//设置和获取deq全局使能
	DLL_API int audioaef_deq_enable(void*ins,int enable);
	DLL_API int audioaef_get_deq_enable(void*ins);
	//设置和获取DRC全局使能
	DLL_API int audioaef_drc_enable(void*ins,int enable);
	DLL_API int audioaef_get_drc_enable(void*ins);

	//获取level分贝值
	DLL_API float audioaef_get_level(void*ins, char*name);
	// 设置 level enable
	DLL_API int audioaef_set_level_enable(void*ins, char*name, int enable);
	//设置level attack time
	DLL_API int audioaef_set_level_att(void*ins, char*name, float second);
	//设置level release time
	DLL_API int audioaef_set_level_rea(void*ins, char*name, float second);
	//level模块整体参数配置
	DLL_API int audioaef_set_level(void*ins, char*name, int enable, float att_time, float rea_time);
	//level模块整体参数读取
	DLL_API int audioaef_get_levelConfig(void*ins, char*name, int *enable, float *att_time, float *rea_time);

	// compressor参数设置
	DLL_API int audioaef_set_compressor_enable(void*ins, char*name, int enable);
	DLL_API int audioaef_set_compressor_att(void*ins, char*name, float second);
	DLL_API int audioaef_set_compressor_rea(void*ins, char*name, float second);
	DLL_API int audioaef_set_compressor_threashold(void*ins, char*name, float dB);
	DLL_API int audioaef_set_compressor_ratio(void*ins, char*name, float ratio);
	//整体参数配置
	DLL_API int audioaef_set_compressor(void*ins, char*name,int enable, float att_time, float rea_time, float threashold, float ratio);
	//整体参数读取
	DLL_API int audioaef_get_compressor(void*ins, char*name,int *enable, float *att_time, float *rea_time, float *threashold, float *ratio);

	// 设置 limiter enable
	DLL_API int audioaef_set_limiter_enable(void*ins, char*name, int enable);
	DLL_API int audioaef_set_limiter_threshold(void*ins, char*name, float threshold);	//设置门限值
	DLL_API int audioaef_set_limiter_att(void*ins, char*name, float second);
	DLL_API int audioaef_set_limiter_rea(void*ins, char*name, float second);
	//整体参数配置
	DLL_API int audio_set_limiter(void*ins, char*name, int enable, float att_time, float rea_time, float threashold);
	//整体参数读取
	DLL_API int audio_get_limiter(void*ins, char*name, int *enable, float *att_time, float *rea_time, float *threashold);

	//设置增益
	DLL_API int audioaef_set_gain(void*ins, char*name, float dB);
	DLL_API int audioaef_set_gain_mute(void*ins, char*name, int mute);
	////整体参数配置
	DLL_API int audioaef_set_gainConfig(void*ins, char*name, float gaindB, int mute);
	//整体参数读取
	DLL_API int audioaef_get_gainConfig(void*ins, char*name, float *gaindB, int *mute);

	//设置xover
	DLL_API int audioaef_set_xover_enable(void*ins, char*name, int enable);
	//0-LP, 1-HP
	DLL_API int audioaef_set_xover_type(void*ins, char*name, int type);
	DLL_API int audioaef_set_xover_freq(void*ins, char*name, float fc);
	//设置滤波器类型 0 -butterwoth,1 - bessel ；2- Linkwitz-Reily
	DLL_API int audioaef_set_xover_func(void*ins, char*name, int func);
	//设置滤波器斜率，取值如下
	//slope 定义参考定义-2
	DLL_API int audioaef_set_xover_slope(void*ins, char*name, int slope);
	//整体参数配置
	DLL_API int audioaef_set_xover(void*ins, char*name, int enable, int type, int func, float fc, int slope);
	//整体参数读取
	DLL_API int audioaef_get_xover(void*ins, char*name, int *enable, int *type, int *func, float *fc, int *slope);


	/*PEQ 滤波器组设置*/
	DLL_API int audioaef_set_peq_enable(void*ins, char*name, int enable);
	//获取peq使能参数
	DLL_API int audioaef_get_peq_enable(void*ins, char*name);
	//滤波器BQF使能 bqf-N段BQF序号： 0-5
	DLL_API int audioaef_set_peq_bqf_enable(void*ins, char*name, int bqfID, int enable);
	//设置滤波器BQF类型，定义如前面注释中的“定义-1”
	DLL_API int audioaef_set_peq_bqf_type(void*ins, char*name, int bqfID, int type);
	DLL_API int audioaef_set_peq_bqf_gain(void*ins, char*name, int bqfID, float gain);
	DLL_API int audioaef_set_peq_bqf_q(void*ins, char*name, int bqfID, float q);
	DLL_API int audioaef_set_peq_bqf_freq(void*ins, char*name, int bqfID, float freq);
	//某一段BQF整体参数配置
	DLL_API int audioaef_set_peq_bqf(void*ins, char*name, int bqfID, int enable, int type, float gain, float q, float freq);
	//某一段BQF整体参数读取
	DLL_API int audioaef_get_peq_bqf(void*ins, char*name, int bqfID, int *enable, int *type, float *gain, float *q, float *freq);

//扬声器非线性补偿器
	//参数type：
	//	0 - bypass；
	//  1 - gene；
	//	2 - hivi；
	//	3 - idea
	//  4 - idea601
	//  5-  idea1025
	DLL_API int audioaef_set_soundNonLinear(void*ins, int type);
	//获取扬声器补偿模式
	DLL_API int audioaef_get_soundNonLinear(void*ins);

	DLL_API char* audioaef_get_vendor(void*ins);

#ifdef __cplusplus
}
#endif

#endif
