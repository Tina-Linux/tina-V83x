/*
 * tutuClear.h
 * yanchen.lu@gmems.com
 *
 */
#ifndef _TUTUCLEAR_H_
#define _TUTUCLEAR_H_

#include "tutu_typedef.h"

// Set below to TUTUClearConfig_t.Version for version control
#define TUTUCLEAR_VERSION   MAKETUTUVER(1, 7, 1)

typedef struct {
    UW32	uw32Version; // assign to TUTUCLEAR_VERSION
    UW16	uw16SamplingFreq; // 0/1/2/3/4: 8000/16000/24000/32000/48000 Hz
    UW16	uw16FrameSz; // set it as 10 [ms]
    UW16	uw16MaxNumOfMic;
    UW16	uw16MaxTailLength;
} TUTUClearConfig_t;

// Enumeration for uw32OpMode argument of TUTUClearParam_t
typedef enum {
    TUTUClearOpMode_RESRV00			= 0x00000001,
	TUTUClearOpMode_MST				= 0x00000002,
	TUTUClearOpMode_REC				= 0x00000004,
	TUTUClearOpMode_RESRV03			= 0x00000008,
	TUTUClearOpMode_ASR				= 0x00000010,
	TUTUClearOpMode_RESRV05			= 0x00000020,
	TUTUClearOpMode_INTERVIEW		= 0x00000040,
	TUTUClearOpMode_RESRV07			= 0x00000080,
	TUTUClearOpMode_RESRV08		    = 0x00000100,
	TUTUClearOpMode_RESRV09		    = 0x00000200,
	TUTUClearOpMode_RESRV10		    = 0x00000400,
	TUTUClearOpMode_RESRV11			= 0x00000800,
	TUTUClearOpMode_RESRV12			= 0x00001000,
	TUTUClearOpMode_ASR_VOIP		= 0x00002000,
	TUTUClearOpMode_ESP				= 0x00004000,
	TUTUClearOpMode_RESRV15			= 0x00008000,
} TUTUClearOpMode_e;

// Enumeration for uw32FuncMode argument of TUTUClearParam_t
typedef enum {
    TUTUClearFuncMode_MicBypass		= 0x00000001,
	TUTUClearFuncMode_Resrv01		= 0x00000002,
	TUTUClearFuncMode_Resrv02		= 0x00000004,
	TUTUClearFuncMode_Resrv03		= 0x00000008,
    TUTUClearFuncMode_Resrv04		= 0x00000010,
	TUTUClearFuncMode_Resrv05		= 0x00000020,
    TUTUClearFuncMode_AEC			= 0x00000040,
    TUTUClearFuncMode_adaSFS		= 0x00000080,
	TUTUClearFuncMode_NS			= 0x00000100,
	TUTUClearFuncMode_EQ			= 0x00000200,
	TUTUClearFuncMode_AGC			= 0x00000400,
    TUTUClearFuncMode_DRC			= 0x00000800,
    TUTUClearFuncMode_HPF			= 0x00001000,
	TUTUClearFuncMode_SC			= 0x00002000,
	TUTUClearFuncMode_DOA			= 0x00004000,
	TUTUClearFuncMode_SPKRID		= 0x00008000,
	TUTUClearFuncMode_MBDRC			= 0x00010000,
	TUTUClearFuncMode_AFC			= 0x00020000,
	TUTUClearFuncMode_Resrv18		= 0x00040000,
	TUTUClearFuncMode_SEQ			= 0x00080000,
	TUTUClearFuncMode_VTRGR			= 0x00100000
} TUTUClearFuncMode_e;

typedef struct {
    UW16	auw16ParamAEC[16];
} TUTUAECParam_t;

typedef struct {
	UW16	uw16Lambda;
	UW16	uw16SupressionLevel;
	UW16	uw16BGSimplicity;
	UW16	uw16D;
	UW16	auw16Resrv[28];
} TUTUNSParam_t;

typedef struct {
	UW16	uw16EQPartitionBegin; // in Hz
	UW16	uw16Resrv0;
	UW16	auw16EQPartitionWidth[16]; // in Hz
	W16		aw16EQGain[16]; // in dB
} TUTUEQParam_t;

typedef struct {
	W16		aw16ExpanderInputLevels[2]; // in dB
	UW16	uw16NumCompressorKnees; // [2;3]
	UW16	uw16Resrv0;
	W16		aw16CompressorKneeInputLevels[4]; // in dB
	W16		aw16CompressorKneeOutputLevels[4]; // in dB
	UW16	uw16AttackTime; // in ms
	UW16	uw16ReleaseTime; // in ms
	UW16	uw16MakeupGain; // linear gain
	UW16	uw16Resrv1;
	UW16	auw16Resrv[2];
} TUTUDRCParam_t;

typedef struct {
	W16		w16TargetOutputLevel; // in dB
	UW16	uw16MaxGain; // Maximal AGC gain
	UW16	uw16MinGain; // Minimal AGC gain
	UW16	uw16RespRate; // AGC adjust rate
} TUTUAGCParam_t;

typedef struct {
	UW16	uw16T3T4DtctThrd;
	UW16	uw16T4SilnDtctThrd;
	UW16	uw16C;
	UW16	uw16D;
	UW16	auw16Resrv[12];
} TUTUSCParam_t;

typedef struct {
	W16		aw16MicPosX[8];
	W16		aw16MicPosY[8];
	UW16	auw16MicPairSlct[8];
	UW16	auw16Resrv[8];
} TUTUDOAParam_t;

typedef struct {
    UW32			uw32OpMode; // check TUTUClearOpMode_e
    UW32			uw32FuncMode; // check TUTUClearFuncMode_e
	UW16			uw16NumOfMic;
	UW16			uw16ECTailLengthInMs;
	UW16			uw16Resv0;
	UW16			uw16Resv1;
	UW16			uw16Resv2;
	UW16			uw16Resv3;
	TUTUAECParam_t	tTUTUAECParam;
	TUTUNSParam_t	tTUTUNSParam;
	TUTUEQParam_t	tTUTUEQParam;
	TUTUDRCParam_t	tTUTUDRCParam;
	TUTUAGCParam_t	tTUTUAGCParam;
	TUTUSCParam_t	tTUTUSCParam;
	TUTUDOAParam_t	tTUTUDOAParam;
} TUTUClearParam_t;


typedef struct {
    W16		w16MicLevel; // MIC waveform level
    W16		w16SCFlg;
	W16		w16SpkrVrfCnfLvl;
	W16		w16Resrv3;
	W16		w16Resrv4;
    W16		w16Resrv5;
	W16		w16Resrv6;
	W16		w16Resrv7;
	W16		w16VtrgrFlg;
	W16		w16Resrv9;
	W16		w16ResrvA;
	W16		w16ResrvB;
	W16		w16ResrvC;
	W16		w16ResrvD;
	W16		w16ResrvE;
	W16		w16ResrvF;
} TUTUClearStat_t;

// Speaker profile structure
typedef struct {
	W32		aw32Info[4];
	W16		aw16Info[72];
} TUTUClearSpkrPrfl_t;

/* ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

UW32 TUTUClear_GetVerNum(W8 *pw8Major, W8 *pw8Minor, W8 *pw8Revision);
UW32 TUTUClear_QueryMemSz(TUTUClearConfig_t *ptTUTUClearConfig);
UW32 TUTUClear_QueryLicenseExpiration(void);
W16 TUTUClear_Init(TUTUClearConfig_t *ptTUTUClearConfig, void *pTUTUClearObjectMem, void **pTUTUClearObject);
W16 TUTUClear_Release(void **pTUTUClearObject);
W16 TUTUClear_OneFrame(void *pTUTUClearObject, W16 *ptAECRef, W16 *ptMIC, W16 *ptLOut, TUTUClearStat_t *ptTUTUClearStat);
W16 TUTUClear_OneFrame_32b(void *pTUTUClearObject, W32 *ptTSMPLAECRef, W32 *ptTSMPLMIC, W32 *ptTSMPLLOut, TUTUClearStat_t *ptTUTUClearStat);
W16 TUTUClear_SetParams(void *pTUTUClearObject, TUTUClearParam_t *ptTUTUClearParam);
W16 TUTUClear_ResetSCFlg(void *pTUTUClearObject, W16 w16FlgHoldinMs);
W16	TUTUClear_InterModComm(void *pTUTUClearObjectSour, void *pTUTUClearObjectDest);
W16 TUTUClear_ImportKeywordData(void *pTUTUClearObject, const W8* fileNameStr);

void TUTUClear_ParsePRMFile_QACT(const char *pszCtrlFileName, TUTUClearConfig_t *ptTUTUClearConfig, TUTUClearParam_t *ptTUTUClearParam);

#ifdef __cplusplus
}
#endif

#endif // _TUTUCLEAR_H_
