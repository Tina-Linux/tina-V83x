// Qualcomm_NCS_NES_Library_V3.5.7.5_20150819.cpp : 定义控制台应用程序的入口点。
//

#include "stdlib.h"
#include "stdio.h"
#include "math.h" 
#include <sys/time.h>
#include "NCS_NES_DEMO_Library.h"


#define SAMPLE_NUMBER 320//320

void main()
{
    struct timeval tm0 ,tm1;
	int i;
	FILE *stream_UL_Input, *stream_DL_Input, *stream_UL_Output;

	NCSNES_ConfigPara_T ptNCSNESConfigPara;

	ptNCSNESConfigPara.NCS_Enable = 0;
	ptNCSNESConfigPara.FixedDelay = 1;
	ptNCSNESConfigPara.AttenuationDB = -20;
	ptNCSNESConfigPara.Tilt = 5;
	ptNCSNESConfigPara.EchoTail = 100;
	ptNCSNESConfigPara.NLPLevel = 1;
	ptNCSNESConfigPara.OutputNoiseLevel = 1;
	ptNCSNESConfigPara.InputFixedGain = 2;
	ptNCSNESConfigPara.PreAGC_Gain = 4;
	ptNCSNESConfigPara.PostAGC_Gain = 2;
	ptNCSNESConfigPara.Mode = 1;
	ptNCSNESConfigPara.FidelityLevel = 2;


	ptNCSNESConfigPara.AES_Enable = 1;
	ptNCSNESConfigPara.AES_Filter_L = 240;//400
	ptNCSNESConfigPara.AES_M_factor = 0.09*32768;
	ptNCSNESConfigPara.AES_i_Delay = 32;
	ptNCSNESConfigPara.AES_Tilt = -0.85*32768;
	ptNCSNESConfigPara.AES_FixedDelaySample = 0;
	ptNCSNESConfigPara.AES_LevelNLP = 0.7*32768;
	
	ptNCSNESConfigPara.ul_in_fixed_gain = 0;
	ptNCSNESConfigPara.dl_in_fixed_gain = 0;
	ptNCSNESConfigPara.ul_out_fixed_gain = 0;
	ptNCSNESConfigPara.Test_mode = 0;
	ptNCSNESConfigPara.Mips_test = 0;


	int numclosed; 

	short BT[SAMPLE_NUMBER];
	short CT[SAMPLE_NUMBER];
	short DT[SAMPLE_NUMBER];
	short output_buf[SAMPLE_NUMBER];

	short r_buf[SAMPLE_NUMBER*2];

	int numread = 1;
	int frame_num=0;
	
	
	if((stream_UL_Input = fopen("UL_In_16k.pcm", "rb")) == NULL)
	{
		printf("The input file UL_Input was not opened\n");
		return;
	}
	if((stream_DL_Input = fopen("DL_In_16k.pcm", "rb")) == NULL)
	{
		printf("The input file DL_Input was not opened\n");
		close(stream_UL_Input);
		return;
	}
	if((stream_UL_Output = fopen("uplink_out_16k_test.pcm", "wb")) == NULL)
	{
		printf("The UL output file was not opened\n");
		close(stream_UL_Input);
		close(stream_DL_Input);
		return;
	}
	
	NCS_NES_Algorithm_Init(&ptNCSNESConfigPara, 16000);
	gettimeofday(&tm0,NULL);
	while(numread > 0)
	{
		numread = fread(BT, 2, SAMPLE_NUMBER, stream_UL_Input);
		numread = fread(DT, 2, SAMPLE_NUMBER, stream_DL_Input);
		
		NCS_NES_Algorithm_Proc(BT, DT, CT, &ptNCSNESConfigPara, SAMPLE_NUMBER, 16000);
		frame_num++;
		fwrite(CT, 2, SAMPLE_NUMBER, stream_UL_Output);
		
	}
    gettimeofday(&tm1,NULL);
    printf("spend time = %ld %ld\n",(tm1.tv_sec-tm0.tv_sec)*1000, (tm1.tv_usec-tm0.tv_usec));
    fclose(stream_UL_Output);
    fclose(stream_DL_Input);
    fclose(stream_UL_Input);
	//numclosed = _fcloseall();
	//printf( "Number of files closed by _fcloseall: %u,frame_num=%d\n" , numclosed,frame_num);
	//getchar();

	//return 0;
}

