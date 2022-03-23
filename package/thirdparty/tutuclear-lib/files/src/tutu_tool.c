#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "tutu/tutu_tool.h"
#include "tutu/tutuClear.h"

//---------------------------------------------------------------------------
// Following 8 functions are for parsing parameter file
TIniFileInfo *OpenIniFile(const char *pszFileName)
{
    FILE            *fptr;
    TIniFileInfo    *pIniFileInfo;
    int             i, iLineCount;
    char            buf[256];
    char            *pBuf;
    char            *pTemp;

    fptr = fopen(pszFileName, "r");
    if (!fptr)
        return NULL;

    pIniFileInfo = (TIniFileInfo *)malloc(sizeof(TIniFileInfo));
    if (!pIniFileInfo) {
        fclose(fptr);
        return NULL;
    }
    pIniFileInfo->fptr = fptr;

    for (iLineCount=0; fgets(buf, 255, fptr); iLineCount++);

    pIniFileInfo->pIdentValueList = (TIdentValue *)malloc(sizeof(TIdentValue) * iLineCount);

    fseek(fptr, 0, SEEK_SET);
    pIniFileInfo->iLineCount = iLineCount;

    for (i=0; i<iLineCount; i++)
    {
        fgets(buf, 255, fptr);
        pTemp = pIniFileInfo->pIdentValueList[i].PszIdent;

        //printf("%2d-th line(i): %s", i, buf);

        // skip while space
        pBuf = buf;
        while ((*pBuf==' ') || (*pBuf=='\t'))
        {
            pBuf ++;
        }

        // --------------------------------------------------------------------
        // fill ident
        while (((*pBuf)!=' ') &&
                ((*pBuf)!='\t') &&
                (*pBuf))
        {
            *pTemp++ = *pBuf++;
        }
        *pTemp = 0;
        //printf("%2d-th line(t): %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        // skip while space
        while ((*pBuf==' ') || (*pBuf=='\t'))
        {
            pBuf ++;
        }

        // --------------------------------------------------------------------
        // fill value
        pTemp = pIniFileInfo->pIdentValueList[i].PszValue;

        while (((*pBuf)!=' ') &&
                ((*pBuf)!='\n') &&
                ((*pBuf)!='\r') &&
                ((*pBuf)!='\t') &&
                ((*pBuf)!=';') &&
                (*pBuf))
        {
            *pTemp++ = *pBuf++;
        }

        *pTemp = 0;
        //printf("%2d-th line(v): %s\n", i, pIniFileInfo->pIdentValueList[i].PszValue);

    }
    fseek(fptr, 0, SEEK_SET);
    return pIniFileInfo;
}
//---------------------------------------------------------------------------
int CloseIniFile(TIniFileInfo *pIniFileInfo)
{
    fclose(pIniFileInfo->fptr);
    free(pIniFileInfo->pIdentValueList);
    free(pIniFileInfo);

    return 1;
}
//---------------------------------------------------------------------------
int ReadUWord16(TIniFileInfo *pIniFileInfo,
                const char   *pszIdent,
                UW16         *puw16,
                UW16         uw16Default)
{
    int         i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *puw16 = uw16Default;
    //printf("%s>>\n", pszIdent);
    for (i=0; i<pIniFileInfo->iLineCount; i++)
    {
        //printf("%2d-th line: %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        if (strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *puw16 = (UW16)strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, uw16Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadWord16(TIniFileInfo *pIniFileInfo,
               const char   *pszIdent,
               W16          *pw16,
               W16          w16Default)
{
    int         i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *pw16 = w16Default;
    //printf("%s>>\n", pszIdent);
    for (i=0; i<pIniFileInfo->iLineCount; i++)
    {
        //printf("%2d-th line: %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        if (strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *pw16 = (W16)strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, w16Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, W32 *pw32Value, W32 w32Default)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *pw32Value = w32Default;
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *pw32Value = strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, w32Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadUWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, UW32 *puw32Value, UW32 uw32Default)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *puw32Value = uw32Default;
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *puw32Value = strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, uw32Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadString(TIniFileInfo *pIniFileInfo, const char *pszIdent, char *pszValue, char *pszDefault)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    strcpy(pszValue, pszDefault);
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            strcpy(pszValue, pIdentValue[i].PszValue);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %s (presse any key to continue).\n", pszIdent, pszDefault);
        //getche();
    }

    return 1;
}

void TUTUClear_ParsePRMFile_QACT(const char            *pszCtrlFileName,
                                 TUTUClearConfig_t     *ptTUTUClearConfig,
                                 TUTUClearParam_t      *ptTUTUClearParam)
{
    TIniFileInfo    *pIniFileInfo;

    if ((pIniFileInfo=OpenIniFile(pszCtrlFileName)) == NULL) return;

    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_SAMPLING_FREQ", &ptTUTUClearConfig->uw16SamplingFreq, 0);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_FRAME_SZ_IN_MS", &ptTUTUClearConfig->uw16FrameSz, 10);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_MAX_NUM_MIC", &ptTUTUClearConfig->uw16MaxNumOfMic, 0);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_MAX_TAIL_LENGTH_IN_MS", &ptTUTUClearConfig->uw16MaxTailLength, 0);

    /* Params to TUTUClear_SetParams() ----------------------------------------- */
    ReadUWord32(pIniFileInfo, "TUTU_PARAM_SYS.uw32OpMode", &ptTUTUClearParam->uw32OpMode, 0);
    ReadUWord32(pIniFileInfo, "TUTU_PARAM_SYS.uw32FuncMode", &ptTUTUClearParam->uw32FuncMode, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16NumOfMic", &ptTUTUClearParam->uw16NumOfMic, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16ECTailLengthInMs", &ptTUTUClearParam->uw16ECTailLengthInMs, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv0", &ptTUTUClearParam->uw16Resv0, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv1", &ptTUTUClearParam->uw16Resv1, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv2", &ptTUTUClearParam->uw16Resv2, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv3", &ptTUTUClearParam->uw16Resv3, 0);

    /* Params to AEC ----------------------------------------------------------- */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[0]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[1]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[2]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[3]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[4]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[5]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[6]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[7]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[8]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[9]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[10]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[11]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[12]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[13]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[14]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[15]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[15], 0);
    /* Params to NS ------------------------------------------------------------ */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16Lambda", &ptTUTUClearParam->tTUTUNSParam.uw16Lambda, 32113);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16SupressionLevel", &ptTUTUClearParam->tTUTUNSParam.uw16SupressionLevel, 1);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16BGSimplicity", &ptTUTUClearParam->tTUTUNSParam.uw16BGSimplicity, 4096);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16D", &ptTUTUClearParam->tTUTUNSParam.uw16D, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[0]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[1]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[2]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[3]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[4]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[5]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[6]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[7]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[8]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[9]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[10]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[11]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[12]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[13]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[14]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[15]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[15], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[16]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[16], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[17]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[17], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[18]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[18], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[19]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[19], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[20]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[20], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[21]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[21], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[22]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[22], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[23]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[23], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[24]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[24], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[25]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[25], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[26]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[26], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[27]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[27], 0);
    /* Params to EQ ------------------------------------------------------------ */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.uw16EQPartitionBegin", &ptTUTUClearParam->tTUTUEQParam.uw16EQPartitionBegin, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.uw16Resrv0", &ptTUTUClearParam->tTUTUEQParam.uw16Resrv0, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[0]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[1]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[2]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[3]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[4]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[5]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[6]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[7]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[8]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[9]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[10]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[11]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[12]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[13]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[14]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[15]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[15], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[0]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[1]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[2]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[3]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[4]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[4], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[5]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[5], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[6]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[6], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[7]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[7], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[8]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[8], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[9]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[9], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[10]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[10], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[11]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[11], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[12]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[12], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[13]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[13], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[14]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[14], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[15]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[15], 0);
    /* Params to DRC ----------------------------------------------------------- */
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16ExpanderInputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16ExpanderInputLevels[0], -96);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16ExpanderInputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16ExpanderInputLevels[1], -96);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16NumCompressorKnees", &ptTUTUClearParam->tTUTUDRCParam.uw16NumCompressorKnees, 3);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16Resrv0", &ptTUTUClearParam->tTUTUDRCParam.uw16Resrv0, 3);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[2]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[3]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[2]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[3]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16AttackTime", &ptTUTUClearParam->tTUTUDRCParam.uw16AttackTime, 5);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16ReleaseTime", &ptTUTUClearParam->tTUTUDRCParam.uw16ReleaseTime, 10);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16MakeupGain", &ptTUTUClearParam->tTUTUDRCParam.uw16MakeupGain, 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16Resrv1", &ptTUTUClearParam->tTUTUDRCParam.uw16Resrv1, 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.auw16Resrv[0]", &ptTUTUClearParam->tTUTUDRCParam.auw16Resrv[0], 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.auw16Resrv[1]", &ptTUTUClearParam->tTUTUDRCParam.auw16Resrv[1], 0x0100);
    /* Params to AGC ----------------------------------------------------------- */
    ReadWord16(pIniFileInfo, "TUTU_PARAM_AGC.w16TargetOutputLevel", &ptTUTUClearParam->tTUTUAGCParam.w16TargetOutputLevel, -9);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16MaxGain", &ptTUTUClearParam->tTUTUAGCParam.uw16MaxGain, 2048);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16MinGain", &ptTUTUClearParam->tTUTUAGCParam.uw16MinGain, 32);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16RespRate", &ptTUTUClearParam->tTUTUAGCParam.uw16RespRate, 16384);

    /* Params to DOA ----------------------------------------------------------- */
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[0]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[1]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[2]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[3]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[4]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[4], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[5]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[5], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[6]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[6], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosX[7]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosX[7], 0);

    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[0]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[1]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[2]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[3]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[4]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[4], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[5]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[5], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[6]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[6], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.aw16MicPosY[7]", &ptTUTUClearParam->tTUTUDOAParam.aw16MicPosY[7], 0);

    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[0]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[1]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[2]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[3]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[4]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[5]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[6]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16MicPairSlct[7]", &ptTUTUClearParam->tTUTUDOAParam.auw16MicPairSlct[7], 0);

    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[0]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[1]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[2]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[3]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[4]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[4], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[5]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[5], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[6]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[6], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DOA.auw16Resrv[7]", &ptTUTUClearParam->tTUTUDOAParam.auw16Resrv[7], 0);

    CloseIniFile(pIniFileInfo);
    return;
}
