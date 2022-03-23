#ifndef __RECODER_WRITER_NEW_H__
#define __RECODER_WRITER_NEW_H__

#include "CdxWriter.h"

#define FILE_PATH_MAX_LEN 128

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
CdxWriterT *CdxWriterCreat(void);

#endif