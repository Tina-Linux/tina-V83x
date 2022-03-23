//#define LOG_NDEBUG 0
#define LOG_TAG "awPlayReadyLicense"
#include <utils/Log.h>

#include "awPlayReadyLicense.h"
#include "cdx_log.h"
#include <utils/String8.h>
#include <media/mediaplayer.h>

#if BOARD_USE_PLAYREADY

#define DUMP_NETWORK_DATA 1

#include <drmwindowsenv.h>
#include <drmfeatures.h>
#include <drmtypes.h>
#include <drmutilitieslite.h>
#include <drmmanager.h>
#include <drmdomainimp.h>
#include <drmrevocation.h>
#include <drmcmdlnpars.h>
#include <drmtoolsutils.h>
#include <drmtoolsmediafile.h>
#include <drmheaderparser.h>
#include <drmutf.h>
#include <drmsoapxmlutility.h>
#include <drmmeterapi.h>
#include <drmmathsafe.h>
#include <drmtoolsnetio.h>
#include <drmtrace.h>
#include <drmtoolsinit.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAYREADY_TMPFILE_DIR "/data/mediadrm/"

static DRM_RESULT DRM_CALL PolicyCallback(
    __in     const DRM_VOID *f_pvPolicyCallbackData,
    __in     DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
    __in_opt const DRM_KID *f_pKID,
    __in_opt const DRM_LID *f_pLID,
    __in     const DRM_VOID *f_pv )
{
    (void)f_pKID;
    (void)f_pLID;
    (void)f_pv;
    return DRMTOOLS_PrintPolicyCallback( f_pvPolicyCallbackData, f_dwCallbackType );
}

static void DebugAnalyze(unsigned long dr, const char* file, unsigned long line, const char* expr)
{
    if (dr != 0)
        logd("%s:%ld 0x%x(%s)", file, line, (int)dr, expr);
}

static DRM_RESULT DRM_CALL DumpData(
    __in                    const DRM_CONST_STRING *f_path,
    __in_ecount(f_cbPacket) DRM_BYTE  *f_pbPacket,
    __in                    DRM_DWORD  f_cbPacket )
{
    DRM_RESULT dr = DRM_SUCCESS;

#if DUMP_NETWORK_DATA

    OEM_FILEHDL fp                                                = OEM_INVALID_HANDLE_VALUE;
    DRM_DWORD   cbWritten                                         = 0;

    ChkArg( f_pbPacket != NULL && f_cbPacket > 0 );

    fp = Oem_File_Open( NULL,
                        f_path->pwszString,
                        OEM_GENERIC_WRITE,
                        OEM_FILE_SHARE_NONE,
                        OEM_CREATE_ALWAYS,
                        OEM_ATTRIBUTE_NORMAL );

    if ( fp == OEM_INVALID_HANDLE_VALUE )
    {
        ChkDR( DRM_E_FAIL );
    }

    if ( !Oem_File_Write( fp, f_pbPacket, f_cbPacket, &cbWritten ) ||
         cbWritten != f_cbPacket )
    {
        ChkDR( DRM_E_FAIL );
    }

ErrorExit:

    if ( fp != OEM_INVALID_HANDLE_VALUE )
    {
        Oem_File_Close( fp );
    }

#endif

    return dr;
}

static DRM_BOOL DRM_CALL LoadFile(const DRM_CONST_STRING * pPath, DRM_BYTE **ppBuffer, DRM_DWORD *pSize)
{
    DRM_RESULT dr = DRM_SUCCESS;
    OEM_FILEHDL fp = OEM_INVALID_HANDLE_VALUE;
    DRM_DWORD cbRead = 0;

    ChkBOOL( ( fp = Oem_File_Open( NULL,
                                     pPath->pwszString,
                                     OEM_GENERIC_READ,
                                     OEM_FILE_SHARE_READ,
                                     OEM_OPEN_EXISTING,
                                     OEM_ATTRIBUTE_NORMAL ) ) != OEM_INVALID_HANDLE_VALUE,
             DRM_E_FILEOPEN );

    ChkBOOL( Oem_File_SetFilePointer( fp,
                                      0,
                                      OEM_FILE_END,
                                      pSize ), DRM_E_FILE_SEEK_ERROR );

    ChkMem( *ppBuffer = ( DRM_BYTE * )Oem_MemAlloc( *pSize ) );

    ChkBOOL( Oem_File_SetFilePointer( fp,
                                      0,
                                      OEM_FILE_BEGIN,
                                      NULL ), DRM_E_FILE_SEEK_ERROR );

    ChkBOOL( Oem_File_Read( fp,
                            *ppBuffer,
                            *pSize,
                            &cbRead ), DRM_E_FILE_READ_ERROR );

    ChkBOOL( cbRead == *pSize, DRM_E_FILE_READ_ERROR );

ErrorExit:

    if ( fp != OEM_INVALID_HANDLE_VALUE )
    {
        Oem_File_Close( fp );
    }

    return dr;

}

static DRM_RESULT GenerateLicenseChallege(DRM_APP_CONTEXT *AppContext, DRM_BYTE *pbHeader, DRM_DWORD cbHeader, DRM_BYTE **ppbChallenge, DRM_DWORD *pcbChallenge)
{
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_CONST_STRING  *rgpdstrRights[ 1 ] = { 0 };
    DRM_BYTE *pbHeader2 = NULL;
    DRM_STRING dstrHeader = DRM_EMPTY_DRM_STRING;
    DRM_SUBSTRING dasstrHeader1 = DRM_EMPTY_DRM_SUBSTRING;
    DRM_BYTE *pbChallenge = NULL;
    DRM_DWORD cbChallenge = 0;
    DRM_CHAR rgchURL[ 1024 ];
    DRM_DWORD cchURL = 1024;
    DRM_ID poBatchID = {0};
    rgpdstrRights[ 0 ] = &g_dstrWMDRM_RIGHT_PLAYBACK;

    ChkMem( pbHeader2 = (DRM_BYTE *)Oem_MemAlloc( cbHeader * sizeof( DRM_WCHAR ) ) );

    DRM_DSTR_FROM_PB( &dstrHeader, pbHeader2, cbHeader * sizeof( DRM_WCHAR ) );

    dasstrHeader1.m_cch = cbHeader;

    DRM_UTL_PromoteASCIItoUNICODE( (const DRM_CHAR *)pbHeader,
                                  &dasstrHeader1,
                                  ( DRM_STRING * )&dstrHeader );

    ChkDR( Drm_Content_SetProperty( AppContext,
                                    DRM_CSP_AUTODETECT_HEADER,
                                    DRM_PB_DSTR( &dstrHeader ),
                                    DRM_CB_DSTR( &dstrHeader ) ) );

    dr = Drm_LicenseAcq_GenerateChallenge( AppContext,
                                           (const DRM_CONST_STRING **)rgpdstrRights,
                                           DRM_NO_OF (rgpdstrRights),
                                           NULL,
                                           NULL,
                                           0,
                                           rgchURL,
                                           &cchURL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           &cbChallenge,
                                           &poBatchID);

    if ( dr != DRM_E_BUFFERTOOSMALL )
    {
        if (dr!= DRM_SUCCESS)
       {
            ChkDR( dr );
        }
        else
        {
            ChkDR( DRM_E_TEST_UNEXPECTED_OUTPUT);
        }
    }

    ChkBOOL( cchURL <= 1024, DRM_E_FAIL );

    ChkMem( pbChallenge = ( DRM_BYTE * )Oem_MemAlloc( cbChallenge ) );

    ChkDR( Drm_LicenseAcq_GenerateChallenge( AppContext,
                                             (const DRM_CONST_STRING **)rgpdstrRights,
                                             DRM_NO_OF (rgpdstrRights),
                                             NULL,
                                             NULL,
                                             0,
                                             rgchURL,
                                             &cchURL,
                                             NULL,
                                             NULL,
                                             pbChallenge,
                                             &cbChallenge,
                                             &poBatchID) );

    *ppbChallenge = pbChallenge;
    *pcbChallenge = cbChallenge;

ErrorExit:

    SAFE_OEM_FREE( pbHeader2 );
    return dr;
}

DRM_RESULT DRM_CALL GenerateLicenseAck(
    DRM_APP_CONTEXT *f_poAppContext,
    DRM_LICENSE_RESPONSE *f_poLicResponse,
    DRM_BYTE **f_ppbAcknowledgement,
    DRM_DWORD *f_pcbAcknowledgement )
{
    DRM_RESULT dr = DRM_SUCCESS;

    ChkArg( f_poAppContext != NULL );
    ChkArg( f_poLicResponse != NULL );
    ChkArg( f_ppbAcknowledgement != NULL );
    ChkArg( f_pcbAcknowledgement != NULL );

    dr = Drm_LicenseAcq_GenerateAck( f_poAppContext,
                                     f_poLicResponse,
                                     NULL,
                                     f_pcbAcknowledgement);

    if ( dr == DRM_E_BUFFERTOOSMALL )
    {
        ChkMem( *f_ppbAcknowledgement =
                    (DRM_BYTE *)Oem_MemAlloc( *f_pcbAcknowledgement ) );

        ChkDR( Drm_LicenseAcq_GenerateAck( f_poAppContext,
                                           f_poLicResponse,
                                           *f_ppbAcknowledgement,
                                           f_pcbAcknowledgement ) );
    }
    else
    {
        goto ErrorExit;
    }

ErrorExit:

    return dr;
}

static inline void stringToWString(DRM_WCHAR *dst, char *src)
{
    while ((*dst++ = *src++));
}

static DRM_RESULT doLicensing(const char *la_url)
{
    DRM_RESULT dr = DRM_SUCCESS;
    logd("%s %d, dr: %08x", __func__, __LINE__, dr);
    DRM_APP_CONTEXT *AppContext = NULL;
    DRM_BYTE *pbOpaqueBuffer = NULL;
    DRM_BYTE *pbRevocationBuffer = NULL;
    DRM_LICENSE_RESPONSE oLicResponse;// = { eUnknownProtocol };

    DRM_BYTE *pbHeader = NULL;
    DRM_DWORD cbHeader = 0;
    DRM_BYTE *pbChallenge1 = NULL;
    DRM_DWORD cbChallenge1 = 0;
    DRM_BYTE *pbChallenge2 = NULL;
    DRM_DWORD cbChallenge2 = 0;
    DRM_BYTE *pbResponse1 = NULL;
    DRM_DWORD cbResponse1 = 0;
    DRM_BYTE *pbResponse2 = NULL;
    DRM_DWORD cbResponse2 = 0;

    int i;
    logd("%s %d", __func__, __LINE__);
    char *pchar;
    short *pshort;
    // chl1.dat, mode = w+b
    char chl1[] = PLAYREADY_TMPFILE_DIR "chl1.dat";
    DRM_WCHAR wchl1[sizeof(chl1)] = {0};
    stringToWString(wchl1, chl1);
    const DRM_CONST_STRING strchl1 = DRM_CREATE_DRM_STRING(wchl1);

    // chl2.dat, mode = w+b
    char chl2[] = PLAYREADY_TMPFILE_DIR "chl2.dat";
    DRM_WCHAR wchl2[sizeof(chl2)] = {0};
    stringToWString(wchl2, chl2);
    const DRM_CONST_STRING strchl2 = DRM_CREATE_DRM_STRING(wchl2);

    // rsp1.dat, mode = w+b
    char rsp1[] = PLAYREADY_TMPFILE_DIR "rsp1.dat";
    DRM_WCHAR wrsp1[sizeof(rsp1)] = {0};
    stringToWString(wrsp1, rsp1);
    const DRM_CONST_STRING strrsp1 = DRM_CREATE_DRM_STRING(wrsp1);

    // rsp2.dat, mode = w+b
    char rsp2[] = PLAYREADY_TMPFILE_DIR "rsp2.dat";
    DRM_WCHAR wrsp2[sizeof(rsp2)] = {0};
    stringToWString(wrsp2, rsp2);
    const DRM_CONST_STRING strrsp2 = DRM_CREATE_DRM_STRING(wrsp2);

    // sstr.xml
    char pin[] = PLAYREADY_TMPFILE_DIR "sstr.xml";
    DRM_WCHAR wpin[sizeof(pin)] = {0};
    stringToWString(wpin, pin);
    DRM_CONST_STRING strin = DRM_CREATE_DRM_STRING(wpin);

    // pr.hds
    char prhds[] = PLAYREADY_TMPFILE_DIR "pr.hds";
    DRM_WCHAR wprhds[sizeof(prhds)] = {0};
    stringToWString(wprhds, prhds);
    const DRM_CONST_STRING strhds = DRM_CREATE_DRM_STRING(wprhds);


    // response.xml
    char res[] = PLAYREADY_TMPFILE_DIR "response.xml";
    DRM_WCHAR wres[sizeof(res)] = {0};
    stringToWString(wres, res);
    DRM_CONST_STRING strresponse = DRM_CREATE_DRM_STRING(wres);

    memset(&oLicResponse, 0, sizeof(oLicResponse));
    oLicResponse.m_eType = eUnknownProtocol;

    SetDbgAnalyzeFunction(DebugAnalyze);

    logd("%s %d", __func__, __LINE__);
    ChkDR( DRMTOOLS_DrmInitializeWithOpaqueBuffer(NULL, &strhds, TOOLS_DRM_BUFFER_SIZE, &pbOpaqueBuffer, &AppContext) );
    logd("%s %d", __func__, __LINE__);
    if( DRM_REVOCATION_IsRevocationSupported() )
    {
        ChkMem( pbRevocationBuffer = ( DRM_BYTE * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE ) );

        ChkDR( Drm_Revocation_SetBuffer( AppContext,
                                         pbRevocationBuffer,
                                         REVOCATION_BUFFER_SIZE ) );
    }

    logd("%s %d", __func__, __LINE__);
    ChkDR( LoadFile(&strin, &pbHeader, &cbHeader) );

    logd("%s %d", __func__, __LINE__);
    ChkDR( GenerateLicenseChallege(AppContext, pbHeader, cbHeader, &pbChallenge1, &cbChallenge1) );
    logd("%s %d", __func__, __LINE__);
    ChkDR( DumpData(&strchl1, pbChallenge1, cbChallenge1) );
    logd("%s %d", __func__, __LINE__);

    ChkDR( DRM_TOOLS_NETIO_SendData( la_url,
                                     eDRM_TOOLS_NET_LICGET,
                                     pbChallenge1,
                                     cbChallenge1,
                                     &pbResponse1,
                                     &cbResponse1) );
    logd("%s %d", __func__, __LINE__);
    ChkDR( DumpData(&strrsp1, pbResponse1, cbResponse1) );
    logd("%s %d", __func__, __LINE__);

    oLicResponse.m_dwResult = DRM_SUCCESS;
    ChkDR( Drm_LicenseAcq_ProcessResponse( AppContext,
                                           DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
                                           pbResponse1,
                                           cbResponse1,
                                           &oLicResponse ) );
    logd("%s %d", __func__, __LINE__);
    ChkDR( DumpData(&strresponse, pbResponse1, cbResponse1) );
    logd("%s %d result: %08x", __func__, __LINE__, oLicResponse.m_dwResult);
    if (!DRM_SUCCEEDED(oLicResponse.m_dwResult)) {
        ChkDR(DRM_E_FAIL);
    } else {
        logd("transid:%d\n", oLicResponse.m_cbTransactionID);
        if (oLicResponse.m_cbTransactionID > 0) {
            ChkDR( GenerateLicenseAck( AppContext,
                                       &oLicResponse,
                                       &pbChallenge2,
                                       &cbChallenge2) );
    logd("%s %d", __func__, __LINE__);
            ChkDR( DumpData(&strchl2, pbChallenge2, cbChallenge2) );
    logd("%s %d", __func__, __LINE__);
            ChkDR( DRM_TOOLS_NETIO_SendData( la_url,
                                             eDRM_TOOLS_NET_LICACK,
                                             pbChallenge2,
                                             cbChallenge2,
                                             &pbResponse2,
                                             &cbResponse2) );
    logd("%s %d", __func__, __LINE__);
            ChkDR( DumpData(&strrsp2, pbResponse2, cbResponse2) );
    logd("%s %d", __func__, __LINE__);
            DRM_RESULT dr1 = DRM_SUCCESS;
            ChkDR( Drm_LicenseAcq_ProcessAckResponse( AppContext,
                                                      pbResponse2,
                                                      cbResponse2,
                                                      &dr1 ) );
    logd("%s %d", __func__, __LINE__);
            ChkDR(dr1);
    logd("%s %d", __func__, __LINE__);

        }
    }


    logd("%s %d", __func__, __LINE__);

ErrorExit:

    SAFE_OEM_FREE(pbRevocationBuffer);
    DRMTOOLS_DrmUninitializeWithOpaqueBuffer( &pbOpaqueBuffer, &AppContext );
    SAFE_OEM_FREE(pbHeader);
    SAFE_OEM_FREE(pbChallenge1);
    SAFE_OEM_FREE(pbChallenge2);
    SAFE_OEM_FREE(pbResponse1);
    SAFE_OEM_FREE(pbResponse2);
    logd("end, dr=0x%x\n", (int)dr);
    return dr;
}

#define FUN_PROCESS_LICENSE 1
status_t PlayReady_Drm_Invoke(const Parcel &request, Parcel *reply)
{
    int func;
    int ret = request.readInt32(&func);
    if(ret != OK)
        return ret;
    switch (func)
    {
    case FUN_PROCESS_LICENSE:
        {
            String8 wrmheader(request.readString8());
            String8 la_url(request.readString8());
            logd("wrmheader=%s", wrmheader.string());
            logd("la_url=%s", la_url.string());
            // save to file, will be used in decryption
            FILE *fp = fopen(PLAYREADY_TMPFILE_DIR "sstr.xml", "wb");
            if (fp) {
                fwrite(wrmheader.string(), 1, wrmheader.length(), fp);
                fclose(fp);
            } else {
                loge("write file failed: %s", strerror(errno));
            }
            ret = doLicensing(la_url.string());
            reply->writeInt32(ret);
        }
        break;
    default:
        break;
    }
    return OK;
}

#else

status_t PlayReady_Drm_Invoke(const Parcel &request, Parcel *reply)
{
    reply->writeInt32(-1);
    return 0;
}

#endif
