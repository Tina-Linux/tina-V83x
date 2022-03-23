
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : CdcMalloc.c
* Description :
* History :
*   Author  : wangxiwang <xyliu@allwinnertech.com>
*   Date    : 2018/07/03
*   Comment :
*
*
*/

/*

todo:

1. multi thread case
2. testCase

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include "CdcIniparserapi.h"

#include "CdcMalloc.h"
#include "log.h"

#define  FILE_NAME_LENGTH 64

#undef malloc
#undef calloc
#undef free

typedef struct MEM_INFO
{
    void            *address;
    unsigned int    size;
    char            func_name[FILE_NAME_LENGTH];
    unsigned int    line;
}MEM_INFO;

typedef struct MEM_NODE {
    MEM_INFO mem_info;
    struct MEM_NODE * next;
}MEM_NODE;

static MEM_NODE * mem_list_head = NULL;
static MEM_NODE * mem_list_tail =  NULL;
static pthread_mutex_t mutex_malloc = PTHREAD_MUTEX_INITIALIZER;
static int bCheckMallocLeakFlag = 0;

/*
* adds allocated memory info. into the list
*
*/
static void add(MEM_INFO alloc_info)
{

     MEM_NODE * mem_leak_info = NULL;
     mem_leak_info = (MEM_NODE *) malloc (sizeof(MEM_NODE));
     mem_leak_info->mem_info.address = alloc_info.address;
     mem_leak_info->mem_info.size = alloc_info.size;
     strncpy(mem_leak_info->mem_info.func_name, alloc_info.func_name, FILE_NAME_LENGTH);
     mem_leak_info->mem_info.line = alloc_info.line;
     mem_leak_info->next = NULL;

    if (mem_list_head == NULL)
    {
        mem_list_head = mem_leak_info;
        mem_list_tail = mem_list_head;
    }
    else {
        mem_list_tail->next = mem_leak_info;
        mem_list_tail = mem_list_tail->next;
    }

}
/*
  * erases memory info. from the list
  *
*/
static void erase(unsigned pos)
{

     MEM_NODE * alloc_info, * temp;

      if(pos == 0)
      {
          MEM_NODE * temp = mem_list_head;
          mem_list_head = mem_list_head->next;
          free(temp);
      }
      else
      {

          unsigned index = 0;
          for(index = 0, alloc_info = mem_list_head; index < pos;
              alloc_info = alloc_info->next, ++index)
          {
              if(pos == index + 1)
              {
                  temp = alloc_info->next;
                  alloc_info->next =  temp->next;
                  if(temp->next==NULL)
                  {
                     mem_list_tail = alloc_info;
                  }
                  free(temp);
                  break;
              }
          }
      }
}

#if 0
/*
* deletes all the elements from the list
*/
static void clear()
{
    MEM_NODE * temp = mem_list_head;
    MEM_NODE * alloc_info = mem_list_head;

    while(alloc_info != NULL)
    {
        alloc_info = alloc_info->next;
        free(temp);
        temp = alloc_info;
    }
    mem_list_head=NULL;
}
#endif

/*
* gets the allocated memory info and adds it to a list
*
*/
static void add_mem_info (void * mem_ref, unsigned int size,  const char * file, unsigned int line)
{
    MEM_INFO mem_alloc_info;

    /* fill up the structure with all info */
    memset( &mem_alloc_info, 0, sizeof ( mem_alloc_info ) );
    mem_alloc_info.address  = mem_ref;
    mem_alloc_info.size = size;
    strncpy(mem_alloc_info.func_name, file, FILE_NAME_LENGTH);
    mem_alloc_info.line = line;

    /* add the above info to a list */
    add(mem_alloc_info);
}

/*
* if the allocated memory info is part of the list, removes it
*
*/
static void remove_mem_info (void * mem_ref)
{
    unsigned short index;
    MEM_NODE  * leak_info = mem_list_head;

     /* check if allocate memory is in our list */
    for(index = 0; leak_info != NULL; ++index, leak_info = leak_info->next)
    {
        if ( leak_info->mem_info.address == mem_ref )
        {
             erase ( index );
             break;
        }
    }
}

/*
  * writes all info of the unallocated memory into a file
  */
void CdcMallocReport(void)
{
    MEM_NODE * leak_info;
    int nCount = 0;
    unsigned int nTotalSize = 0;
    unsigned int nLeakSize = 0;

    if(bCheckMallocLeakFlag == 0)
        return ;

    logd("***************************************************");
    logd("*****************malloc info summary **************");

    for(leak_info = mem_list_head; leak_info != NULL; leak_info = leak_info->next)
    {
        nTotalSize += leak_info->mem_info.size;

        if(strcasecmp(leak_info->mem_info.func_name, "VDecoderRegister") != 0)
        {
            logv("  leak item[%d]: addr = %p, size = %d, func = %s, line = %d\n",nCount,
                leak_info->mem_info.address,
                leak_info->mem_info.size,
                leak_info->mem_info.func_name,
                leak_info->mem_info.line);
            nLeakSize += leak_info->mem_info.size;
        }
        else
        {
            logv("malloc item[%d]: addr = %p, size = %d, func = %s, line = %d\n",nCount,
                    leak_info->mem_info.address,
                    leak_info->mem_info.size,
                    leak_info->mem_info.func_name,
                    leak_info->mem_info.line);
        }
        nCount++;
    }
    logd("*****************malloc costSize: %d MB, %d KB, %d B**************",
         nTotalSize/1024/1024, nTotalSize/1024,nTotalSize);
    logd("*******************leak buf Size: %d MB, %d KB, %d B**************",
        nLeakSize/1024/1024, nLeakSize/1024, nLeakSize);
}

/*
 * replacement of malloc
 */
void * cdc_malloc (unsigned int size, const char * file, unsigned int line)
{
    void * ptr = malloc (size);

    if(bCheckMallocLeakFlag == 1)
    {
        pthread_mutex_lock(&mutex_malloc);
        if (ptr != NULL)
        {
            add_mem_info(ptr, size, file, line);
        }
        pthread_mutex_unlock(&mutex_malloc);
    }

    return ptr;
}

/*
 * replacement of calloc
 */
void * cdc_calloc (unsigned int elements, unsigned int size, const char * file, unsigned int line)
{
    void * ptr = calloc(elements , size);

    if(bCheckMallocLeakFlag == 1)
    {
        pthread_mutex_lock(&mutex_malloc);
        if(ptr != NULL)
        {
            unsigned total_size = elements * size;
            add_mem_info (ptr, total_size, file, line);
        }
        pthread_mutex_unlock(&mutex_malloc);
    }

    return ptr;
}


/*
 * replacement of free
 */
void cdc_free(void * mem_ref)
{
    if(bCheckMallocLeakFlag == 1)
    {
        pthread_mutex_lock(&mutex_malloc);
        remove_mem_info(mem_ref);
        pthread_mutex_unlock(&mutex_malloc);
    }

    free(mem_ref);
}

int CdcMallocGetInfo(char*  pDst, int nDstBufsize)
{
    MEM_NODE * leak_info;
    unsigned int nTotalSize = 0;
    int count = 0;

    if(bCheckMallocLeakFlag == 1)
    {
        for(leak_info = mem_list_head; leak_info != NULL; leak_info = leak_info->next)
        {
            nTotalSize += leak_info->mem_info.size;
        }

        if(nTotalSize/1024/1024 > 0)
        {
            count += sprintf(pDst + count, "malloc: %d MB, %d KB, %d B\n",
                             (int)nTotalSize/1024/1024, (int)nTotalSize/1024, (int)nTotalSize);
        }
        else if(nTotalSize/1024 > 0)
        {
            count += sprintf(pDst + count, "malloc: %d KB, %d B\n",
                             (int)nTotalSize/1024, (int)nTotalSize);

        }
        else
        {
            count += sprintf(pDst + count, "malloc: %d B\n",
                             (int)nTotalSize);

        }

        if(count > nDstBufsize)
           count = nDstBufsize;
    }

    return count;
}

int CdcMallocCheckFlag()
{
    bCheckMallocLeakFlag = CdcGetConfigParamterInt("check_malloc_leak", 0);
    return 0;
}
