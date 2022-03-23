/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : MuxerWriter.c
 * Description : MuxerWriter
 * History :
 *
 */

#include "MuxerWriter.h"

int __CdxRead(CdxWriterT *writer, void *buf, int size)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        loge("buf is NULL\n");
        return -1;
    }
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        ret = read(muxer_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            loge("CdxRead read failed\n");
            return -1;
        }
    }
    else if (muxer_writer->file_mode == FP_FILE_MODE)
    {
        ret = fread(pbuf, 1, size, muxer_writer->fp_file);
        if (ret < 0)
        {
            loge("CdxRead fread failed\n");
            return -1;
        }
    }
    return ret;
}

int __CdxWrite(CdxWriterT *writer, void *buf, int size)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        loge("buf is NULL\n");
        return -1;
    }
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        ret = write(muxer_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            loge("CdxWrite write failed\n");
            return -1;
        }
    }
    else if (muxer_writer->file_mode == FP_FILE_MODE)
    {
        ret = fwrite(pbuf, 1, size, muxer_writer->fp_file);
        if (ret < 0)
        {
            loge("CdxWrite fwrite failed\n");
            return -1;
        }
    }
    return ret;
}

long __CdxSeek(CdxWriterT *writer, long moffset, int mwhere)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    long ret = 0;
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        ret = lseek(muxer_writer->fd_file, moffset, mwhere);
        if (ret < 0)
        {
            loge("CdxSeek lseek failed\n");
            return -1;
        }
    }
    else if(muxer_writer->file_mode == FP_FILE_MODE)
    {
        ret = fseek(muxer_writer->fp_file, moffset, mwhere);
        if (ret < 0)
        {
            loge("CdxSeek fseek failed\n");
            return -1;
        }
    }
    return ret;
}

long __CdxTell(CdxWriterT *writer)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        return CdxWriterSeek(writer, 0, SEEK_CUR);
    }
    else if (muxer_writer->file_mode == FP_FILE_MODE)
    {
        return ftell(muxer_writer->fp_file);
    }
    return 0;
}

int MWOpen(CdxWriterT *writer)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    if (muxer_writer->file_path == NULL)
    {
        loge("muxer_writer->file_path is NULL\n");
        return -1;
    }
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        muxer_writer->fd_file = open(muxer_writer->file_path, O_RDWR | O_CREAT, 666);
        if (muxer_writer->fd_file < 0)
        {
            loge("muxer_writer->fd_file open failed\n");
            return -1;
        }
    }
    else if (muxer_writer->file_mode == FP_FILE_MODE)
    {
        muxer_writer->fp_file = fopen(muxer_writer->file_path, "wb+");
        if (muxer_writer->fp_file == NULL)
        {
            loge("muxer_writer->fp_file fopen failed\n");
            return -1;
        }
    }
    return 0;
}

int MWClose(CdxWriterT *writer)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    int ret = 0;
    if (muxer_writer->file_mode == FD_FILE_MODE)
    {
        ret = close(muxer_writer->fd_file);
    }
    else if (muxer_writer->file_mode == FP_FILE_MODE)
    {
        ret = fclose(muxer_writer->fp_file);
    }
    if (ret < 0)
    {
        loge("__CdxClose() failed\n");
    }
    return ret;
}

static int __CdxClose(CdxWriterT *writer)
{
    return 0;
}

static struct CdxWriterOps writerOps =
{
    .read   = __CdxRead,
    .write  = __CdxWrite,
    .seek   = __CdxSeek,
    .tell   = __CdxTell,
    .close  = __CdxClose
};

CdxWriterT *CdxWriterCreat()
{
    MuxerWriterT *muxer_writer = NULL;

    if ((muxer_writer = (MuxerWriterT*)malloc(sizeof(MuxerWriterT))) == NULL)
    {
        loge("CdxWriter creat failed\n");
        return NULL;
    }
    memset(muxer_writer, 0, sizeof(MuxerWriterT));
    if ((muxer_writer->file_path = malloc(128)) == NULL)
    {
        loge("MyWriter->file_path malloc failed");
        free(muxer_writer);
        return NULL;
    }
    memset(muxer_writer->file_path, 0, 128);

    muxer_writer->cdx_writer.ops = &writerOps;

    return &muxer_writer->cdx_writer;
}

void CdxWriterDestroy(CdxWriterT *writer)
{
    MuxerWriterT *muxer_writer = (MuxerWriterT*)writer;
    if (muxer_writer == NULL)
    {
        logw("writer is NULL, no need to be destroyed\n");
        return;
    }
    if (muxer_writer->file_path)
    {
        free(muxer_writer->file_path);
        muxer_writer->file_path = NULL;
    }
    free(muxer_writer);
}
