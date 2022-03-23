

#ifndef NCS_NES_DEMO_Library_H
#define NCS_NES_DEMO_Library_H

typedef char QOS_INT8;
typedef unsigned char QOS_UINT8;
typedef short int QOS_INT16;
typedef unsigned short int QOS_UINT16;
typedef int QOS_INT32;
typedef unsigned int QOS_UINT32;

#define QOSOUND_NCS_NES_ALGO
#ifdef QOSOUND_NCS_NES_ALGO

#define NCS_NES_TRUE        1
#define NCS_NES_FALSE       0


typedef enum
{
    NCS_NES_SUCCESS = 0xFF,
    NCS_NES_INVALID_ENABLE = 0,
    NCS_NES_INVALID_FIXED_DELAY = 1,
    NCS_NES_INVALID_A_DB = 2,
    NCS_NES_INVALID_TILT = 3,
    NCS_NES_INVALID_ECHO_TAIL = 4,
    NCS_NES_INVALID_NLP_LEVEL = 5,
    NCS_NES_INVALID_NOISE_LEVEL = 6,
    NCS_NES_INVALID_INPUT_FIXED_GAIN = 7,
    NCS_NES_INVALID_PRE_AGC_GAIN = 8,
    NCS_NES_INVALID_POST_AGC_GAIN = 9,
    NCS_NES_INVALID_MODE = 10,
} NCS_NES_CONFIG_PARA_ERR_CODE_E;


#define NCS_NES_ALGO_DISABLE         0
#define NCS_NES_ALGO_ENABLE_NORMAL   1
#define NCS_NES_ALGO_ENABLE_BEEP     2

#define NCS_NES_FIXED_DELAY_MIN   0
#define NCS_NES_FIXED_DELAY_MAX   300

#define NCS_NES_A_DB_MIN   -30
#define NCS_NES_A_DB_MAX   5

#define NCS_NES_TILT_MIN   -9
#define NCS_NES_TILT_MAX   9

#define NCS_NES_ECHO_TAIL_MIN   0
#define NCS_NES_ECHO_TAIL_MAX   500

#define NCS_NES_NLP_LEVEL_MIN   1
#define NCS_NES_NLP_LEVEL_MAX   5

#define NCS_NES_NOISE_LEVEL_MIN   1
#define NCS_NES_NOISE_LEVEL_MAX   8

#define NCS_NES_INPUT_FIXED_GAIN_MIN   0
#define NCS_NES_INPUT_FIXED_GAIN_MAX   6

#define NCS_NES_PRE_AGC_GAIN_MIN   0
#define NCS_NES_PRE_AGC_GAIN_MAX   20

#define NCS_NES_POST_AGC_GAIN_MIN   0
#define NCS_NES_POST_AGC_GAIN_MAX   10

#define MODE_HANDSET             0
#define MODE_SPEAKERPHONE        1
#define MODE_PLUG_IN             2
#define MODE_BT_EARPHONE         3
#define MODE_BT_SPEAKER          4



extern QOS_INT16 NCS_NES_UL_In[320];
extern QOS_INT16 NCS_NES_DL_In[320];
extern QOS_INT16 NCS_NES_UL_Out[320];

typedef struct
{
    int NCS_Enable; // 0: Disable;  1: Enable NormalMode;
    int FixedDelay; // Fixed delay in sample(0~300 samples)
    int AttenuationDB; // Attenuation in dB (-30 ~ 5 dB)
    int Tilt; // Tilt (-9<<15 ~ 9<<15) Q15
    int EchoTail; // Echo tail (<500ms)
    int NLPLevel; // NLP level: 1=weakest, 5=stronges
    int OutputNoiseLevel; // Output noise level: 1=Lowest, 8=Highest
    int InputFixedGain; // Input fixed-gain
    int PreAGC_Gain; // Expected Pre-AGC gain in dB (0--0dB, 1--1dB,..., 19--19dB,20--20dB)
    int PostAGC_Gain; // Expected AGC gain in dB (0--0dB, 1--1dB,..., 9--9dB,10--10dB)
    int Mode; // 0-5: 0-handset, 1-speakerphone, (2-plug-in, 3-BT-earphone, 4-BT-speaker,no support)
    int FidelityLevel;
    
	int AES_Enable;
	int AES_Filter_L;
	int AES_M_factor;	 //Q15,such as 0.005*32768
	int AES_i_Delay;
	int AES_Tilt;	//Q15,such as -0.85*32768
	int AES_FixedDelaySample;
	int AES_LevelNLP;	 //Q15, such as 0.1*32768
	int AES_UL_InputGain;
	int AES_DL_InputGain;
	int AES_UL_OutputGain;
	
	int ul_in_fixed_gain;
	int dl_in_fixed_gain;
	int ul_out_fixed_gain;
	
	int Test_mode;// 0---no mix beep data, 1---mix beep data
	int Mips_test;
}NCSNES_ConfigPara_T;



QOS_UINT32 NCS_NES_ConfigParaCheck(NCSNES_ConfigPara_T *ptNCSNESConfigPara);

void NCS_NES_Algorithm_Init(NCSNES_ConfigPara_T *ptNCSNESConfigPara, int SampleRate);  //SampleRate:  8000/16000/48000 etc.

void NCS_NES_Algorithm_Proc(short *uplink_in,
                            short *downlink_in,
                            short *uplink_out,
                            NCSNES_ConfigPara_T *ptNCSNESConfigPara,
                            int sample_n,  //sample_n:  80@10ms;  160@20ms   for 8khz sample rate
							int SampleRate);  //SampleRate:  8000/16000/48000 etc.

#endif
#endif




