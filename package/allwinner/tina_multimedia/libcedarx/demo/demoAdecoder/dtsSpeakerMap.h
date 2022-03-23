/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : dtsSpeakerMap.h
 * Description : demoAdecoder
 * History :
 *
 */

#ifndef DTSSPEAKERMAP_H
#define DTSSPEAKERMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "demo_utils.h"

typedef enum {
    /**< if set then the channel buffers in pSamples do not map to the speakerMask */
    DTSDEC_REPTYPE_UNMAPPED = 0x1,
    /**< indicate Lt/Rt representation, speakerMask will still shows L/R */
    DTSDEC_REPTYPE_LT_RT    = 0x2,
    /**< indicate Lhp/Rhp, headphone representation, speakerMask will still shows L/R */
    DTSDEC_REPTYPE_LH_RH    = 0x4
} DTSDEC_REPTYPE;

typedef enum {
    DTS_SPEAKER_CENTRE = 0, /**< Centre */
    DTS_SPEAKER_LEFT = 1, /**< Left  */
    DTS_SPEAKER_RIGHT = 2, /**< Right  */
    DTS_SPEAKER_LS = 3,/**< Left Surround */
    DTS_SPEAKER_RS = 4,/**< Right Surround  */
    DTS_SPEAKER_LFE1 = 5,/**< Low Frequency Effects 1 */
    DTS_SPEAKER_Cs = 6,/**< Center Surround */
    DTS_SPEAKER_Lsr = 7,/**< Left Surround in Rear  */
    DTS_SPEAKER_Rsr = 8,/**< Right Surround in Rear  */
    DTS_SPEAKER_Lss = 9,/**< Left Surround on Side */
    DTS_SPEAKER_Rss = 10,/**< Right Surround on Side  */
    DTS_SPEAKER_Lc = 11,/**< Between Left and Centre in front  */
    DTS_SPEAKER_Rc = 12,/**< Between Right and Centre in front  */
    DTS_SPEAKER_Lh = 13,/**< Left Height in front */
    DTS_SPEAKER_Ch = 14,/**< Centre Height in Front  */
    DTS_SPEAKER_Rh = 15,/**< Right Height in front  */
    DTS_SPEAKER_LFE2 = 16,/**< Low Frequency Effects 2 */
    DTS_SPEAKER_Lw = 17,/**< Left on side in front */
    DTS_SPEAKER_Rw = 18,/**< Right on side in front  */
    DTS_SPEAKER_Oh = 19,/**< Over the listeners Head */
    DTS_SPEAKER_Lhs = 20,/**< Left Height on Side */
    DTS_SPEAKER_Rhs = 21,/**< Right Height on Side  */
    DTS_SPEAKER_Chr = 22,/**< Centre Height in Rear  */
    DTS_SPEAKER_Lhr = 23,/**< Left Height in Rear */
    DTS_SPEAKER_Rhr = 24,/**< Right Height in Rear  */
    DTS_SPEAKER_Clf = 25,/**< Low Center in Front */
    DTS_SPEAKER_Llf = 26,/**< Low Left in Front */
    DTS_SPEAKER_Rlf = 27,/**< Low Right in Front */
    DTS_SPEAKER_NONE = 28, /* dummy used for stereo output with only one channel */
    DTS_SPEAKER_MAX_SPEAKERS /**< This must always be the last entry on the list */
} dtsDecoderSpeakers ;

typedef enum {
    DTS_MASK_SPEAKER_CENTRE = 0x00000001,/**< Centre */
    DTS_MASK_SPEAKER_LEFT   = 0x00000002,/**< Left  */
    DTS_MASK_SPEAKER_RIGHT  = 0x00000004,/**< Right  */
    DTS_MASK_SPEAKER_LS     = 0x00000008,/**< Left Surround */
    DTS_MASK_SPEAKER_RS     = 0x00000010,/**< Right Surround  */
    DTS_MASK_SPEAKER_LFE1   = 0x00000020,/**< Low Frequency Effects 1 */
    DTS_MASK_SPEAKER_Cs     = 0x00000040,/**< Center Surround  */
    DTS_MASK_SPEAKER_Lsr    = 0x00000080,/**< Left Surround in Rear  */
    DTS_MASK_SPEAKER_Rsr    = 0x00000100,/**< Right Surround in Rear  */
    DTS_MASK_SPEAKER_Lss    = 0x00000200,/**< Left Surround on Side */
    DTS_MASK_SPEAKER_Rss    = 0x00000400,/**< Right Surround on Side  */
    DTS_MASK_SPEAKER_Lc     = 0x00000800,/**< Between Left and Centre in front  */
    DTS_MASK_SPEAKER_Rc     = 0x00001000,/**< Between Right and Centre in front  */
    DTS_MASK_SPEAKER_Lh     = 0x00002000,/**< Left Height in front */
    DTS_MASK_SPEAKER_Ch     = 0x00004000,/**< Centre Height in Front  */
    DTS_MASK_SPEAKER_Rh     = 0x00008000,/**< Right Height in front  */
    DTS_MASK_SPEAKER_LFE2   = 0x00010000,/**< Low Frequency Effects 2 */
    DTS_MASK_SPEAKER_Lw     = 0x00020000,/**< Left on side in front */
    DTS_MASK_SPEAKER_Rw     = 0x00040000,/**< Right on side in front  */
    DTS_MASK_SPEAKER_Oh     = 0x00080000,/**< Over the listeners Head */
    DTS_MASK_SPEAKER_Lhs    = 0x00100000,/**< Left Height on Side */
    DTS_MASK_SPEAKER_Rhs    = 0x00200000,/**< Right Height on Side  */
    DTS_MASK_SPEAKER_Chr    = 0x00400000,/**< Centre Height in Rear  */
    DTS_MASK_SPEAKER_Lhr    = 0x00800000,/**< Left Height in Rear */
    DTS_MASK_SPEAKER_Rhr    = 0x01000000,/**< Right Height in Rear  */
    DTS_MASK_SPEAKER_Clf    = 0x02000000,/**< Low Center in Front */
    DTS_MASK_SPEAKER_Llf    = 0x04000000,/**< Low Left in Front */
    DTS_MASK_SPEAKER_Rlf    = 0x08000000,/**< Low Right in Front */
} dtsSpeakerMask ;

typedef struct dtsWavFileMap {
    uint32_t combinedSpeakerMask ;
    dtsDecoderSpeakers leftSpeaker ;
    dtsDecoderSpeakers rightSpeaker ;
    uint64_t numberOfChannels ;
    const char *pFilename ;
    uint32_t repTypes ;
} dtsWavFileMap ;

#define DTS_WAV_FILE_OUTPUT_MAX_WAV_FILES           (60)

void dtsCapture_getFileBaseName(DecDemo *Demo);

void dtsCapture_capturePcm2wave(DecDemo *Demo);

#endif
