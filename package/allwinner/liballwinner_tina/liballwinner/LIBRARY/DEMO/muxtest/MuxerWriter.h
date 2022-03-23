#ifndef __MUXER_WRITER_H__
#define __MUXER_WRITER_H__

#include "CdxWriter.h"

typedef enum CDX_FILE_MODE
{
    FD_FILE_MODE,
    FP_FILE_MODE
}CDX_FILE_MODE;

typedef struct MuxerWriter MuxerWriterT;
struct MuxerWriter
{
    CdxWriterT cdx_writer;
    int        file_mode;
    int        fd_file;
    FILE       *fp_file;
    char       *file_path;
};

int MWOpen(CdxWriterT * writer);
int MWClose(CdxWriterT * writer);

#endif