/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : CdcMalloc.h
* Description :
* History :
*   Author  :
*   Date    : 2018/07/03
*   Comment :
*
*
*/

#ifndef  CDC_MALLOC_H
#define  CDC_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#define malloc CdcMalloc
#define calloc CdcCalloc
#define free   CdcFree

#define CdcMalloc(size)               cdc_malloc (size, __FUNCTION__, __LINE__)
#define CdcCalloc(elements, size)     cdc_calloc (elements, size, __FUNCTION__, __LINE__)
#define CdcFree(mem_ref)              cdc_free(mem_ref)

void *cdc_malloc(unsigned int size, const char * file, unsigned int line);
void *cdc_calloc(unsigned int elements, unsigned int size, const char * file, unsigned int line);
void cdc_free(void * mem_ref);

void CdcMallocReport(void);
int CdcMallocGetInfo(char*  pDst, int nDstBufsize);
int CdcMallocCheckFlag();

#ifdef __cplusplus
}
#endif


#endif
