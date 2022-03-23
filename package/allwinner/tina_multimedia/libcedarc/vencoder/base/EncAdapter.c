/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd.
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 *
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from
 * Softwinner Products.
 *
 * The entire notice above must be reproduced on all copies
 * and should not be removed.
 */

#include "log.h"

#include "EncAdapter.h"

//* should include other  '*.h'  before CdcMalloc.h
#include "CdcMalloc.h"

//* use method provide by libVE.so to process ve control methods.
//* use method provide by libMemAdapter.so to process physical continue memory allocation.

int EncAdapterInitializeMem(struct ScMemOpsS *memops)
{
    struct ScMemOpsS *_memops = memops;

    if(EncMemAdapterOpen() < 0)
        return -1;
    //If mediaserver died when playing protected media,
    //some hardware is in protected mode, shutdown.
#if 0 // TODO: do it in outside.
{
    SecureMemAdapterOpen();
    SecureMemAdapterClose();
}
#endif
    return 0;
}

void EncAdpaterRelease(struct ScMemOpsS *memops)
{
    struct ScMemOpsS *_memops = memops;
    if(memops)
    {
        EncMemAdapterClose();
    }
    return;
}

//* memory methods.

int __EncAdapterMemOpen(struct ScMemOpsS *memops)
{
    return memops->open();
}

void __EncAdapterMemClose(struct ScMemOpsS *memops)
{
    return memops->close();
}

void* __EncAdapterMemPalloc(struct ScMemOpsS *memops, int nSize, void *veOps, void *pVeopsSelf)
{
    return memops->palloc(nSize, veOps, pVeopsSelf);
}

void __EncAdapterMemPfree(struct ScMemOpsS *memops, void* pMem, void *veOps, void *pVeopsSelf)
{
    memops->pfree(pMem, veOps, pVeopsSelf);
}

void __EncAdapterMemFlushCache(struct ScMemOpsS *memops, void* pMem, int nSize)
{
    memops->flush_cache(pMem, nSize);
}

void* __EncAdapterMemGetPhysicAddress(struct ScMemOpsS *memops, void* pVirtualAddress)
{
    return memops->ve_get_phyaddr(pVirtualAddress);
}

void* __EncAdapterMemGetPhysicAddressCpu(struct ScMemOpsS *memops, void* pVirtualAddress)
{
    return memops->cpu_get_phyaddr(pVirtualAddress);
}

void* __EncAdapterMemGetVirtualAddress(struct ScMemOpsS *memops, void* pPhysicAddress)
{
    return memops->ve_get_viraddr(pPhysicAddress);
}

unsigned int __EncAdapterMemGetVeAddrOffset(struct ScMemOpsS *memops)
{
    return memops->get_ve_addr_offset();
}

unsigned int EncAdapterGetICVersion(void* nTopBaseAddr)
{
   volatile unsigned int value;
   value = *((unsigned int*)((char *)nTopBaseAddr + 0xf0));
   if(value == 0)
   {
        value = *((unsigned int*)((char *)nTopBaseAddr + 0xe4));
        if(value == 0)
        {
            loge("can not get the ve version ,both 0xf0 and 0xe4 is 0x00000000\n");
            return 0;
        }
        else
            return value;
   }
   else
        return (value>>16);
}

void EncAdapterPrintTopVEReg(void* pTopBaseAddr)
{
    int i;
    volatile int *ptr = (int*)pTopBaseAddr;

    logd("--------- register of top level ve base:%p -----------\n",ptr);
    for(i=0;i<16;i++)
    {
        logd("reg%02x:%08x %08x %08x %08x",i*16,ptr[0],ptr[1],ptr[2],ptr[3]);
        ptr += 4;
    }
}

void EncAdapterPrintEncReg(void* pTopBaseAddr)
{
    int i;
    volatile int *ptr = (int*)((unsigned long)pTopBaseAddr + 0xB00);

    logd("--------- register of ve encoder base:%p -----------\n",ptr);
    for(i=0;i<16;i++)
    {
        logd("reg%02x:%08x %08x %08x %08x",i*16,ptr[0],ptr[1],ptr[2],ptr[3]);
        ptr += 4;
    }
}

void EncAdapterPrintIspReg(void* pTopBaseAddr)
{
    CEDARC_UNUSE(__EncAdapterMemGetVirtualAddress);

    int i;
    volatile int *ptr = (int*)((unsigned long)pTopBaseAddr + 0xA00);

    logd("--------- register of ve isp base:%p -----------\n",ptr);
    for(i=0;i<16;i++)
    {
        logd("reg%02x:%08x %08x %08x %08x",i*16,ptr[0],ptr[1],ptr[2],ptr[3]);
        ptr += 4;
    }
}
