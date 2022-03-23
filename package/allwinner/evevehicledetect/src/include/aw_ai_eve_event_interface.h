#ifndef _AW_AI_EVE_EVENT_INTERFACE_H
#define _AW_AI_EVE_EVNET_INTERFACE_H

#include "aw_ai_eve_type.h"

//----------------------------------------------
// EXPORTS and IMPORTS definition
//----------------------------------------------
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined CVE_API_EXPORTS
    #define AW_EXPORTS __declspec(dllexport)
#else
    #define AW_EXPORTS
#endif

//event type
typedef enum
{
	AW_AI_EVE_EVENT_FACEDETECT = 3001,  //face detect
	AW_AI_EVE_EVENT_HEADDETECT = 3002,  //reserved
}AW_AI_EVE_EVENT_TYPE_E;

//face detect parameter
typedef struct _AW_AI_EVE_EVENT_FACEDET_PARAM_S
{
	AW_ROI_SET_S sRoiSet;          //facedetect roi, support maximize 8 rois
	AW_S32     s32OverLapCoeff;    //overlap coeff, [1, 100]%, default: [20]%
	AW_S32     s32MinFaceSize;     //minimize face  size,[20]pixel
	AW_S32     s32MergeThreshold;  //merger threshold, [2, 5], default: 3
	AW_S32     s32MaxFaceNum;      //maximize face num, [1, 128], default: 128
	AW_S32     s32ClassifyFlag;    //the second Classify flag, 0-off, 1-on, reseverd, default: 0
	//AW_S32     s32MinSnapFaceSize; //the minimize snap face size, default: 20
	//AW_S32     s32MaxSnapFaceSize; //the maximum  snap face size, default: 120
}AW_AI_EVE_EVENT_FACEDET_PARAM_S, *lpAW_AI_EVE_EVENT_FACEDET_PARAM_S;

//eve result
typedef struct _AW_AI_EVE_EVENT_RESULT_S
{
	AW_EVETARGET_SET_S  sTarget;
	AW_EVENT_SET_S      sEvent;
}AW_AI_EVE_EVENT_RESULT_S, *lpAW_AI_EVE_EVENT_RESULT_S;

//interface
#ifdef __cplusplus
extern "C"{
#endif

//! \ingroup group_eve_application
//! \{
//! \brief ��ѯEVE�㷨�汾
//! \param pVersion     [out] �汾�ַ������ַ�������������������32
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS  AW_STATUS_E AW_AI_EVE_Event_GetAlgoVersion( AW_S8 *pVersion );

//! \brief �����㷨ģ�鲢��ʼ����
//! \pParam pEVECtrl [in] EVE���ò������ο�\ref AW_AI_EVE_CTRL_S
//! \return             ģ������
AW_EXPORTS  AW_HANDLE   AW_AI_EVE_Event_Init(AW_AI_EVE_CTRL_S *pEVECtrl);

//! \brief ��ֹ�㷨ģ�鲢�ͷ��ڴ�
//! \param hHandle      [in] ģ������
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_UnInit( AW_HANDLE hHandle);

//! \brief ����һ֡ͼ�񣬲���ȡ�����������ʺ���Ƶ����
//! \param hHandle      [in] ģ������
//! \param pstImage     [in] ������������ͼ�񣬲ο�\ref AW_IMAGE_S
//! \param u32TimeStamp [in] ����ͼ����ʱ��������ͼ��֡Ϊ��λ
//! \param pstResult    [out] �����������ο�\ref AW_AI_EVE_EVENT_RESULT_S
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_Process( AW_HANDLE hHandle, AW_IMAGE_S *pstImage, AW_S64 u32TimeStamp, AW_AI_EVE_EVENT_RESULT_S *pstResult );

//! \brief ����һ��ͼ�񣬲���ȡ�����������ʺϵ���ͼƬ����
//! \param hHandle      [in] ģ������
//! \param pstImage     [in] ������������ͼ�񣬲ο�\ref AW_IMAGE_S
//! \param pstResult    [out] �����������ο�\ref AW_AI_EVE_EVENT_RESULT_S
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_ProcessImage( AW_HANDLE hHandle, AW_IMAGE_S *pstImage, AW_AI_EVE_EVENT_RESULT_S *pstResult );


//! \brief �����¼������ṹ��
//! \param hHandle      [in] ģ������
//! \param eventType    [in] �¼����ͣ��ο�\ref AW_AI_EVE_EVENT_TYPE_E
//! \param pEventParam  [in] �¼��ṹ�����ã��ο�\ref ��ͬ���¼��ṹ������
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_SetEventParam( AW_HANDLE hHandle, AW_AI_EVE_EVENT_TYPE_E eventType, AW_PVOID pEventParam);

//! \brief ����EVE����
//! \param hHandle      [in] ģ������
//! \param pEveParam    [in] ����EVE�������ο�\ref AW_AI_EVE_PARAMETER_S
//! \return             ����״̬���ο�\ref AW_STATUS_E
//AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_SetEveParam( AW_HANDLE hHandle, AW_AI_EVE_KERNEL_CTRL_S *pEveParam);

//! \brief ����EVE����Դ��ַ
//! \param hHandle      [in] ģ������
//! \param pSourceAddr  [in] pSourceAddr,����Դ��ַ
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_SetEveSourceAddress( AW_HANDLE hHandle, void *pSourceAddr);

//! \brief ����EVE�ں�DMA������EVE���ӵ�ִ��˳��
//! \param hHandle      [in] ģ������
//! \param dmaMode      [in] DMA��EVE����ִ��˳����0-�첽��1-ͬ��
//! \return             ����״̬���ο�\ref AW_STATUS_E
AW_EXPORTS AW_STATUS_E  AW_AI_EVE_Event_SetEveDMAExecuteMode( AW_HANDLE hHandle, AW_S32 dmaMode);

//AW_EXPORTS  AW_STATUS_E  AW_AI_EVE_Event_SetDMAParam(AW_HANDLE hHandle, AW_AI_EVE_DMA_CTRL_S* pDmaCtrl);


//! \brief ��ȡ�����Ĵ���������
//! \param hHandle      [in] ģ������
//! \return             �㷨�����룬�ο�\ref AW_S32
AW_EXPORTS AW_S32  AW_AI_EVE_Event_GetLastError( AW_HANDLE hHandle );

#ifdef __cplusplus
}
#endif


#endif // _AW_AI_EVE_INTERFACE_H
