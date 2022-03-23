/***************************************************************************
 tutuSunxi.h
 yanchen.lu@gmems.com

***************************************************************************/
#ifndef _TUTU_SUNXI_H_
#define _TUTU_SUNXI_H_

// define
#define TUTU_SUNXI_DBG_VERBOS_LEVLEL 0 // {0,1,2,3}
#define __TUTU_SUNXI_R16__ // otherwise R18

// structure
// ----------------------------------------------------------------------------
typedef struct tutu_sunxi_state
{
	W64 		aw64OpenCmd[7];
	W64 		w64Signature;
	const UW64	*puw64TblA0;
	const UW64	*puw64TblB0;
	const UW64	*puw64TblC0;
    const UW32	*puw32TblA1;
	const UW32	*puw32TblB1;
	const UW32	*puw32TblC1;
    W32         w32Method;
    W32         w32FsInfo;


} TTTSUNXIState;

#ifdef __cplusplus
extern "C" {
#endif

#define TTSUNXI_Initial		TTAALLW1
#define TTSUNXI_Release		TTAALLW2
#define TTSUNXI_GenRnd		TTAALLW3
#define TTSUNXI_GenCode		TTAALLW4
#define TTSUNXI_VerifyCode	TTAALLW5

// routine
W16 TTSUNXI_Initial(TTTSUNXIState *ptTTSUNXIState);
W16 TTSUNXI_Release(TTTSUNXIState *ptTTSUNXIState);
W16 TTSUNXI_GenRnd(TTTSUNXIState *ptTTSUNXIState, W16 w16Seed);
W16 TTSUNXI_GenCode(TTTSUNXIState *ptTTSUNXIState);
W16 TTSUNXI_VerifyCode(TTTSUNXIState *ptTTSUNXIState);

#ifdef __cplusplus
}
#endif

#endif //_TUTU_SUNXI_H_
