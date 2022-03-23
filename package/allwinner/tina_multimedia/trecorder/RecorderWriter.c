/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : RecoderWriter.c
 * Description : RecoderWriter
 * History :
 *
 */

#include "RecorderWriter.h"
#include "TRlog.h"

int __CdxRead(CdxWriterT *writer, void *buf, int size)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        TRerr("[%s] buf is NULL\n", __func__);
        return -1;
    }
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        ret = read(recoder_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            TRerr("[%s] CdxRead read failed\n", __func__);
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fread(pbuf, 1, size, recoder_writer->fp_file);
        if (ret < 0)
        {
            TRerr("[%s] CdxRead fread failed\n", __func__);
            return -1;
        }
    }
    return ret;
}

int __CdxWrite(CdxWriterT *writer, void *buf, int size)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        TRerr("[%s] buf is NULL\n", __func__);
        return -1;
    }

    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
#ifdef DETECT_SD_CARD
	if (access("/dev/mmcblk0", F_OK) < 0) {
	    TRerr("[%s] TF Card has been removed!!!!!!!!\n", __func__);
	    return -1;
	}
	    //TRlog(TR_LOG_MUXER, "detect   dev/mmcblk0!!!!!!!!\n");
#endif
        ret = write(recoder_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            TRerr("[%s] CdxWrite write failed\n", __func__);
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
#ifdef DETECT_SD_CARD
	if (access("/dev/mmcblk0", F_OK) < 0) {
	    TRerr("[%s] TF Card has been removed!!!!!!!!\n", __func__);
	    return -1;
	}
	    //TRlog(TR_LOG_MUXER, "detect   dev/mmcblk0!!!!!!!!\n");
#endif
        ret = fwrite(pbuf, 1, size, recoder_writer->fp_file);
        if (size && ret != size)
        {
            TRerr("[%s] CdxWrite fwrite failed\n", __func__);
            return -1;
        }
    }
    return ret;
}

long __CdxSeek(CdxWriterT *writer, long moffset, int mwhere)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    long ret = 0;
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        ret = lseek(recoder_writer->fd_file, moffset, mwhere);
        if (ret < 0)
        {
            TRerr("[%s] CdxSeek lseek failed\n", __func__);
            return -1;
        }
    }
    else if(recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fseek(recoder_writer->fp_file, moffset, mwhere);
        if (ret < 0)
        {
            TRerr("[%s] CdxSeek fseek failed\n", __func__);
            return -1;
        }
    }
    return ret;
}

long __CdxTell(CdxWriterT *writer)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        return CdxWriterSeek(writer, 0, SEEK_CUR);
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        return ftell(recoder_writer->fp_file);
    }
    return 0;
}

int RWOpen(CdxWriterT *writer)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    if (recoder_writer->file_path == NULL)
    {
        TRerr("[%s] recoder_writer->file_path is NULL\n", __func__);
        return -1;
    }
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        recoder_writer->fd_file = open(recoder_writer->file_path, O_RDWR | O_CREAT, 666);
        if (recoder_writer->fd_file < 0)
        {
            TRerr("[%s] recoder_writer->fd_file open %s failed\n", __func__, recoder_writer->file_path);
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        recoder_writer->fp_file = fopen(recoder_writer->file_path, "wb+");
        if (recoder_writer->fp_file == NULL)
        {
            TRerr("[%s] recoder_writer->fp_file fopen %s failed\n", __func__, recoder_writer->file_path);
            return -1;
        }
    }
    return 0;
}

int RWClose(CdxWriterT *writer)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    int ret = 0;
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        fsync(recoder_writer->fd_file);
        ret = close(recoder_writer->fd_file);
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        fflush(recoder_writer->fp_file);
        fsync(fileno(recoder_writer->fp_file));
        ret = fclose(recoder_writer->fp_file);
    }
    if (ret < 0)
    {
        TRerr("[%s] __CdxClose() failed\n", __func__);
    }
    return ret;
}

static int __CdxClose(CdxWriterT *writer)
{
    return 0;
}

static struct CdxWriterOps writeOps =
{
    .read = __CdxRead,
    .write = __CdxWrite,
    .seek = __CdxSeek,
    .tell = __CdxTell,
    .close = __CdxClose
};

CdxWriterT *CdxWriterCreat(void)
{
    RecoderWriterT *recoder_writer = NULL;

    if ((recoder_writer = (RecoderWriterT*)malloc(sizeof(RecoderWriterT))) == NULL)
    {
        TRerr("[%s] CdxWriter creat failed\n", __func__);
        return NULL;
    }
    memset(recoder_writer, 0, sizeof(RecoderWriterT));
    if ((recoder_writer->file_path = (char *)malloc(FILE_PATH_MAX_LEN)) == NULL)
    {
        TRerr("[%s] MyWriter->file_path malloc failed\n", __func__);
        free(recoder_writer);
        return NULL;
    }
    memset(recoder_writer->file_path, 0, FILE_PATH_MAX_LEN);

    recoder_writer->cdx_writer.ops = &writeOps;

    return &recoder_writer->cdx_writer;
}

void CdxWriterDestroy(CdxWriterT *writer)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    if (recoder_writer == NULL)
    {
        TRerr("[%s] writer is NULL, no need to be destroyed\n", __func__);
        return;
    }
    if (recoder_writer->file_path)
    {
        free(recoder_writer->file_path);
        recoder_writer->file_path = NULL;
    }
    free(recoder_writer);
}
