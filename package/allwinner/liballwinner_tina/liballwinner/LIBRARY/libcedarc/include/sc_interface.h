/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : typedef.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef SC_INTERFACE_H
#define SC_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct ScMemOpsS
{
    int (*open)(void);

    void (*close)(void);

    int (*total_size)(void);

    void *(*palloc)(int /*size*/);

    void  (*pfree)(void* /*mem*/);

    void (*flush_cache)(void * /*mem*/, int /*size*/);

    void *(*ve_get_phyaddr)(void * /*viraddr*/);

    void *(*ve_get_viraddr)(void * /*phyaddr*/);

    void *(*cpu_get_phyaddr)(void * /*viraddr*/);

    void *(*cpu_get_viraddr)(void * /*phyaddr*/);

    int (*mem_set)(void * /*s*/, int /*c*/, size_t /*n*/);

    int (*mem_cpy)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*mem_read)(void * /*dest */, void * /*src*/, size_t /*n*/);

    int (*mem_write)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*setup)(void);

    int (*shutdown)(void);

    //*the interface just for secureAlloc.c, not used by other function
    void *(*palloc_secure)(int /*size*/);

};

static inline int CdcMemOpen(struct ScMemOpsS *memops)
{
    return memops->open();
}

//* close the memory adapter.
static inline void CdcMemClose(struct ScMemOpsS *memops)
{
    memops->close();
}

static inline int CdcMemTotalSize(struct ScMemOpsS *memops)
{
    return memops->total_size();
}

static inline void *CdcMemPalloc(struct ScMemOpsS *memops, int nSize)
{
    return memops->palloc(nSize);
}

static inline void CdcMemPfree(struct ScMemOpsS *memops, void* pMem)
{
    memops->pfree(pMem);
}

static inline void CdcMemFlushCache(struct ScMemOpsS *memops, void* pMem, int nSize)
{
    memops->flush_cache(pMem, nSize);
}

static inline void *CdcMemGetPhysicAddress(struct ScMemOpsS *memops, void* pVirtualAddress)
{
    return memops->ve_get_phyaddr(pVirtualAddress);
}

static inline void *CdcMemGetVirtualAddress(struct ScMemOpsS *memops, void* pPhysicAddress)
{
    return memops->ve_get_viraddr(pPhysicAddress);
}

static inline void CdcMemSet(struct ScMemOpsS *memops, void* pMem, int nValue, int nSize)
{
   memops->mem_set(pMem, nValue, nSize);
}

static inline void CdcMemCopy(struct ScMemOpsS *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    memops->mem_cpy(pMemDst, pMemSrc, nSize);
}

static inline int CdcMemRead(struct ScMemOpsS *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    memops->mem_read(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline int CdcMemWrite(struct ScMemOpsS *memops,void* pMemDst,  void* pMemSrc, int nSize)
{
    (void)memops; /*not use memops */
    memops->mem_write(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline void *CdcMemGetPhysicAddressCpu(struct ScMemOpsS *memops, void *virt)
{
    return memops->cpu_get_phyaddr(virt);
}

static inline void *CdcMemGetVirtualAddressCpu(struct ScMemOpsS *memops, void *phy)
{
    return memops->cpu_get_viraddr(phy);
}

static inline int CdcMemSetup(struct ScMemOpsS *memops)
{
    return memops->setup();
}

static inline int CdcMemShutdown(struct ScMemOpsS *memops)
{
    return memops->shutdown();
}

static inline void *CdcMemPallocSecure(struct ScMemOpsS *memops, int nSize)
{
    return memops->palloc_secure(nSize);
}

#ifdef __cplusplus
}
#endif

#endif
