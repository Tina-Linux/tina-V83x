
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include "memoryAdapter.h"

#include "config.h"
#include "log.h"

#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM

extern int  sunxi_alloc_open(void);
extern int  sunxi_alloc_close(void);
extern unsigned long  sunxi_alloc_alloc(int size);
extern int sunxi_alloc_free(void * pbuf);
extern unsigned long  sunxi_alloc_vir2phy(void * pbuf);
extern unsigned long  sunxi_alloc_phy2vir(void * pbuf);
extern void sunxi_flush_cache(void* startAddr, int size);

#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION

extern int  ion_alloc_open();
extern int  ion_alloc_close();
extern unsigned long ion_alloc_alloc(int size);
extern int  ion_alloc_free(void * pbuf);
extern unsigned long ion_alloc_vir2phy(void * pbuf);
extern unsigned long ion_alloc_phy2vir(void * pbuf);
extern void ion_flush_cache(void* startAddr, int size);

#else

#error "invalid configuration of memory driver."

#endif

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif

#define NODE_DDR_FREQ	"/sys/class/devfreq/sunxi-ddrfreq/cur_freq"
static int readNodeValue(const char *node, char * strVal, size_t size)
{
	int ret = -1;
	int fd = open(node, O_RDONLY);
	if (fd >= 0)
	{
		read(fd, strVal, size);
		close(fd);
		ret = 0;
	}
	return ret;
}

static pthread_mutex_t gMutexMemAdater = PTHREAD_MUTEX_INITIALIZER;
static int gMemAdapterRefCount = 0;

//* open the memory adater.
int MemAdapterOpen(void)
{
    pthread_mutex_lock(&gMutexMemAdater);

    if(gMemAdapterRefCount == 0)
    {
        //* open memory allocate module.
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
        sunxi_alloc_open();
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
        ion_alloc_open();
#else
    #error "invalid configuration of memory driver."
#endif
    }

    gMemAdapterRefCount++;

    pthread_mutex_unlock(&gMutexMemAdater);

    return 0;
}


//* close the memory adapter.
void MemAdapterClose(void)
{
    pthread_mutex_lock(&gMutexMemAdater);

    if(gMemAdapterRefCount <= 0)
    {
        pthread_mutex_unlock(&gMutexMemAdater);
        return;
    }

    gMemAdapterRefCount--;

    if(gMemAdapterRefCount == 0)
    {
        //* close memory alloc module.
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
        sunxi_alloc_close();
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
        ion_alloc_close();
#else
    #error "invalid configuration of memory driver."
#endif
    }

    pthread_mutex_unlock(&gMutexMemAdater);

    return;
}

//* get total ion memory, just for BOX H3
//it should be implement to all platform
int MemAdapterGetTotalMemory()
{
	int total = -1;
#if ((CONFIG_CHIP == OPTION_CHIP_1680) && (CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX))
	total = get_ion_total_mem();
#endif
	return total;
}



//* allocate memory that is physically continue.
void* MemAdapterPalloc(int nSize)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_alloc(nSize);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_alloc(nSize);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* free memory allocated by AdapterMemPalloc()
int  MemAdapterPfree(void* pMem)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return sunxi_alloc_free(pMem);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return ion_alloc_free(pMem);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* synchronize dram and cpu cache.
void  MemAdapterFlushCache(void* pMem, int nSize)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return sunxi_flush_cache(pMem, nSize);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return ion_flush_cache(pMem, nSize);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* get physic address of a memory block allocated by AdapterMemPalloc().
void* MemAdapterGetPhysicAddress(void* pVirtualAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)(sunxi_alloc_vir2phy(pVirtualAddress) - PHY_OFFSET);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)(ion_alloc_vir2phy(pVirtualAddress) - PHY_OFFSET);
#else
    #error "invalid configuration of memory driver."
#endif
}


//* get virtual address with a memory block's physic address.
void* MemAdapterGetVirtualAddress(void* pPhysicAddress)
{
    //* transform the physic address for modules to physic address for cpu.
    long nPhysicAddressForCpu = (long)pPhysicAddress + PHY_OFFSET;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_phy2vir((void*)nPhysicAddressForCpu);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    #error "invalid configuration of memory driver."
#endif
}

//* get cpu physic address of a memory block allocated by AdapterMemPalloc().
//* 'cpu physic address' means the physic address the cpu saw, it is different
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetPhysicAddressCpu(void* pVirtualAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_vir2phy(pVirtualAddress);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_vir2phy(pVirtualAddress);
#else
    #error "invalid configuration of memory driver."
#endif
}

//* get virtual address with a memory block's cpu physic address,
//* 'cpu physic address' means the physic address the cpu saw, it is different
//* to the physic address for the hardware modules like Video Engine and Display Engine,
//* that because the SOC map memory to the CPU and other hardware modules at different address space.
void* MemAdapterGetVirtualAddressCpu(void* pCpuPhysicAddress)
{
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_SUNXIMEM
    return (void*)sunxi_alloc_phy2vir(pCpuPhysicAddress);
#elif CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir(pCpuPhysicAddress);
#else
    #error "invalid configuration of memory driver."
#endif
}

//* get current ddr frequency, if it is too slow, we will cut some spec off.
int MemAdapterGetDramFreq()
{
	char freq_val[8] = {0};
	if (!readNodeValue(NODE_DDR_FREQ, freq_val, 8))
	{
		return atoi(freq_val);
	}

	// unknow freq
	return -1;
}
