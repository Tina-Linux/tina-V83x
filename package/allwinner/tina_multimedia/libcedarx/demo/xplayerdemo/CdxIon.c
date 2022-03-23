/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ionAlloc.c
* Description :
* History :
*   Author  : weihai <liweihai@allwinnertech.com>
*   Date    : 2017/07/21
*   Comment :
*
*
*/

//#define CONFIG_LOG_LEVEL    LOG_LEVEL_VERBOSE
#define LOG_TAG "ionAlloc"

#include <cdx_log.h>
#include <CdxIon.h>
#include <CdxList.h>

#include <sys/ioctl.h>
#include <errno.h>

#define DEBUG_ION_REF 0    //just for H3 ION memery info debug
#define ION_ALLOC_ALIGN    SZ_4k
#define DEV_NAME                     "/dev/ion"
#define ION_IOC_SUNXI_POOL_INFO        10

#define UNUSA_PARAM(param) (void)param

//----------------------
#if DEBUG_ION_REF==1
int   cdx_use_mem = 0;
typedef struct ION_BUF_NODE_TEST
{
    unsigned int addr;
    int size;
} ion_buf_node_test;

#define ION_BUF_LEN  50
ion_buf_node_test ion_buf_nodes_test[ION_BUF_LEN];
#endif
//----------------------

struct sunxi_pool_info
{
    unsigned int total;     //unit kb
    unsigned int free_kb;  // size kb
    unsigned int free_mb;  // size mb
};

typedef struct BUFFER_NODE
{
    struct CdxListNodeS node;
    unsigned long phy;        //phisical address
    unsigned long vir;        //virtual address
    unsigned int size;        //buffer size
    unsigned int tee;      //
    unsigned long user_virt;//
    ion_fd_data_t fd_data;
    ion_fd_data_t share_data;
} buffer_node;

typedef struct ION_ALLOC_CONTEXT
{
    int                    fd;            // driver handle
    struct CdxListS        list;        // buffer list
    int                    ref_cnt;    // reference count
} ion_alloc_context;

ion_alloc_context    *    g_alloc_context = NULL;
pthread_mutex_t            g_mutex_alloc = PTHREAD_MUTEX_INITIALIZER;

/*funciton begin*/
int CdxIonOpen()
{
    logd("begin ion_alloc_open \n");

    pthread_mutex_lock(&g_mutex_alloc);
    if (g_alloc_context != NULL)
    {
        logv("ion allocator has already been created \n");
        goto SUCCEED_OUT;
    }

    g_alloc_context = (ion_alloc_context*)malloc(sizeof(ion_alloc_context));
    if (g_alloc_context == NULL)
    {
        loge("create ion allocator failed, out of memory \n");
        goto ERROR_OUT;
    }
    else
    {
        logv("pid: %d, g_alloc_context = %p \n", getpid(), g_alloc_context);
    }

    memset((void*)g_alloc_context, 0, sizeof(ion_alloc_context));

    /* Readonly should be enough. */
    g_alloc_context->fd = open(DEV_NAME, O_RDONLY, 0);

    if (g_alloc_context->fd <= 0)
    {
        loge("open %s failed \n", DEV_NAME);
        goto ERROR_OUT;
    }

#if DEBUG_ION_REF==1
    cdx_use_mem = 0;
    memset(&ion_buf_nodes_test, sizeof(ion_buf_nodes_test), 0);
    logd("ion_open, cdx_use_mem=[%dByte].", cdx_use_mem);
    ion_alloc_get_total_size();
#endif

    CdxListInit(&g_alloc_context->list);


SUCCEED_OUT:
    g_alloc_context->ref_cnt++;
    pthread_mutex_unlock(&g_mutex_alloc);
    return 0;

ERROR_OUT:
    if (g_alloc_context != NULL
            && g_alloc_context->fd > 0)
    {
        close(g_alloc_context->fd);
        g_alloc_context->fd = 0;
    }

    if (g_alloc_context != NULL)
    {
        free(g_alloc_context);
        g_alloc_context = NULL;
    }

    pthread_mutex_unlock(&g_mutex_alloc);
    return -1;
}

void CdxIonClose()
{
    buffer_node * pos, *q;

    logv("ion_alloc_close \n");

    pthread_mutex_lock(&g_mutex_alloc);
    if (--g_alloc_context->ref_cnt <= 0)
    {
        logv("pid: %d, release g_alloc_context = %p \n", getpid(), g_alloc_context);

        CdxListForEachEntrySafe(pos, q, &g_alloc_context->list, node)
        {
            logv("ion_alloc_close del item phy= 0x%lx vir= 0x%lx, size= %d \n", \
                 pos->phy, pos->vir, pos->size);
            CdxListDel(&pos->node);
            free(pos);
        }

#if DEBUG_ION_REF==1
        logd("ion_close, cdx_use_mem=[%d MB]", cdx_use_mem/1024/1024);
        ion_alloc_get_total_size();
#endif
        close(g_alloc_context->fd);
        g_alloc_context->fd = 0;

        free(g_alloc_context);
        g_alloc_context = NULL;
    }
    else
    {
        logv("ref cnt: %d > 0, do not free \n", g_alloc_context->ref_cnt);
    }
    pthread_mutex_unlock(&g_mutex_alloc);

    //--------------
#if DEBUG_ION_REF==1
    int i = 0;
    int counter = 0;
    for(i=0; i<ION_BUF_LEN; i++)
    {
        if(ion_buf_nodes_test[i].addr != 0 || ion_buf_nodes_test[i].size != 0)
        {

            loge("ion mem leak????  addr->[0x%x], leak size->[%dByte]", \
                 ion_buf_nodes_test[i].addr, ion_buf_nodes_test[i].size);
            counter ++;
        }
    }

    if(counter != 0)
    {
        loge("my god, have [%d]blocks ion mem leak.!!!!", counter);
    }
    else
    {
        logd("well done, no ion mem leak.");
    }
#endif
    //--------------
    return ;
}

// return virtual address: 0 failed
void* CdxIonPalloc(int size)
{
    aw_ion_allocation_info_t alloc_data;
    ion_fd_data_t fd_data;
    struct ion_handle_data handle_data;
    struct aw_ion_custom_info custom_data;
    sunxi_phys_data   phys_data;

    int rest_size = 0;
    unsigned long addr_phy = 0;
    unsigned long addr_vir = 0;
    buffer_node * alloc_buffer = NULL;
    int ret = 0;

    pthread_mutex_lock(&g_mutex_alloc);

    if (g_alloc_context == NULL)
    {
        loge("ion_alloc do not opened, should call ion_alloc_open() \
            before ion_alloc_alloc(size) \n");
        goto ALLOC_OUT;
    }

    if(size <= 0)
    {
        loge("can not alloc size 0 \n");
        goto ALLOC_OUT;
    }

    alloc_data.aw_len = (size_t)size;
    alloc_data.aw_align = ION_ALLOC_ALIGN ;

    alloc_data.aw_heap_id_mask = AW_ION_DMA_HEAP_MASK | AW_ION_CARVEOUT_HEAP_MASK;
    alloc_data.flags = AW_ION_CACHED_FLAG | AW_ION_CACHED_NEEDS_SYNC_FLAG;

    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_ALLOC, &alloc_data);
    if (ret)
    {
        loge("ION_IOC_ALLOC error \n");
        goto ALLOC_OUT;
    }

    ion_fd_data_t share_data =
    {
        .handle = alloc_data.handle,
    };

    ret = ioctl(g_alloc_context->fd, AW_MEMION_IOC_SHARE, &share_data);
    if (ret < 0)
    {
        loge("share ioctl returned negative ret\n");
        goto ALLOC_OUT;
    }

    logv("dma_fd %d",share_data.aw_fd);

    if (share_data.aw_fd < 0)
    {
        loge("share ioctl returned negative fd\n");
        goto ALLOC_OUT;
    }

    /* get dmabuf fd */
    fd_data.handle = alloc_data.handle;
    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_MAP, &fd_data);
    if(ret)
    {
        loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.aw_fd);
        goto ALLOC_OUT;
    }

    /* mmap to user */
    addr_vir = (unsigned long)mmap(NULL, alloc_data.aw_len, \
                                   PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.aw_fd, 0);
    if((unsigned long)MAP_FAILED == addr_vir)
    {
        loge("mmap err, ret %d\n", (unsigned int)addr_vir);
        addr_vir = 0;
        goto ALLOC_OUT;
    }
    logv("addr_vir 0x%x",(unsigned int)addr_vir);

    /* get phy address */
    memset(&phys_data, 0, sizeof(phys_data));
    phys_data.handle = alloc_data.handle;
    phys_data.size = size;
    custom_data.aw_cmd = ION_IOC_SUNXI_PHYS_ADDR;
    custom_data.aw_arg = (unsigned long)&phys_data;

    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret)
    {
        loge("ION_IOC_CUSTOM err, ret %d\n", ret);
        addr_phy = 0;
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    addr_phy = phys_data.phys_addr;
    logv("addr_phy 0x%x\n",(unsigned int)addr_phy);
    alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
    if (alloc_buffer == NULL)
    {
        loge("malloc buffer node failed");

        /* unmmap */
        ret = munmap((void*)addr_vir, alloc_data.aw_len);
        if(ret)
        {
            loge("munmap err, ret %d\n", ret);
        }

        /* close dmabuf fd */
        close(fd_data.aw_fd);

        /* free buffer */
        handle_data.handle = alloc_data.handle;
        ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_FREE, &handle_data);

        if(ret)
        {
            loge("ION_IOC_FREE err, ret %d\n", ret);
        }

        addr_phy = 0;
        addr_vir = 0;        // value of MAP_FAILED is -1, should return 0

        goto ALLOC_OUT;
    }
    alloc_buffer->phy     = addr_phy;
    alloc_buffer->vir     = addr_vir;
    alloc_buffer->user_virt = addr_vir;
    alloc_buffer->size    = size;
    alloc_buffer->fd_data.handle = fd_data.handle;
    alloc_buffer->fd_data.aw_fd = fd_data.aw_fd;
    alloc_buffer->share_data.handle = share_data.handle;
    alloc_buffer->share_data.aw_fd = share_data.aw_fd;

    //logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d", addr_phy, addr_vir, size);

    CdxListAddTail(&alloc_buffer->node, &g_alloc_context->list);

    //------start-----------------
#if DEBUG_ION_REF==1
    cdx_use_mem += size;
    logd("++++++cdx_use_mem = [%d MB], increase size->[%d B], addr_vir=[0x%x], addr_phy=[0x%x]", \
         cdx_use_mem/1024/1024, size, addr_vir, addr_phy);
    int i = 0;

    for(i=0; i<ION_BUF_LEN; i++)
    {
        if(ion_buf_nodes_test[i].addr == 0 && ion_buf_nodes_test[i].size == 0)
        {
            ion_buf_nodes_test[i].addr = addr_vir;
            ion_buf_nodes_test[i].size = size;
            break;
        }
    }

    if(i>= ION_BUF_LEN)
    {
        loge("error, ion buf len is large than [%d]", ION_BUF_LEN);
    }
#endif
//--------------------------------

ALLOC_OUT:
    pthread_mutex_unlock(&g_mutex_alloc);
    return (void*)addr_vir;
}

void CdxIonPfree(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = (unsigned long)pbuf;
    buffer_node * tmp;
    int ret;
    struct ion_handle_data handle_data;

    if (0 == pbuf)
    {
        loge("can not free NULL buffer \n");
        return ;
    }

    pthread_mutex_lock(&g_mutex_alloc);

    if (g_alloc_context == NULL)
    {
        loge("ion_alloc do not opened, should call ion_alloc_open() \
            before ion_alloc_alloc(size) \n");
        return ;
    }

    CdxListForEachEntry(tmp, &g_alloc_context->list, node)
    {
        if (tmp->vir == addr_vir)
        {
            logv("ion_alloc_free item phy= 0x%lx vir= 0x%lx, size= %d \n", \
                 tmp->phy, tmp->vir, tmp->size);
            /*unmap user space*/
            //if (munmap(pbuf, tmp->size) < 0)
            if (munmap((void *)(tmp->user_virt), tmp->size) < 0)
            {
                loge("munmap 0x%p, size: %d failed \n", (void*)addr_vir, tmp->size);
            }

            /*close dma buffer fd*/
            close(tmp->fd_data.aw_fd);
            /* free buffer */
            handle_data.handle = tmp->fd_data.handle;

            ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_FREE, &handle_data);
            if (ret)
            {
                logv("TON_IOC_FREE failed \n");
            }

            close(tmp->share_data.aw_fd);

            CdxListDel(&tmp->node);
            free(tmp);

            flag = 1;

            //------start-----------------
#if DEBUG_ION_REF==1
            int i = 0;
            for(i=0; i<ION_BUF_LEN; i++)
            {
                if(ion_buf_nodes_test[i].addr == addr_vir && ion_buf_nodes_test[i].size > 0)
                {

                    cdx_use_mem -= ion_buf_nodes_test[i].size;
                    logd("--------cdx_use_mem = [%d MB], reduce size->[%d B]",\
                         cdx_use_mem/1024/1024, ion_buf_nodes_test[i].size);
                    ion_buf_nodes_test[i].addr = 0;
                    ion_buf_nodes_test[i].size = 0;

                    break;
                }
            }

            if(i>= ION_BUF_LEN)
            {
                loge("error, ion buf len is large than [%d]", ION_BUF_LEN);
            }
#endif
            //--------------------------------

            break;
        }
    }

    if (0 == flag)
    {
        loge("ion_alloc_free failed, do not find virtual address: 0x%lx \n", addr_vir);
    }

    pthread_mutex_unlock(&g_mutex_alloc);
    return ;
}

void* CdxIonVir2Phy(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = (unsigned long)pbuf;
    unsigned long addr_phy = 0;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        // logv("can not vir2phy NULL buffer \n");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc);

    CdxListForEachEntry(tmp, &g_alloc_context->list, node)
    {
        if (addr_vir >= tmp->vir
                && addr_vir < tmp->vir + tmp->size)
        {
            addr_phy = tmp->phy + addr_vir - tmp->vir;
            // logv("ion_alloc_vir2phy phy= 0x%08x vir= 0x%08x \n", addr_phy, addr_vir);
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        loge("ion_alloc_vir2phy failed, do not find virtual address: 0x%lx \n", addr_vir);
    }

    pthread_mutex_unlock(&g_mutex_alloc);

    return (void*)addr_phy;
}

int CdxIonPhy2ShareFd(void * pbuf)
{
    int flag = 0;
    int share_fd = 0;
    unsigned long addr_phy = (unsigned long)pbuf;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        logd("can not phy2vir NULL buffer \n");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc);

    CdxListForEachEntry(tmp, &g_alloc_context->list, node)
    {
        if (addr_phy >= tmp->phy
                && addr_phy < tmp->phy + tmp->size)
        {
            share_fd = tmp->share_data.aw_fd;
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        logd("ion_alloc_phy2vir failed, do not find physical address: 0x%lx \n", addr_phy);
    }

    pthread_mutex_unlock(&g_mutex_alloc);

    return share_fd;
}

int CdxIonVir2Fd(void * pbuf)
{
    int flag = 0;
    int fd = 0;
    unsigned long addr_vir = (unsigned long)pbuf;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        logd("can not phy2vir NULL buffer");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc);

    CdxListForEachEntry(tmp, &g_alloc_context->list, node)
    {
        if (addr_vir >= tmp->vir
                && addr_vir < tmp->vir + tmp->size)
        {
            fd = tmp->fd_data.aw_fd;
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        logd("CdxIonVir2Fd failed, do not find virtual address: 0x%lx", addr_vir);
    }

    pthread_mutex_unlock(&g_mutex_alloc);

    return fd;
}

void* CdxIonPhy2Vir(void * pbuf)
{
    int flag = 0;
    unsigned long addr_vir = 0;
    unsigned long addr_phy = (unsigned long)pbuf;
    buffer_node * tmp;

    if (0 == pbuf)
    {
        loge("can not phy2vir NULL buffer \n");
        return 0;
    }

    pthread_mutex_lock(&g_mutex_alloc);

    CdxListForEachEntry(tmp, &g_alloc_context->list, node)
    {
        if (addr_phy >= tmp->phy
                && addr_phy < tmp->phy + tmp->size)
        {
            addr_vir = tmp->vir + addr_phy - tmp->phy;
            flag = 1;
            break;
        }
    }

    if (0 == flag)
    {
        loge("ion_alloc_phy2vir failed, do not find physical address: 0x%lx \n", addr_phy);
    }

    pthread_mutex_unlock(&g_mutex_alloc);

    return (void*)addr_vir;
}

#ifdef CONF_KERNEL_VERSION_3_10
void CdxIonFlushCache(void* startAddr, int size)
{
    sunxi_cache_range range;
    int ret;

    /* clean and invalid user cache */
    range.start = (unsigned long)startAddr;
    range.end = (unsigned long)startAddr + size;

    ret = ioctl(g_alloc_context->fd, ION_IOC_SUNXI_FLUSH_RANGE, &range);
    if (ret)
    {
        loge("ION_IOC_SUNXI_FLUSH_RANGE failed \n");
    }

    return;
}
#else
void CdxIonFlushCache(void* startAddr, int size)
{
    sunxi_cache_range range;
    struct aw_ion_custom_info custom_data;
    int ret;

    /* clean and invalid user cache */
    range.start = (unsigned long)startAddr;
    range.end = (unsigned long)startAddr + size;

    custom_data.aw_cmd = ION_IOC_SUNXI_FLUSH_RANGE;
    custom_data.aw_arg = (unsigned long)&range;

    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if (ret)
    {
        loge("ION_IOC_CUSTOM failed \n");
    }

    return;
}
#endif

void CdxIonFlushAll()
{
    ioctl(g_alloc_context->fd, ION_IOC_SUNXI_FLUSH_ALL, 0);
}

void* CdxIonDrm(int size)
{
    aw_ion_allocation_info_t alloc_data;
    ion_fd_data_t fd_data;
    struct ion_handle_data handle_data;
    struct aw_ion_custom_info custom_data;
    sunxi_phys_data   phys_data, tee_data;

    int rest_size = 0;
    unsigned long addr_phy = 0;
    unsigned long addr_vir = 0;
    unsigned long addr_tee = 0;
    buffer_node * alloc_buffer = NULL;
    int ret = 0;

    pthread_mutex_lock(&g_mutex_alloc);

    if (g_alloc_context == NULL)
    {
        loge("ion_alloc do not opened, should call ion_alloc_open() \
            before ion_alloc_alloc(size) \n");
        goto ALLOC_OUT;
    }

    if(size <= 0)
    {
        logv("can not alloc size 0 \n");
        goto ALLOC_OUT;
    }

    /*alloc buffer*/
    alloc_data.aw_len = size;
    alloc_data.aw_align = ION_ALLOC_ALIGN ;
    alloc_data.aw_heap_id_mask = AW_ION_SECURE_HEAP_MASK;
    alloc_data.flags = AW_ION_CACHED_FLAG | AW_ION_CACHED_NEEDS_SYNC_FLAG;
    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_ALLOC, &alloc_data);
    if (ret)
    {
        loge("ION_IOC_ALLOC error %s \n", strerror(errno));
        goto ALLOC_OUT;
    }

    /* get dmabuf fd */
    fd_data.handle = alloc_data.handle;
    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_MAP, &fd_data);
    if(ret)
    {
        loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.aw_fd);
        goto ALLOC_OUT;
    }

    /* mmap to user */
    addr_vir = (unsigned long)mmap(NULL, alloc_data.aw_len, \
                                   PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.aw_fd, 0);
    if((unsigned long)MAP_FAILED == addr_vir)
    {
        //loge("mmap err, ret %d\n", (unsigned int)addr_vir);
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    /* get phy address */
    memset(&phys_data, 0, sizeof(phys_data));
    phys_data.handle = alloc_data.handle;
    phys_data.size = size;
    custom_data.aw_cmd = ION_IOC_SUNXI_PHYS_ADDR;
    custom_data.aw_arg = (unsigned long)&phys_data;

    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret)
    {
        loge("ION_IOC_CUSTOM err, ret %d\n", ret);
        addr_phy = 0;
        addr_vir = 0;
        goto ALLOC_OUT;
    }

    addr_phy = phys_data.phys_addr;
#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    memset(&tee_data, 0, sizeof(tee_data));
    tee_data.handle = alloc_data.handle;
    tee_data.size = size;
    custom_data.aw_cmd = ION_IOC_SUNXI_TEE_ADDR;
    custom_data.aw_arg = (unsigned long)&tee_data;
    ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_CUSTOM, &custom_data);
    if(ret)
    {
        loge("ION_IOC_CUSTOM err, ret %d\n", ret);
        addr_phy = 0;
        addr_vir = 0;
        goto ALLOC_OUT;
    }
    addr_tee = tee_data.phys_addr;
#else
    addr_tee = addr_vir;
#endif
    alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
    if (alloc_buffer == NULL)
    {
        loge("malloc buffer node failed");

        /* unmmap */
        ret = munmap((void*)addr_vir, alloc_data.aw_len);
        if(ret)
        {
            loge("munmap err, ret %d\n", ret);
        }

        /* close dmabuf fd */
        close(fd_data.aw_fd);

        /* free buffer */
        handle_data.handle = alloc_data.handle;
        ret = ioctl(g_alloc_context->fd, AW_MEM_ION_IOC_FREE, &handle_data);

        if(ret)
        {
            loge("ION_IOC_FREE err, ret %d\n", ret);
        }

        addr_phy = 0;
        addr_vir = 0;        // value of MAP_FAILED is -1, should return 0

        goto ALLOC_OUT;
    }


    alloc_buffer->size        = size;
    alloc_buffer->phy         = addr_phy;
    alloc_buffer->user_virt = addr_vir;
    alloc_buffer->vir         = addr_tee;
    alloc_buffer->tee         = addr_tee;
    alloc_buffer->fd_data.handle = fd_data.handle;
    alloc_buffer->fd_data.aw_fd = fd_data.aw_fd;

    CdxListAddTail(&alloc_buffer->node, &g_alloc_context->list);

ALLOC_OUT:

    pthread_mutex_unlock(&g_mutex_alloc);

    return (void*)addr_tee;
}

//return total meminfo with MB
int CdxIonGetTotalSize()
{
    int ret = 0;

    int ion_fd = open(DEV_NAME, O_WRONLY);

    if (ion_fd < 0)
    {
        loge("open ion dev failed, cannot get ion mem.");
        goto err;
    }

    struct sunxi_pool_info binfo =
    {
        .total   = 0,    // mb
        .free_kb  = 0, //the same to free_mb
        .free_mb = 0,
    };

    struct aw_ion_custom_info cdata;

    cdata.aw_cmd = ION_IOC_SUNXI_POOL_INFO;
    cdata.aw_arg = (unsigned long)&binfo;
    ret = ioctl(ion_fd,AW_MEM_ION_IOC_CUSTOM, &cdata);
    if (ret < 0)
    {
        loge("Failed to ioctl ion device, errno:%s\n", strerror(errno));
        goto err;
    }

    logd(" ion dev get free pool [%d MB], total [%d MB]\n", binfo.free_mb, binfo.total / 1024);
    ret = binfo.total;
err:
    if(ion_fd > 0)
    {
        close(ion_fd);
    }
    return ret;
}
