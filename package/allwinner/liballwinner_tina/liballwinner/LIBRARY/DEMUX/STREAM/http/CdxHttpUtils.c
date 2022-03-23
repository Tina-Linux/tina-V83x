/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxHttpUtils.c
 * Description : Part of http stream.
 * History :
 *
 */

#include <string.h>
#include <stdlib.h>
#include <CdxHttpStream.h>

//set User-Agent field.
const char *GetUA(int n, CdxHttpHeaderFieldT *pHttpHeader)
{
    int i;
    /*const char *defaultUA = "AppleCoreMedia/1.0.0.9A405
     (iPad; U; CPU OS 5_0_1 like Mac OS X; zh_cn)";*/
    //const char *defaultUA = "stagefright/1.2 (Linux;Android 4.2.2)";
    const char *defaultUA = "Allwinner/CedarX 2.6";

    if(pHttpHeader)
    {
        for(i = 0; i < n; i++)
        {
            if(strcasecmp("User-Agent", pHttpHeader[i].key) == 0)
            {
                //return strdup(pHttpHeader[i].val);
                return pHttpHeader[i].val;
            }
        }
    }
    return defaultUA;
}
//get "len" in "len\r\n". tmpLen: store "len", size: num
//return -2: force stop while read len or \r
//       -3: force stop while read \n
cdx_int32 ReadChunkedSize(CdxStreamT *stream, cdx_char tmpLen[], cdx_int32 *size)
{
    cdx_int32 ret;
    cdx_char len[11] = {0};
    cdx_int32 pos = 0;

    while(1)
    {
        cdx_char byte;

        ret = CdxStreamRead(stream, &byte, 1);
        if(ret <= 0)
        {
            if(ret == -2)
            {
                CDX_LOGW("force stop ReadChunkedSize while get len.");
                strcpy(tmpLen, len);
                *size = pos;
                return -2;
            }
            CDX_LOGE("Read failed.");
            return -1;
        }

        CDX_CHECK(ret == 1);
        if((byte >= '0' && byte <='9') || (byte >= 'a' && byte <= 'f') ||
            (byte >= 'A' && byte <= 'F'))
        {
            len[pos++] = byte;
            if(pos > 10)
            {
                CDX_LOGE("chunked len is too big...");
                return -1;
            }
            continue;
        }
        else if(byte == '\r')
        {
            ret = CdxStreamRead(stream, &byte, 1);
            if(ret == -2)
            {
                CDX_LOGW("force stop ReadChunkedSize while get LF.");
                strcpy(tmpLen, len);
                *size = pos;
                return -3;
            }
            CDX_CHECK(ret == 1);
            CDX_CHECK(byte == '\n');
            break;
        }
        else
        {
            CDX_LOGE("Something error happen, %d lencrlf.", byte);
            return -1;
        }
    }

    return strtol(len, NULL, 16);
}
//return > 0 : read data bytes
//      == -1: read failed.
//      == -2: force stop while read data, no data has read.
//      == -3: force stop while read \r\n in data\r\n, all data has been read.
//      == -4: force stop while read \n in data\r\n, all data has been read.
cdx_int32 ReadChunkedData(CdxStreamT *stream, void *ptr, cdx_int32 size)
{
    cdx_int32 ret;
    cdx_int32 pos = 0;
    cdx_char dummy[2];

    while(1)
    {
        ret = CdxStreamRead(stream, (char *)ptr + pos, size - pos);
        if(ret <= 0)
        {
            if(ret == -2)
            {
                CDX_LOGW("force stop ReadChunkedDate.");
                return pos>0 ? pos : -2;
            }
            CDX_LOGE("Read failed.");
            return ret;
        }
        pos += ret;
        if(pos == size)
        {
            ret = CdxStreamRead(stream, dummy, 2);
            if(ret <= 0)
            {
                if(ret == -2)
                {
                    CDX_LOGW("force stop ReadChunkedData.");
                    return -3; // force stop while read \r\n in data\r\n.
                }
                CDX_LOGE("read failed.");
                return -1;
            }
            else if(ret == 1)
            {
                CDX_LOGW("force stop ReadChunkedData.");
                return -4; // force stop while read \n in data\r\n.
            }

            if(memcmp(dummy, "\r\n", 2) != 0)
            {
                CDX_LOGE("Not end with crlf, check the content.");
                return -1;
            }
            break;
        }
    }
    return pos;
}
//remove spaces at head and tail of string
char *RmSpace(char *str)
{
    char *it = NULL;

    if(!str || *str=='\0')
        return str;

    while(*str == ' ')
    {
        ++str;
    }
    it = str;

    while(*str)//point to the str end
    {
        str++;
    }

    while(*(--str)==' ')//remove the space of str end
    {
        ;
    }

    *(++str)='\0';

    return it;
}
