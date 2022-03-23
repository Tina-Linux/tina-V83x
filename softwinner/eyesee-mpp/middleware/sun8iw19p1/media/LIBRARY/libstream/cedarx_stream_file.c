/*******************************************************************************
--                                                                            --
--                    CedarX Multimedia Framework                             --
--                                                                            --
--          the Multimedia Framework for Linux/Android System                 --
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                         Softwinner Products.                               --
--                                                                            --
--                   (C) COPYRIGHT 2011 SOFTWINNER PRODUCTS                   --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
*******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "cedarx_stream_file"
#include <utils/plat_log.h>

#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cedarx_stream.h"
#include "cedarx_stream_file.h"
//#include <tsemaphore.h>
#include <SystemBase.h>
#include <dup2SeldomUsedFd.h>

#include <ConfigOption.h>


//#include <CDX_SystemBase.h>
#if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
static const int DIRECTIO_UNIT_SIZE = (64*1024);   //cluster is 64*1024, align by cluster unit will has the max speed. (512)
static const int DIRECTIO_USER_MEMORY_ALIGN = (512);
#endif


static pthread_mutex_t gFallocateLock;

static void cedarx_stream_file_init(void)  __attribute__((constructor));
static void cedarx_stream_file_init(void)
{
    int err = pthread_mutex_init(&gFallocateLock, NULL);
    if (err != 0) 
    {
        aloge("fatal error! pthread mutex init fail!");
    }
}

static char *generateFilepathFromFd(const int fd)
{
    char fdPath[1024];
    char absoluteFilePath[1024];
    int ret;
    if(fd < 0)
    {
        aloge("fatal error! fd[%d] wrong", fd);
        return NULL;
    }

    snprintf(fdPath, sizeof(fdPath), "/proc/self/fd/%d", fd);
    ret = readlink(fdPath, absoluteFilePath, 1024 - 1);
    if (ret == -1)
    {
        aloge("fatal error! readlink[%s] failure, errno(%s)", fdPath, strerror(errno));
        return NULL;
    }
    absoluteFilePath[ret] = '\0';
    alogd("readlink[%s], filePath[%s]", fdPath, absoluteFilePath);
    return strdup(absoluteFilePath);
}

int cdx_seek_stream_file(struct cdx_stream_info *stream, cdx_off_t offset, int whence)
{
//    int fd = fileno(stream->file_handle);
//    int seek_ret;
//	cdx_off_t seekResult = lseek64(fd, offset, whence);
//    seek_ret = (seekResult == -1) ? -1 : 0;
//    return seek_ret;
    return fseeko(stream->file_handle, offset, whence);
}

cdx_off_t cdx_tell_stream_file(struct cdx_stream_info *stream)
{
//    int fd = fileno(stream->file_handle);
//    cdx_off_t pos = lseek64(fd, 0, SEEK_CUR);
    cdx_off_t pos = ftello(stream->file_handle);
    return pos;
}

ssize_t cdx_read_stream_file(void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
//    int fd = fileno(stream->file_handle);
//    int bytesToBeRead = size * nmemb;
//    int result = read(fd, ptr, bytesToBeRead);
//    if (result > 1) {
//    	result /= size;
//    }
//    return result;
    return fread(ptr, size, nmemb,stream->file_handle);
}

ssize_t cdx_write_stream_file(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
//    int fd = fileno(stream->file_handle);
//    int num_bytes_written = write(fd , ptr, (size * nmemb));
//    if (num_bytes_written != -1) {
//    	num_bytes_written /=size;
//    }
//    return num_bytes_written;
    //alogd("ptr[%p], size[%d]nmemb[%d]file_handle[%p]!", ptr, size, nmemb, stream->file_handle);
    return fwrite(ptr, size, nmemb,stream->file_handle);
    //alogd("writeNum[%d]", writeNum);
}

long long cdx_get_stream_size_file(struct cdx_stream_info *stream)
{
	long long size = -1;
	cdx_off_t curr_offset;

	if (stream->data_src_desc.stream_type == CEDARX_STREAM_NETWORK)
		return -1;

	if (stream->data_src_desc.source_type == CEDARX_SOURCE_FD) {
		if (stream->data_src_desc.ext_fd_desc.length >= 0) {
			size = stream->data_src_desc.ext_fd_desc.length;
			return size;
		}
	}
//    int fd = fileno(stream->file_handle);
//    curr_offset = lseek64(fd , 0, SEEK_CUR);
//	size = lseek64(fd, 0, SEEK_END);
//	lseek64(fd, curr_offset, SEEK_SET);
    curr_offset = ftello(stream->file_handle);
	fseeko(stream->file_handle, 0, SEEK_END);
	size = ftello(stream->file_handle);
	fseeko(stream->file_handle, curr_offset, SEEK_SET);
	return size;
}


int cdx_truncate_stream_file(struct cdx_stream_info *stream, cdx_off_t length)
{
    int ret;
    int nFd = fileno(stream->file_handle);
    ret = ftruncate(nFd, length);
    if(ret!=0)
    {
        aloge("fatal error! ftruncate fail");
    }
    return ret;
}

int cdx_fallocate_stream_file(struct cdx_stream_info *stream, int mode, int64_t offset, int64_t len)
{
    int ret = 0;
    int nFd = fileno(stream->file_handle);
    if(nFd >= 0)
    {
        pthread_mutex_lock(&gFallocateLock);
        int64_t tm1, tm2;
        tm1 = CDX_GetSysTimeUsMonotonic();
        if (fallocate(nFd, mode, offset, len) < 0)
        //if((ret = posix_fallocate(nFd, offset, len)) != 0)
        {
            aloge("Failed to fallocate size %lld, fd[%d], ret[%d](%s)", len, nFd, ret, strerror(errno));
            pthread_mutex_unlock(&gFallocateLock);
            return -1;
        }
        tm2 = CDX_GetSysTimeUsMonotonic();
        alogd("stream[%p] fallocate size(%lld)Byte, time(%lld)ms", stream, len, (tm2-tm1)/1000);
        pthread_mutex_unlock(&gFallocateLock);
        return 0;
    }
    else
    {
        aloge("fatal error! wrong fd[%d]", nFd);
        return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////
int cdx_seek_fd_file(struct cdx_stream_info *stream, cdx_off_t offset, int whence)
{
    alogv("stream %p, cur offset 0x%08llx, seek offset 0x%08llx, whence %d, length 0x%08llx", 
			stream, stream->fd_desc.cur_offset, offset, whence, stream->fd_desc.length);
    cdx_off_t seek_result;
	if(whence == SEEK_SET) {
		//set from start, add origial offset.
		offset += stream->fd_desc.offset;
	} else if(whence == SEEK_CUR) {
		//seek from current position, seek to cur offset at first.
		seek_result = lseek64(stream->fd_desc.fd, stream->fd_desc.cur_offset, SEEK_SET);
        if(-1 == seek_result)
        {
            aloge("fatal error! seek failed (%s)", strerror(errno));
            return -1;
        }
	} else {
      #if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
        if(stream->mIODirectionFlag)
        {
            //seek from backfront, no matter with current position and offset.
            if(stream->mFileEndOffset<stream->mFileSize)
            {
                aloge("fatal error! called seek when endOffset[%lld]!=fileSize[%lld]", stream->mFileEndOffset, stream->mFileSize);
                offset -= (stream->mFileSize - stream->mFileEndOffset);
            }
            else if (stream->mFileEndOffset == stream->mFileSize)
            {
            }
            else
            {
                aloge("fatal error! [%lld]>[%lld]", stream->mFileEndOffset, stream->mFileSize);
                assert(0);
            }
        }
      #endif
	}
#if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
    if(stream->mIODirectionFlag)
    {
        cdx_off_t   absSeekPos;
        if(whence == SEEK_SET)
        {
            absSeekPos = offset;
        }
        else if(whence == SEEK_CUR)
        {
            absSeekPos = offset + stream->fd_desc.cur_offset;
        }
        else
        {
            absSeekPos = offset + stream->mFileSize;
        }
        if(absSeekPos > stream->mFileSize)
        {
            //we use write to extend file size, to avoid linux kernel use cache.
            cdx_off_t nExtendSize = absSeekPos - stream->mFileSize;
            stream->mFtruncateFlag = 1;
            int64_t tm1, tm2;
            tm1 = CDX_GetSysTimeUsMonotonic();
            ssize_t wtSize = stream->write(NULL, 1, nExtendSize, stream);
            tm2 = CDX_GetSysTimeUsMonotonic();
            stream->mFtruncateFlag = 0;
            alogd("directIO seek extend [%lld]bytes, use [%lld]ms", nExtendSize, (tm2-tm1)/1000);
            if(wtSize == nExtendSize)
            {
                seek_result = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);
            }
            else
            {
                aloge("fatal error! directIO truncate fail, wtSize[%ld]!=extendSize[%lld]", wtSize, nExtendSize);
                seek_result = -1;
            }
        }
        else
        {
            seek_result = lseek64(stream->fd_desc.fd, offset, whence);
        }
    }
    else
    {
        seek_result = lseek64(stream->fd_desc.fd, offset, whence);
    }
#else
    seek_result = lseek64(stream->fd_desc.fd, offset, whence);
#endif
	
	int seek_ret = 0;
	if(seek_result == -1) 
    {
		aloge("fatal error! seek failed (%s)", strerror(errno));
		seek_ret = -1;
	} 
    else 
    {
		stream->fd_desc.cur_offset = seek_result < stream->fd_desc.offset
				? stream->fd_desc.offset : seek_result;
        if(seek_result < stream->fd_desc.offset)
        {
            aloge("fatal error! seek pos[%lld]<offset[%lld]", seek_result, stream->fd_desc.offset);
        }
	}
	return seek_ret;
}

cdx_off_t cdx_tell_fd_file(struct cdx_stream_info *stream)
{
    alogv("(f:%s, l:%d)");
	return stream->fd_desc.cur_offset - stream->fd_desc.offset;
}

ssize_t cdx_read_fd_file(void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
    alogv("(f:%s, l:%d)");
	//seek to where current actual pos is.
	ssize_t result = lseek64(stream->fd_desc.fd, stream->fd_desc.cur_offset, SEEK_SET);
	if(result == -1) {
        aloge("fatal error! seek fail!");
		return -1;
	}

	int bytesToBeRead = size * nmemb;
	alogv("stream %p, cur offset 0x%08llx, require bytes %d, length 0x%08llx",
			stream, stream->fd_desc.cur_offset, bytesToBeRead, stream->fd_desc.length);

    result = read(stream->fd_desc.fd, ptr, bytesToBeRead);
    stream->fd_desc.cur_offset = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);
    if(-1 == stream->fd_desc.cur_offset)
    {
        aloge("fatal error! seek fail!");
    }
    assert(stream->fd_desc.cur_offset >= stream->fd_desc.offset);

    if (result != -1) 
    {
    	result /= size;
    }
    else
    {
        aloge("fatal error! read [%d]bytes[%d] fail!", size*nmemb, result);
    }
	alogv("stream %p, cur offset 0x%08llx, read bytes %d, length 0x%08llx",
			stream, stream->fd_desc.cur_offset, result, stream->fd_desc.length);
    return result;
}

#if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
ssize_t cdx_write_fd_file(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
    alogv("(f:%s, l:%d)");
    off64_t result = lseek64(stream->fd_desc.fd, stream->fd_desc.cur_offset, SEEK_SET);
	if(result == -1) {
        aloge("fatal error! seek fail!");
		return -1;
	}
    if(NULL == stream->mpAlignBuf)
    {
        stream->mAlignBufSize = DIRECTIO_UNIT_SIZE;
        int ret = posix_memalign((void **)&stream->mpAlignBuf, 4096, stream->mAlignBufSize);
        if(ret!=0)
        {
            aloge("fatal error! malloc fail!");
            return -1;
        }
    }
    char *pCurPtr = (char*)ptr;
    off64_t nByteSize = size*nmemb;
    off64_t fileOffset = result;
    off64_t alignOffset;
    if(stream->mFtruncateFlag)
    {
        memset(stream->mpAlignBuf, 0xFF, stream->mAlignBufSize);
        fileOffset = lseek64(stream->fd_desc.fd, 0, SEEK_END);
        alogd("use write() to ftruncate file! curOffset[%lld], endOffset[%lld], curSize[%lld], extend[%lld]bytes", 
            __FUNCTION__, __LINE__, fileOffset, stream->mFileEndOffset, stream->mFileSize, nByteSize);
        stream->mFileEndOffset = stream->mFileSize;

        off64_t nAlignByteSize = (nByteSize/DIRECTIO_UNIT_SIZE)*DIRECTIO_UNIT_SIZE;
        nByteSize -= nAlignByteSize;
        if(nAlignByteSize > 0)
        {
            char *pTmpBuf = NULL;
            if(nAlignByteSize <= stream->mAlignBufSize)
            {
                pTmpBuf = stream->mpAlignBuf;
            }
            else
            {
                int ret = posix_memalign((void **)&pTmpBuf, 4096, nAlignByteSize);
                if(ret!=0)
                {
                    aloge("fatal error! malloc fail!");
                    return -1;
                }
            }
            
            memset(pTmpBuf, 0xFF, nAlignByteSize);
            off64_t wrtSize = write(stream->fd_desc.fd, pTmpBuf, nAlignByteSize);
            if(wrtSize!=nAlignByteSize)
            {
                aloge("fatal error! [%lld]!=[%lld]", wrtSize, nAlignByteSize);
            }
            fileOffset = stream->mFileEndOffset = stream->mFileSize = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);
            if(nAlignByteSize <= stream->mAlignBufSize)
            {
            }
            else
            {
                free(pTmpBuf);
                pTmpBuf = NULL;
            }
        }
        if(0 == nByteSize)
        {
            return nmemb;
        }
        else
        {
            alogw("why it happened? write() instead of ftruncate(), left [%lld]bytes to write again!", nByteSize);
        }
    }
    //process fileOffset align of first block.
    if(fileOffset%DIRECTIO_UNIT_SIZE!=0)
    {
        alogd("DirectIO! process file offset not align![%lld]", fileOffset);
        alignOffset = (fileOffset/DIRECTIO_UNIT_SIZE)*DIRECTIO_UNIT_SIZE;
        off64_t seekRet = lseek64(stream->fd_desc.fd, alignOffset, SEEK_SET);
        if(-1 == seekRet)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        int blockLeftSize = DIRECTIO_UNIT_SIZE - (fileOffset - alignOffset);
        result = read(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
        if (-1 == result) 
        {
        	aloge("fatal error! read [%d]bytes[%lld] fail!", DIRECTIO_UNIT_SIZE, result);
            return -1;
        }
        if(result != DIRECTIO_UNIT_SIZE)
        {
            alogw("fatal error! read [%d]bytes[%lld], maybe read file end!", DIRECTIO_UNIT_SIZE, result);
            //int trucRet = ftruncate(stream->fd_desc.fd, alignOffset + DIRECTIO_UNIT_SIZE);
            off64_t seekPos = lseek64(stream->fd_desc.fd, alignOffset, SEEK_SET);
            if(-1 == seekPos)
            {
                aloge("fatal error! seek fail!");
                return -1;
            }
            memset(stream->mpAlignBuf+result, 0xFF, DIRECTIO_UNIT_SIZE-result);
            off64_t wrtSize = write(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
            alogw("fatal error! truncate size to [%lld]bytes!", alignOffset + DIRECTIO_UNIT_SIZE);
            if(-1 == wrtSize)
            {
                aloge("fatal error! truncate fail");
                return -1;
            }
            stream->mFileEndOffset = alignOffset + result;
            stream->mFileSize = alignOffset + DIRECTIO_UNIT_SIZE;
        }
        result = lseek64(stream->fd_desc.fd, alignOffset, SEEK_SET);
        if(-1 == result)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        if(blockLeftSize >= nByteSize)
        {
            alogd("can write done once! fileOffset[%lld][%lld], writeSize[%lld]", alignOffset, fileOffset, nByteSize);
            if(0 == stream->mFtruncateFlag)
            {
                memcpy(stream->mpAlignBuf + (fileOffset - alignOffset), pCurPtr, nByteSize);
            }
            result = write(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
            if (result != DIRECTIO_UNIT_SIZE) 
            {
                aloge("fatal error! write[%d]bytes[%lld] fail[%s]!", DIRECTIO_UNIT_SIZE, result, strerror(errno));
                return -1;
            }
            pCurPtr += nByteSize;
            result = lseek64(stream->fd_desc.fd, fileOffset+nByteSize, SEEK_SET);
            if(-1 == result)
            {
                aloge("fatal error! seek fail!");
                return -1;
            }
            stream->fd_desc.cur_offset = result;
            return nmemb;
        }
        else
        {
            alogd("fileOffset[%lld][%lld], write[%d], need writeSize[%lld]", alignOffset, fileOffset, blockLeftSize, nByteSize);
            if(0 == stream->mFtruncateFlag)
            {
                memcpy(stream->mpAlignBuf + (fileOffset - alignOffset), pCurPtr, blockLeftSize);
            }
            result = write(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
            if (result != DIRECTIO_UNIT_SIZE) 
            {
                aloge("fatal error! write[%d]bytes[%lld] fail[%s]!", blockLeftSize, result, strerror(errno));
                return -1;
            }
            pCurPtr += blockLeftSize;
            nByteSize -= blockLeftSize;
        }
    }
    //write more whole blockSize
    off64_t alignSize = (nByteSize/DIRECTIO_UNIT_SIZE)*DIRECTIO_UNIT_SIZE;
    nByteSize -= alignSize;
    if(stream->mFtruncateFlag || (off64_t)pCurPtr%DIRECTIO_USER_MEMORY_ALIGN!=0)
    {
		if(pCurPtr!=NULL)
		{
        	alogw("directIO ptr[%p] not align [%d]bytes, size[%lld]bytes copy", pCurPtr, DIRECTIO_USER_MEMORY_ALIGN, alignSize);
		}
        while(alignSize > 0)
        {
            if(alignSize >= stream->mAlignBufSize)
            {
                if(0 == stream->mFtruncateFlag)
                {
                    memcpy(stream->mpAlignBuf, pCurPtr, stream->mAlignBufSize);
                }
                result = write(stream->fd_desc.fd, stream->mpAlignBuf, stream->mAlignBufSize);
                if(result != stream->mAlignBufSize)
                {
                    aloge("fatal error! write[%d]bytes[%lld] fail[%s]!", stream->mAlignBufSize, result, strerror(errno));
                    return -1;
                }
                pCurPtr += stream->mAlignBufSize;
                alignSize -= stream->mAlignBufSize;
            }
            else
            {
                if(alignSize%DIRECTIO_UNIT_SIZE!=0)
                {
                    aloge("fatal error! not align[%lld][%d]!", alignSize, DIRECTIO_UNIT_SIZE);
                    assert(0);
                }
                if(0 == stream->mFtruncateFlag)
                {
                    memcpy(stream->mpAlignBuf, pCurPtr, alignSize);
                }
                result = write(stream->fd_desc.fd, stream->mpAlignBuf, alignSize);
                if(result != alignSize)
                {
                    aloge("fatal error! write[%lld]bytes[%lld] fail[%s]!", alignSize, result, strerror(errno));
                    return -1;
                }
                pCurPtr += alignSize;
                alignSize = 0;
            }
        }
    }
    else
    {
        //alogd("directIO ptr[%p] and size[%lld] all align [%d]bytes, directly write", pCurPtr, alignSize, DIRECTIO_UNIT_SIZE);
        if(alignSize > 0)
        {
            result = write(stream->fd_desc.fd, pCurPtr, alignSize);
            if(result != alignSize)
            {
                aloge("fatal error! write[%lld]bytes[%lld] fail[%s]!", alignSize, result, strerror(errno));
                return -1;
            }
            pCurPtr += alignSize;
            alignSize = 0;
        }
    }
    //write last bytes, must < DIRECTIO_UNIT_SIZE
    if(nByteSize)
    {
        alogd("DirectIO! left not align bytes[%lld] to process!", nByteSize);
        off64_t curOffset = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);   //must align.
        if(-1 == curOffset)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        off64_t desOffset = curOffset + nByteSize;
        if(curOffset == stream->mFileEndOffset)
        {
            alogd("DirectIO! not need read because curOffset[%lld]==fileEndOffset!", curOffset);
            result = 0;
        }
        else
        {
            result = read(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
            if (-1 == result) 
            {
            	aloge("fatal error! read [%d]bytes[%lld] fail!", DIRECTIO_UNIT_SIZE, result);
                return -1;
            }
        }
        if(result != DIRECTIO_UNIT_SIZE)
        {
            alogw("read [%d]bytes[%lld], maybe read file end! extend file size to [%lld]bytes", 
                __FUNCTION__, __LINE__, DIRECTIO_UNIT_SIZE, result, curOffset + DIRECTIO_UNIT_SIZE);
            //I don't know whether it can run normally when call ftruncate() here, so I use write() to extend file size.
            memset(stream->mpAlignBuf+result, 0xFF, DIRECTIO_UNIT_SIZE-result);
            //need update mEndOffset and mFileSize.
            stream->mFileEndOffset = desOffset;
            stream->mFileSize = curOffset + DIRECTIO_UNIT_SIZE;
        }
        else
        {
            //if can read full size, it means file not extend. so not need update mEndOffset and mFileSize.
            if(curOffset + DIRECTIO_UNIT_SIZE == stream->mFileSize)
            {
                //if meet file physical end, process mEndOffset carefully.
                if(stream->mFileEndOffset < desOffset)
                {
                    alogw("Be careful! update FileEndOffset[%lld] to [%lld]!", stream->mFileEndOffset, desOffset);
                    stream->mFileEndOffset = desOffset;
                }
            }
            else if(curOffset + DIRECTIO_UNIT_SIZE > stream->mFileSize)
            {
                aloge("fatal error! [%lld][%d][%lld]!", curOffset, DIRECTIO_UNIT_SIZE, stream->mFileSize);
                assert(0);
            }
        }
        result = lseek64(stream->fd_desc.fd, curOffset, SEEK_SET);
        if(-1 == result)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        if(0 == stream->mFtruncateFlag)
        {
            memcpy(stream->mpAlignBuf, pCurPtr, nByteSize);
        }
        result = write(stream->fd_desc.fd, stream->mpAlignBuf, DIRECTIO_UNIT_SIZE);
        if(result != DIRECTIO_UNIT_SIZE)
        {
            aloge("fatal error! write[%d]bytes[%lld] fail[%s]!", DIRECTIO_UNIT_SIZE, result, strerror(errno));
            return -1;
        }
        pCurPtr += nByteSize;
        nByteSize = 0;
        result = lseek64(stream->fd_desc.fd, desOffset, SEEK_SET);
        if(-1 == result)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        stream->fd_desc.cur_offset = result;
    }
    else
    {
        //update mEndOffset and mFileSize.
        fileOffset = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);
        if(-1 == fileOffset)
        {
            aloge("fatal error! seek fail!");
            return -1;
        }
        stream->fd_desc.cur_offset = fileOffset;
        if(stream->mFileSize < fileOffset)
        {
            stream->mFileSize = fileOffset;
        }
        if(stream->mFileEndOffset < fileOffset)
        {
            stream->mFileEndOffset = fileOffset;
        }
    }
    return nmemb;
}

#else
ssize_t cdx_write_fd_file(const void *ptr, size_t size, size_t nmemb, struct cdx_stream_info *stream)
{
	int result = lseek64(stream->fd_desc.fd, stream->fd_desc.cur_offset, SEEK_SET);
	if(result == -1) {
        aloge("fatal error! seek fail!");
		return -1;
	}
    ssize_t num_bytes_written = 0;
    char *curPtr = (char*)ptr;
    size_t leftCount = size * nmemb;
    ssize_t writeNum = 0;
    while(leftCount > 0)
    {
        writeNum = write(stream->fd_desc.fd , (const void*)curPtr, leftCount);
        if(writeNum != leftCount)
        {
            aloge("Stream[%p] write error [%d]!=[%u](%s)", stream, writeNum, leftCount, strerror(errno));
            if(writeNum > 0)
            {
                curPtr += writeNum;
                leftCount -= writeNum;
                continue;
            }
            else
            {
                if(EAGAIN == errno || EINTR == errno)
                {
                    alogd("Be careful. write error is[%d], writeNum[%d], continue", errno, writeNum);
                    continue;
                }
                else
                {
                    aloge("fatal error! write error is[%d], writeNum[%d], don't write again.", errno, writeNum);
                    break;
                }
            }
        }
        else
        {
            leftCount -= writeNum;
        }
    }
    num_bytes_written = size * nmemb - leftCount;
	cdx_off_t seekResult = lseek64(stream->fd_desc.fd, 0, SEEK_CUR);
    if(-1 == seekResult)
    {
        aloge("fatal error! seek error!");
        return -1;
    }
	stream->fd_desc.cur_offset = seekResult < stream->fd_desc.offset
			? stream->fd_desc.offset: seekResult;
    if(seekResult < stream->fd_desc.offset)
    {
        aloge("fatal error! write pos[%lld]<offset[%lld]", seekResult, stream->fd_desc.offset);
    }
    if (num_bytes_written != -1) 
    {
    	num_bytes_written /= size;
    }
    else
    {
        aloge("fatal error! write[%d]bytes[%d] fail[%s]!", (size*nmemb), num_bytes_written, strerror(errno));
    }

    return num_bytes_written;
}

#endif

long long cdx_get_fd_size_file(struct cdx_stream_info *stream)
{
    alogv("(f:%s, l:%d)");
    if(stream->mFileEndOffset != stream->mFileSize)
    {
        aloge("fatal error! [%lld]!=[%lld]", stream->mFileEndOffset, stream->mFileSize);
    }
	//return stream->fd_desc.length;
	cdx_off_t fdSize = lseek64(stream->fd_desc.fd, 0, SEEK_END);
    if(-1 == fdSize)
    {
        aloge("fatal error! seek fail!");
	}
    cdx_off_t size = fdSize - stream->fd_desc.offset;
	cdx_off_t seekResult = lseek64(stream->fd_desc.fd, stream->fd_desc.cur_offset, SEEK_SET);
    if(-1 == seekResult)
    {
        aloge("fatal error! seek fail!");
	}
    if(size < 0)
    {
        aloge("fatal error! size[%lld]<0, fdSize[%lld], offset[%lld]", size, fdSize, stream->fd_desc.offset);
    }
    return size;
}

int cdx_truncate_fd_file(struct cdx_stream_info *stream, cdx_off_t length)
{
    alogv("(f:%s, l:%d)");
    int ret = ftruncate(stream->fd_desc.fd, length);
    if(ret!=0)
    {
        aloge("fatal error! ftruncate fail[%d]", ret);
    }
    stream->mFileSize = length;
    stream->mFileEndOffset = length;
    return ret;
}

int cdx_fallocate_fd_file(struct cdx_stream_info *stream, int mode, int64_t offset, int64_t len)
{
    alogv("(f:%s, l:%d)");
    int ret = 0;
#if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
    alogw("Don't fallocate when use directIO, mode[%d], offset[%lld], len[%lld]", mode, offset, len);
    return 0;
#else
    if(stream->fd_desc.fd >= 0)
    {
        pthread_mutex_lock(&gFallocateLock);
        int64_t tm1, tm2;
        tm1 = CDX_GetSysTimeUsMonotonic();
        if (fallocate(stream->fd_desc.fd, mode, offset, len) < 0)
        //if((ret = posix_fallocate(stream->fd_desc.fd, offset, len)) != 0)
        {
            aloge("fatal error! Failed to fallocate size %lld, ret[%d](%s)", len, ret, strerror(errno));
            pthread_mutex_unlock(&gFallocateLock);
            return -1;
        }
        tm2 = CDX_GetSysTimeUsMonotonic();
        alogd("stream[%p] fallocate size(%lld)Byte, time(%lld)ms", stream, len, (tm2-tm1)/1000);
        pthread_mutex_unlock(&gFallocateLock);
        return 0;
    }
    else
    {
        aloge("fatal error! wrong fd[%d]", stream->fd_desc.fd);
        return -1;
    }
#endif
}

void file_destory_instream_handle(struct cdx_stream_info * stm_info)
{
    if(!stm_info)
    {
		return;
	}
    if(stm_info->data_src_desc.source_type == CEDARX_SOURCE_FILEPATH) 
    {
        if(stm_info->file_handle != NULL)
        {
            fclose(stm_info->file_handle);
            stm_info->file_handle = NULL;
        }
    } 
    if(stm_info->fd_desc.fd >= 0)
    {
        if(0 == stm_info->fd_desc.fd)
        {
            alogd("Be careful! fd==0!");
        }
        //Local videos.
        close(stm_info->fd_desc.fd);
        stm_info->fd_desc.fd = -1;
    }
	
    if(stm_info->mpFilePath)
    {
        free(stm_info->mpFilePath);
        stm_info->mpFilePath = NULL;
    }
    if(stm_info->mpAlignBuf)
    {
        free(stm_info->mpAlignBuf);
        stm_info->mpAlignBuf = NULL;
    }
    stm_info->mAlignBufSize = 0;
}

int file_create_instream_handle(CedarXDataSourceDesc *datasource_desc, struct cdx_stream_info *stm_info)
{
    int ret = 0;
    stm_info->mIODirectionFlag = 0;
    stm_info->fd_desc.fd = -1;
    if(datasource_desc->source_type == CEDARX_SOURCE_FILEPATH) 
    {
        alogv(" source url = %s", datasource_desc->source_url);
//              stm_info->fd_desc.fd  = open(datasource_desc->source_url,
//                      O_RDONLY | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP
//                      | S_IWGRP | S_IROTH | S_IWOTH);
        stm_info->file_handle = fopen(datasource_desc->source_url,"rb");
        if (stm_info->file_handle == NULL) 
        {
            aloge(" fat_open error(%s)", strerror(errno));
            ret = -1;
        }
        if(-1 == ret)
        {
            return ret;
        }
        stm_info->seek  = cdx_seek_stream_file ;
        stm_info->tell  = cdx_tell_stream_file ;
        stm_info->read  = cdx_read_stream_file ;
        stm_info->write = cdx_write_stream_file;
        stm_info->getsize = cdx_get_stream_size_file;
        stm_info->destory = file_destory_instream_handle;
        alogv("open url file:%s",datasource_desc->source_url);
        return ret;
    }       
    else if(datasource_desc->source_type == CEDARX_SOURCE_FD)
    {
        //Must dup a new file descirptor here and close it when destroying stream handle.
        stm_info->fd_desc.fd         = dup(datasource_desc->ext_fd_desc.fd);
        //stm_info->fd_desc.fd         = dup2SeldomUsedFd(datasource_desc->ext_fd_desc.fd);
        stm_info->fd_desc.offset     = datasource_desc->ext_fd_desc.offset;
        stm_info->fd_desc.cur_offset = datasource_desc->ext_fd_desc.offset;
        stm_info->fd_desc.length     = datasource_desc->ext_fd_desc.length;
        alogv("stm_info %p, open fd file:%d, offset %lld, length %lld", stm_info, stm_info->fd_desc.fd,
                stm_info->fd_desc.offset, stm_info->fd_desc.length);
        stm_info->seek      = cdx_seek_fd_file;
        stm_info->tell      = cdx_tell_fd_file;
        stm_info->read      = cdx_read_fd_file;
        stm_info->write     = cdx_write_fd_file;
        stm_info->getsize   = cdx_get_fd_size_file;
        stm_info->destory = file_destory_instream_handle;
        stm_info->seek(stm_info, 0, SEEK_SET);
        return ret;
    }
    else
    {
        aloge("fatal error! source_type[%d] stream_type[%d] wrong!", datasource_desc->source_type, datasource_desc->stream_type);
        ret = -1;
        return ret;
    }
}

static void destory_outstream_handle_file(struct cdx_stream_info *stm_info)
{
    alogv("(f:%s, l:%d)");
  #if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
    if(stm_info->mFileEndOffset < stm_info->mFileSize)
    {
        alogd("fd[%d], [%lld]<[%lld], DirectIO use ftruncate() before close.", 
            __FUNCTION__, __LINE__, stm_info->fd_desc.fd, stm_info->mFileEndOffset, stm_info->mFileSize);
        int ret = ftruncate(stm_info->fd_desc.fd, stm_info->mFileEndOffset);
        if(ret!=0)
        {
            aloge("fatal error! ftruncate fail[%d]", ret);
        }
        stm_info->mFileSize = stm_info->mFileEndOffset;
    }
  #endif

	if(stm_info->file_handle != NULL)
    {
//            int fd = fileno(stm_info->file_handle);
//            if(fd) {
//        		close(fd);
//        		fd = -1;
//        	}
            fclose(stm_info->file_handle);
            stm_info->file_handle = NULL;
	}
    if(stm_info->fd_desc.fd >= 0)
    {
        if(0 == stm_info->fd_desc.fd)
        {
            alogd("Be careful! close fd==0");
        }
        close(stm_info->fd_desc.fd);
        stm_info->fd_desc.fd = -1;
    }

    if(stm_info->mpFilePath)
    {
        free(stm_info->mpFilePath);
        stm_info->mpFilePath = NULL;
    }
    if(stm_info->mpAlignBuf)
    {
        free(stm_info->mpAlignBuf);
        stm_info->mpAlignBuf = NULL;
    }
    stm_info->mAlignBufSize = 0;
	//return 0;
}

int create_outstream_handle_file(struct cdx_stream_info *stm_info, CedarXDataSourceDesc *datasource_desc)
{
    stm_info->mIODirectionFlag = 1;
//#ifdef __OS_ANDROID
#if 0
	if (datasource_desc->source_type == CEDARX_SOURCE_FD) {
		stm_info->fd_desc.fd = datasource_desc->ext_fd_desc.fd;
	} else {
		stm_info->fd_desc.fd = open(datasource_desc->source_url, O_CREAT | O_TRUNC | O_RDWR, 0644);
	}

	if (stm_info->fd_desc.fd == -1){
		LOGE("open file error!");
		return -1;
	}
#else
    stm_info->fd_desc.fd = -1;
    if(CEDARX_SOURCE_FD == datasource_desc->source_type)
    {
        //alogd("fd[%d], offset[%lld], length[%lld]", datasource_desc->ext_fd_desc.fd, datasource_desc->ext_fd_desc.offset, datasource_desc->ext_fd_desc.length);
        #if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_IO)
            //get filepath, reopen again!
            stm_info->mpFilePath = generateFilepathFromFd(datasource_desc->ext_fd_desc.fd);
            stm_info->fd_desc.fd = open(stm_info->mpFilePath, O_CREAT | O_TRUNC | O_RDWR | O_DIRECT, 0666);
            alogd("directIO dup fd[%d]->[%d]!", datasource_desc->ext_fd_desc.fd, stm_info->fd_desc.fd);
            stm_info->fd_desc.offset = datasource_desc->ext_fd_desc.offset;
            stm_info->fd_desc.cur_offset = datasource_desc->ext_fd_desc.offset;
            stm_info->fd_desc.length = datasource_desc->ext_fd_desc.length;
            stm_info->mFileEndOffset = lseek64(stm_info->fd_desc.fd, 0, SEEK_END);
            stm_info->mFileSize = stm_info->mFileEndOffset;
            if(stm_info->mFileEndOffset!=0)
            {
                aloge("fatal error! mEndOffset[%lld], mFileSize[%lld] should be 0!", stm_info->mFileEndOffset, stm_info->mFileSize);
            }
            if(stm_info->fd_desc.cur_offset!=0)
            {
                aloge("fatal error! cur_offset[%lld]!=0", stm_info->fd_desc.cur_offset);
            }
            stm_info->seek  = cdx_seek_fd_file ;
        	stm_info->tell  = cdx_tell_fd_file ;
        	stm_info->read  = cdx_read_fd_file ;
        	stm_info->write = cdx_write_fd_file;
            stm_info->truncate = cdx_truncate_fd_file;
            stm_info->fallocate = cdx_fallocate_fd_file;
        	stm_info->getsize = cdx_get_fd_size_file;
            stm_info->destory = destory_outstream_handle_file;
            stm_info->seek(stm_info, 0, SEEK_SET);
        	return 0;
        #else
          #if 0
            int nFd;
            if(datasource_desc->ext_fd_desc.fd >= 0)
            {
                nFd = dup(datasource_desc->ext_fd_desc.fd);
                //nFd = dup2SeldomUsedFd(datasource_desc->ext_fd_desc.fd);
            }
            else
            {
                nFd = -1;
            }
            if (nFd < 0)
            {
                aloge("fatal error! open file error[%s]! fd[%d][%d]", strerror(errno), datasource_desc->ext_fd_desc.fd, nFd);
                return -1;
            }
            stm_info->file_handle = fdopen(nFd, "w+b");
            if(stm_info->file_handle==NULL) 
            {
                aloge("fatal error! get file fd failed");
                close(nFd);
                nFd = -1;
                return -1;
            }
            nFd = -1;
          #else
            stm_info->fd_desc.fd = dup(datasource_desc->ext_fd_desc.fd);
            stm_info->mpFilePath = generateFilepathFromFd(stm_info->fd_desc.fd);
            stm_info->fd_desc.offset = datasource_desc->ext_fd_desc.offset;
            stm_info->fd_desc.cur_offset = datasource_desc->ext_fd_desc.offset;
            stm_info->fd_desc.length = datasource_desc->ext_fd_desc.length;
            stm_info->mFileEndOffset = lseek64(stm_info->fd_desc.fd, 0, SEEK_END);
            stm_info->mFileSize = stm_info->mFileEndOffset;
            if(stm_info->mFileEndOffset!=0)
            {
                alogw("maybe file is exist long ago! mEndOffset[%lld], mFileSize[%lld] should be 0!", stm_info->mFileEndOffset, stm_info->mFileSize);
            }
            if(stm_info->fd_desc.cur_offset!=0)
            {
                aloge("fatal error! cur_offset[%lld]!=0", stm_info->fd_desc.cur_offset);
            }
            stm_info->seek  = cdx_seek_fd_file ;
        	stm_info->tell  = cdx_tell_fd_file ;
        	stm_info->read  = cdx_read_fd_file ;
        	stm_info->write = cdx_write_fd_file;
            stm_info->truncate = cdx_truncate_fd_file;
            stm_info->fallocate = cdx_fallocate_fd_file;
        	stm_info->getsize = cdx_get_fd_size_file;
        	stm_info->destory = destory_outstream_handle_file;
            stm_info->seek(stm_info, 0, SEEK_SET);
            return 0;
          #endif
        #endif
    }
    else if(CEDARX_SOURCE_FILEPATH == datasource_desc->source_type)
    {
            stm_info->file_handle = fopen(datasource_desc->source_url, "w+b");
            if(stm_info->file_handle == NULL)
            {
                aloge("Failed to open file %s(%s)", datasource_desc->source_url, strerror(errno));
            	return -1;
            }
    }
    else
    {
        aloge("fatal error! wrong source_type[%d]", datasource_desc->source_type);
        return -1;
    }
#endif

	stm_info->seek  = cdx_seek_stream_file ;
	stm_info->tell  = cdx_tell_stream_file ;
	stm_info->read  = cdx_read_stream_file ;
	stm_info->write = cdx_write_stream_file;
    stm_info->truncate = cdx_truncate_stream_file;
    stm_info->fallocate = cdx_fallocate_stream_file;
	stm_info->getsize = cdx_get_stream_size_file;
	stm_info->destory = destory_outstream_handle_file;

	return 0;
}

int stream_remove_file(char* fileName)
{
    return remove(fileName);
}

int stream_mkdir(char *dirName, int mode)
{
    int ret;
    ret = 0;
    if(access (dirName, F_OK) != 0)
    {
        alogv("creat directory , path=%s is not exist,so creat it\n", dirName);
        ret = mkdir(dirName, mode);
    }
    return ret;
}

