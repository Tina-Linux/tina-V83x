#include "RecoderWriter.h"

int __CdxRead(CdxWriterT *writer, void *buf, int size)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        loge("buf is NULL\n");
        return -1;
    }
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        ret = read(recoder_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            loge("CdxRead read failed\n");
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fread(pbuf, 1, size, recoder_writer->fp_file);
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
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    unsigned char *pbuf = (unsigned char *)buf;
    int ret = 0;
    if (pbuf == NULL)
    {
        loge("buf is NULL\n");
        return -1;
    }
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        ret = write(recoder_writer->fd_file, pbuf, size);
        if (ret < 0)
        {
            loge("CdxWrite write failed\n");
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fwrite(pbuf, 1, size, recoder_writer->fp_file);
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
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    long ret = 0;
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        ret = lseek(recoder_writer->fd_file, moffset, mwhere);
        if (ret < 0)
        {
            loge("CdxSeek lseek failed\n");
            return -1;
        }
    }
    else if(recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fseek(recoder_writer->fp_file, moffset, mwhere);
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
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        return writer->cdxSeek(writer, 0, SEEK_CUR);
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
        loge("recoder_writer->file_path is NULL\n");
        return -1;
    }
    if (recoder_writer->file_mode == FD_FILE_MODE)
    {
        recoder_writer->fd_file = open(recoder_writer->file_path, O_RDWR | O_CREAT, 666);
        if (recoder_writer->fd_file < 0)
        {
            loge("recoder_writer->fd_file open failed\n");
            return -1;
        }
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        recoder_writer->fp_file = fopen(recoder_writer->file_path, "wb+");
        if (recoder_writer->fp_file == NULL)
        {
            loge("recoder_writer->fp_file fopen failed\n");
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
        ret = close(recoder_writer->fd_file);
    }
    else if (recoder_writer->file_mode == FP_FILE_MODE)
    {
        ret = fclose(recoder_writer->fp_file);
    }
    if (ret < 0)
    {
        loge("__CdxClose() failed\n");
    }
    return ret;
}

CdxWriterT *CdxWriterCreat()
{
    RecoderWriterT *recoder_writer = NULL;
    CdxWriterT *p_writer = NULL;
    if ((recoder_writer = (RecoderWriterT*)malloc(sizeof(RecoderWriterT))) == NULL)
    {
        loge("CdxWriter creat failed\n");
        return NULL;
    }
    memset(recoder_writer, 0, sizeof(RecoderWriterT));
    if ((recoder_writer->file_path = malloc(128)) == NULL)
    {
        loge("MyWriter->file_path malloc failed");
        free(recoder_writer);
        return NULL;
    }
    memset(recoder_writer->file_path, 0, 128);
    p_writer = (CdxWriterT*)recoder_writer;
    p_writer->cdxRead = __CdxRead;
    p_writer->cdxWrite = __CdxWrite;
    p_writer->cdxSeek = __CdxSeek;
    p_writer->cdxTell = __CdxTell;
    return p_writer;
}

void CdxWriterDestroy(CdxWriterT *writer)
{
    RecoderWriterT *recoder_writer = (RecoderWriterT*)writer;
    if (recoder_writer == NULL)
    {
        logw("writer is NULL, no need to be destroyed\n");
        return;
    }
    if (recoder_writer->file_path)
    {
        free(recoder_writer->file_path);
        recoder_writer->file_path = NULL;
    }
    free(recoder_writer);
}
