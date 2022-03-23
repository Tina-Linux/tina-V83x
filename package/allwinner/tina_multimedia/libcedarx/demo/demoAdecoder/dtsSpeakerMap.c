/*
 * Copyright (c) 2008-2017 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : dtsSpeakerMap.c
 * Description : demoAdecoder
 * History :
 *
 */

#include "dtsSpeakerMap.h"
#include "demo_utils.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG "dtsSpeakerMap"
#endif
#define DEMO_DEBUG 0

#ifdef CAPTURE_DATA
extern const dtsWavFileMap dtsWavFileSpeakerMapper[DTS_WAV_FILE_OUTPUT_MAX_WAV_FILES];
FILE* ch_data[DTS_SPEAKER_MAX_SPEAKERS]={0};
#endif

const dtsWavFileMap dtsWavFileSpeakerMapper[DTS_WAV_FILE_OUTPUT_MAX_WAV_FILES] =
{
    /* stereo */
    { DTS_MASK_SPEAKER_CENTRE | DTS_MASK_SPEAKER_LFE1,
DTS_SPEAKER_CENTRE, DTS_SPEAKER_LFE1,  2, "_C_LFE1.wav", 0 },
    { DTS_MASK_SPEAKER_CENTRE | DTS_MASK_SPEAKER_Cs,
DTS_SPEAKER_CENTRE, DTS_SPEAKER_Cs,  2, "_C_Cs.wav", 0 },
    { DTS_MASK_SPEAKER_CENTRE | DTS_MASK_SPEAKER_Ch,
DTS_SPEAKER_CENTRE, DTS_SPEAKER_Ch,  2, "_C_Ch.wav", 0 },
    { DTS_MASK_SPEAKER_LEFT | DTS_MASK_SPEAKER_RIGHT,
DTS_SPEAKER_LEFT, DTS_SPEAKER_RIGHT, 2, "_L_R.wav", 0 },
    { DTS_MASK_SPEAKER_LS | DTS_MASK_SPEAKER_RS,
DTS_SPEAKER_LS, DTS_SPEAKER_RS, 2, "_Ls_Rs.wav", 0 },
    { DTS_MASK_SPEAKER_Cs | DTS_MASK_SPEAKER_Ch,
DTS_SPEAKER_Cs, DTS_SPEAKER_Ch, 2, "_Cs_Ch.wav", 0 },
    { DTS_MASK_SPEAKER_Cs | DTS_MASK_SPEAKER_Oh,
DTS_SPEAKER_Cs, DTS_SPEAKER_Oh, 2, "_Cs_Oh.wav", 0 },
    { DTS_MASK_SPEAKER_Lh | DTS_MASK_SPEAKER_Rh,
DTS_SPEAKER_Lh, DTS_SPEAKER_Rh, 2, "_Lh_Rh.wav", 0 },
    { DTS_MASK_SPEAKER_Lsr | DTS_MASK_SPEAKER_Rsr,
DTS_SPEAKER_Lsr, DTS_SPEAKER_Rsr, 2, "_Lsr_Rsr.wav", 0 },
    { DTS_MASK_SPEAKER_Lc | DTS_MASK_SPEAKER_Rc,
DTS_SPEAKER_Lc, DTS_SPEAKER_Rc, 2, "_Lc_Rc.wav", 0 },
    { DTS_MASK_SPEAKER_Lw | DTS_MASK_SPEAKER_Rw,
DTS_SPEAKER_Lw, DTS_SPEAKER_Rw, 2, "_Lw_Rw.wav", 0 },
    { DTS_MASK_SPEAKER_Lss | DTS_MASK_SPEAKER_Rss,
DTS_SPEAKER_Lss, DTS_SPEAKER_Rss, 2, "_Lss_Rss.wav", 0 },
    { DTS_MASK_SPEAKER_Lhs | DTS_MASK_SPEAKER_Rhs,
DTS_SPEAKER_Lhs, DTS_SPEAKER_Rhs, 2, "_Lhs_Rhs.wav", 0 },
    { DTS_MASK_SPEAKER_Lhr | DTS_MASK_SPEAKER_Rhr,
DTS_SPEAKER_Lhr, DTS_SPEAKER_Rhr, 2, "_Lhr_Rhr.wav", 0 },
    { DTS_MASK_SPEAKER_Llf | DTS_MASK_SPEAKER_Rlf,
DTS_SPEAKER_Llf, DTS_SPEAKER_Rlf, 2, "_Llf_Rlf.wav", 0 },
    { DTS_MASK_SPEAKER_LEFT | DTS_MASK_SPEAKER_RIGHT,
DTS_SPEAKER_LEFT, DTS_SPEAKER_RIGHT, 2, "_Lt_Rt.wav", DTSDEC_REPTYPE_LT_RT },
    { DTS_MASK_SPEAKER_LEFT | DTS_MASK_SPEAKER_RIGHT,
DTS_SPEAKER_LEFT, DTS_SPEAKER_RIGHT, 2, "_Lhp_Rhp.wav", DTSDEC_REPTYPE_LH_RH },
    { DTS_MASK_SPEAKER_Oh | DTS_MASK_SPEAKER_Chr,
DTS_SPEAKER_Oh, DTS_SPEAKER_Chr,  2, "_Oh_Chr.wav", 0 },
    { DTS_MASK_SPEAKER_CENTRE,
DTS_SPEAKER_CENTRE, DTS_SPEAKER_NONE, 2, "_C_0.wav", 0 },
    { DTS_MASK_SPEAKER_Cs,
DTS_SPEAKER_Cs, DTS_SPEAKER_NONE, 2, "_Cs_0.wav", 0 },
    { DTS_MASK_SPEAKER_Ch,
DTS_SPEAKER_Ch, DTS_SPEAKER_NONE, 2, "_Ch_0.wav", 0 },
    { DTS_MASK_SPEAKER_Oh,
DTS_SPEAKER_Oh, DTS_SPEAKER_NONE,  2, "_Oh_0.wav", 0 },
    { DTS_MASK_SPEAKER_Chr,
DTS_SPEAKER_Chr, DTS_SPEAKER_NONE, 2, "_Chr_0.wav", 0 },
    { DTS_MASK_SPEAKER_Clf,
DTS_SPEAKER_Clf, DTS_SPEAKER_NONE, 2, "_Clf_0.wav", 0 },
    { DTS_MASK_SPEAKER_LFE1,
DTS_SPEAKER_NONE, DTS_SPEAKER_LFE1,  2, "_0_LFE1.wav", 0 },
    { DTS_MASK_SPEAKER_LFE1,
DTS_SPEAKER_NONE, DTS_SPEAKER_LFE1,  2, "_0_LFE1.wav", DTSDEC_REPTYPE_LT_RT },
    { DTS_MASK_SPEAKER_LFE2,
DTS_SPEAKER_NONE, DTS_SPEAKER_LFE2,  2, "_0_LFE2.wav", 0 },

    /* mono */
    { DTS_MASK_SPEAKER_LEFT, DTS_SPEAKER_LEFT, DTS_SPEAKER_NONE, 1, "L.wav", 0 },
    { DTS_MASK_SPEAKER_RIGHT, DTS_SPEAKER_RIGHT, DTS_SPEAKER_NONE, 1, "R.wav", 0 },
    { DTS_MASK_SPEAKER_LS, DTS_SPEAKER_LS, DTS_SPEAKER_NONE, 1, "Ls.wav", 0 },
    { DTS_MASK_SPEAKER_RS, DTS_SPEAKER_RS, DTS_SPEAKER_NONE, 1, "Rs.wav", 0 },
    { DTS_MASK_SPEAKER_CENTRE, DTS_SPEAKER_CENTRE, DTS_SPEAKER_NONE, 1, "C.wav", 0 },
    { DTS_MASK_SPEAKER_LFE1, DTS_SPEAKER_LFE1, DTS_SPEAKER_NONE, 1, "LFE1.wav", 0 },
    { DTS_MASK_SPEAKER_LFE1, DTS_SPEAKER_LFE1, DTS_SPEAKER_NONE, 1, "LFE1.wav",
DTSDEC_REPTYPE_LT_RT },
    { DTS_MASK_SPEAKER_Lh, DTS_SPEAKER_Lh, DTS_SPEAKER_NONE, 1, "Lh.wav", 0 },
    { DTS_MASK_SPEAKER_Rh, DTS_SPEAKER_Rh, DTS_SPEAKER_NONE, 1, "Rh.wav", 0 },
    { DTS_MASK_SPEAKER_Lsr, DTS_SPEAKER_Lsr, DTS_SPEAKER_NONE, 1, "Lsr.wav", 0 },
    { DTS_MASK_SPEAKER_Rsr, DTS_SPEAKER_Rsr, DTS_SPEAKER_NONE, 1, "Rsr.wav", 0 },
    { DTS_MASK_SPEAKER_Lc, DTS_SPEAKER_Lc, DTS_SPEAKER_NONE, 1, "Lc.wav", 0 },
    { DTS_MASK_SPEAKER_Rc, DTS_SPEAKER_Rc, DTS_SPEAKER_NONE, 1, "Rc.wav", 0 },
    { DTS_MASK_SPEAKER_Lw, DTS_SPEAKER_Lw, DTS_SPEAKER_NONE, 1, "Lw.wav", 0 },
    { DTS_MASK_SPEAKER_Rw, DTS_SPEAKER_Rw, DTS_SPEAKER_NONE, 1, "Rw.wav", 0 },
    { DTS_MASK_SPEAKER_Lss, DTS_SPEAKER_Lss, DTS_SPEAKER_NONE, 1, "Lss.wav", 0 },
    { DTS_MASK_SPEAKER_Rss, DTS_SPEAKER_Rss, DTS_SPEAKER_NONE, 1, "Rss.wav", 0 },
    { DTS_MASK_SPEAKER_Lhs, DTS_SPEAKER_Lhs, DTS_SPEAKER_NONE, 1, "Lhs.wav", 0 },
    { DTS_MASK_SPEAKER_Rhs, DTS_SPEAKER_Rhs, DTS_SPEAKER_NONE, 1, "Rhs.wav", 0 },
    { DTS_MASK_SPEAKER_Lhr, DTS_SPEAKER_Lhr, DTS_SPEAKER_NONE, 1, "Lhr.wav", 0 },
    { DTS_MASK_SPEAKER_Rhr, DTS_SPEAKER_Rhr, DTS_SPEAKER_NONE, 1, "Rhr.wav", 0 },
    { DTS_MASK_SPEAKER_Cs, DTS_SPEAKER_Cs, DTS_SPEAKER_NONE, 1, "Cs.wav", 0 },
    { DTS_MASK_SPEAKER_LFE2, DTS_SPEAKER_LFE2, DTS_SPEAKER_NONE, 1, "LFE2.wav", 0 },
    { DTS_MASK_SPEAKER_Ch, DTS_SPEAKER_Ch, DTS_SPEAKER_NONE, 1, "Ch.wav", 0 },
    { DTS_MASK_SPEAKER_Chr, DTS_SPEAKER_Chr, DTS_SPEAKER_NONE, 1, "Chr.wav", 0 },
    { DTS_MASK_SPEAKER_Llf, DTS_SPEAKER_Llf, DTS_SPEAKER_NONE, 1, "Llf.wav", 0 },
    { DTS_MASK_SPEAKER_Rlf, DTS_SPEAKER_Rlf, DTS_SPEAKER_NONE, 1, "Rlf.wav", 0 },
    { DTS_MASK_SPEAKER_Oh, DTS_SPEAKER_Oh, DTS_SPEAKER_NONE, 1, "Oh.wav", 0 },
    { DTS_MASK_SPEAKER_Clf, DTS_SPEAKER_Clf, DTS_SPEAKER_NONE, 1, "Clf.wav", 0 },
    { DTS_MASK_SPEAKER_LEFT, DTS_SPEAKER_LEFT, DTS_SPEAKER_NONE, 1, "Lt.wav",
DTSDEC_REPTYPE_LT_RT },
    { DTS_MASK_SPEAKER_RIGHT, DTS_SPEAKER_RIGHT, DTS_SPEAKER_NONE, 1, "Rt.wav",
DTSDEC_REPTYPE_LT_RT },
    { DTS_MASK_SPEAKER_LEFT, DTS_SPEAKER_LEFT, DTS_SPEAKER_NONE, 1, "Lhp.wav",
DTSDEC_REPTYPE_LH_RH },
    { DTS_MASK_SPEAKER_RIGHT, DTS_SPEAKER_RIGHT, DTS_SPEAKER_NONE, 1, "Rhp.wav",
DTSDEC_REPTYPE_LH_RH },
} ;

const char* SPDIF_OUTPUT_FILENAME = "_SPDIF.wav";
const char* NON_SIX_CHANNEL_DOWNMIX_OUTPUT_FILE_SUFFIX = "_MultDmixLess5xCh";
const char* SIX_CHANNEL_DOWNMIX_OUTPUT_FILE_SUFFIX = "_MultDmix6Ch";

void dtsCapture_getFileBaseName(DecDemo *Demo)
{
    int ret_line = 0, ret_com = 0;
    if(!Demo){
        demo_loge("Demo nul ptr...");
        return ;
    }

    ret_com = strlen(Demo->pInputFile);
    while(1)
    {
        char start = Demo->pInputFile[ret_com-1];
        if(start == '.')
            break;
        ret_com--;
    }
    ret_line = ret_com;
    while(1)
    {
        char start = Demo->pInputFile[ret_line-1];
        if(start == '/')
            break;
        ret_line--;
    }
    memcpy(Demo->file_base_name, &Demo->pInputFile[ret_line], ret_com - ret_line - 1);
    demo_logd("file_base_name >>>> %s", Demo->file_base_name);
}

void dtsCapture_capturePcm2wave(DecDemo *Demo)
{
#ifdef CAPTURE_DATA
    int64_t file_len = 0;
    uint32_t chan_mask = 0;
    int  repTypeinfo = 0;
    int  totalchannel = 0;
    int  i;
    FILE * repType_ca = 0;

    if(!Demo){
        demo_loge("Demo nul ptr...");
        return ;
    }

    DEMO_SAFE_OPEN_FILE(repType_ca, "/data/camera/repType_ca_info", "rb", "repTypeinfo")
    if(repType_ca)
    {
        int over = 0;

        DEMO_SAFE_READ_FILE(&repTypeinfo, 4, 1, repType_ca)
        DEMO_SAFE_READ_FILE(&chan_mask, 4, 1, repType_ca)
        DEMO_SAFE_READ_FILE(&file_len, 8, 1, repType_ca)
        DEMO_SAFE_READ_FILE(&over, 4, 1, repType_ca)
        DEMO_SAFE_CLOSE_FILE(repType_ca, "repType_ca")

        demo_logd("repTypeinfo : %d, chan_mask : 0x%04x, file_len : %ld, over : %d"
                , repTypeinfo, chan_mask, file_len, over);
    }

    for( i = 0; i < DTS_SPEAKER_MAX_SPEAKERS; i++ )
    {
        char src_path[64] = {0};
        int64_t len_tmp = 0;
        sscanf("/data/camera/ch_", "%63s", src_path);
        sprintf(&src_path[strlen(src_path)], "%d.pcm", i);
        DEMO_SAFE_OPEN_FILE(ch_data[i], src_path, "rb", src_path)

        if(chan_mask & (1<<i))
        {
            totalchannel++;
            demo_logd("find chan totalNumberOfSpeakers : %d", totalchannel);
        }
    }

    demo_logd("channel mask : 0x%04x, totalchannel : %d", chan_mask, totalchannel);
    for( i = 0; i < DTS_WAV_FILE_OUTPUT_MAX_WAV_FILES; i++ ){
        if(( dtsWavFileSpeakerMapper[i].numberOfChannels == 2 ) &&
            (( chan_mask & dtsWavFileSpeakerMapper[i].combinedSpeakerMask ) ==
            dtsWavFileSpeakerMapper[i].combinedSpeakerMask) &&
            repTypeinfo == dtsWavFileSpeakerMapper[i].repTypes)
        {
            FILE* this_out = NULL;
            int samples = 0, j;
            char wavheader[64] = {0};
            char tmpfiledir[256] = {0};

            if( totalchannel != 8 )
            {
                if( DTS_MASK_SPEAKER_CENTRE & dtsWavFileSpeakerMapper[i].combinedSpeakerMask )
                {
                    uint32_t theOtherChannel = 0;
                    theOtherChannel =
                    dtsWavFileSpeakerMapper[i].combinedSpeakerMask & ( ~DTS_MASK_SPEAKER_CENTRE );

                    if( ( ( DTS_MASK_SPEAKER_LFE1 & theOtherChannel ) == 0 ) &&
                            ( theOtherChannel != 0 ) )
                    {
                        continue;
                    }
                }
            }
            chan_mask &= ~( dtsWavFileSpeakerMapper[i].combinedSpeakerMask );
            memcpy(tmpfiledir, Demo->pOutputDir, strlen(Demo->pOutputDir));
            memcpy(&tmpfiledir[strlen(tmpfiledir)], Demo->file_base_name,
                strlen(Demo->file_base_name));
            sscanf(dtsWavFileSpeakerMapper[i].pFilename, "%255s", &tmpfiledir[strlen(tmpfiledir)]);
            demo_logd("file_len : %ld, bps : %d", file_len, Demo->dsp->bps);
            demo_logd("pick combinedSpeakerMask : 0x%04x, rest chan_mask : 0x%04x",
                    dtsWavFileSpeakerMapper[i].combinedSpeakerMask, chan_mask);

            DEMO_SAFE_OPEN_FILE(this_out, tmpfiledir, "wb", tmpfiledir)

            Setaudiobs_pcm(wavheader, 2, Demo->dsp->spr, Demo->dsp->bps,
                        WAVE_FORMAT_PCM, file_len*2);

            DEMO_SAFE_WRITE_FILE(wavheader, 1, 0x2c, this_out);

            samples = file_len * 8/ Demo->dsp->bps;
            for(j=0; j<samples; j++)
            {
                int tmp = 0;
                if(dtsWavFileSpeakerMapper[i].leftSpeaker != DTS_SPEAKER_NONE){
                    DEMO_SAFE_READ_FILE(&tmp, Demo->dsp->bps/8, 1,
                                    ch_data[dtsWavFileSpeakerMapper[i].leftSpeaker]);
                    DEMO_SAFE_WRITE_FILE(&tmp, Demo->dsp->bps/8, 1, this_out);
                }
                else
                {
                    tmp = 0;
                    DEMO_SAFE_WRITE_FILE(&tmp, Demo->dsp->bps/8, 1, this_out);
                }

                if(dtsWavFileSpeakerMapper[i].rightSpeaker != DTS_SPEAKER_NONE){
                    DEMO_SAFE_READ_FILE(&tmp, Demo->dsp->bps/8, 1,
                                    ch_data[dtsWavFileSpeakerMapper[i].rightSpeaker]);
                    DEMO_SAFE_WRITE_FILE(&tmp, Demo->dsp->bps/8, 1, this_out);
                }
                else
                {
                    tmp = 0;
                    DEMO_SAFE_WRITE_FILE(&tmp, Demo->dsp->bps/8, 1, this_out);
                }
            }
            DEMO_SAFE_CLOSE_FILE(this_out, tmpfiledir)
        }
    }
    for( i = 0; i < DTS_SPEAKER_MAX_SPEAKERS; i++ )
    {
        char file_name[64] = {0};
        sscanf("ch_data_", "%63s", file_name);
        sprintf(&file_name[strlen(file_name)], "%d", i);
        DEMO_SAFE_CLOSE_FILE(ch_data[i], file_name);
    }
#else
    CDX_UNUSE(Demo);
#endif
}
