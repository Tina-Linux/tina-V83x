/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : RecoderWriter.h
 * Description : RecoderWriter
 * History :
 *
 */


#ifndef __RECODER_WRITER_H__
#define __RECODER_WRITER_H__

#include "CdxWriter.h"

typedef enum CDX_FILE_MODE
{
    FD_FILE_MODE,
    FP_FILE_MODE
}CDX_FILE_MODE;

typedef struct RecoderWriter RecoderWriterT;
struct RecoderWriter
{
    CdxWriterT cdx_writer;
    int        file_mode;
    int        fd_file;
    FILE       *fp_file;
    char       *file_path;
};

int RWOpen(CdxWriterT *writer);
int RWClose(CdxWriterT *writer);
CdxWriterT *CdxWriterCreat(char* pUrl);

#endif
