/***************************************************************************
 tutuZ8ida.h
 yanchen.lu@gmems.com

***************************************************************************/
#ifndef _TUTU_Z8IDA_H_
#define _TUTU_Z8IDA_H_

// define
//#define __EN_Z8IDA_DBG_MSG__
//#define __EN_Z8IDA_ENDINIT__
//#define __EN_Z8IDA_KEY_PRESET__
#define Z8IDA_CMD_BFR_L             64 // in byte

// structure
typedef struct tutu_z8ida_state
{
	W32		    w32Fd;
	UW32		uw32SleepTInMicroSec;
    UW8			auw8SN[16];
    UW8		    auw8Key[16];
    UW8			auw8RN[16];
    UW8		    auw8Code[16];
    UW8	        auw8CmdBfr[Z8IDA_CMD_BFR_L];
	UW8         *puw8CmdBfr;

} TTTZ8IDAState;

#ifdef __cplusplus
extern "C" {
#endif

// routine
W16 TTZ8IDA_Initial(TTTZ8IDAState *ptTTZ8IDAState);
W16 TTZ8IDA_Release(TTTZ8IDAState *ptTTZ8IDAState);
W16 TTZ8IDA_GenRnd(TTTZ8IDAState *ptTTZ8IDAState);
W16 TTZ8IDA_GenCode(TTTZ8IDAState *ptTTZ8IDAState);
W16 TTZ8IDA_VerifyCode(TTTZ8IDAState *ptTTZ8IDAState);

#ifdef __cplusplus
}
#endif

#endif //_TUTU_Z8IDA_H_
