/*
 * ion_alloc.c
 *
 * john.fu@allwinnertech.com
 *
 * ion memory allocate
 *
 */

#include "config.h"
#include "log.h"
#include "ion_alloc.h"
#include "ion_alloc_list.h"
#include <sys/ioctl.h>

#include <errno.h>


#define DEBUG_ION_REF 0	//just for H3 ION memery info debug

#define ION_ALLOC_ALIGN	SZ_4k

#define DEV_NAME					"/dev/ion"
#define ION_IOC_SUNXI_POOL_INFO		10

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

	struct sunxi_pool_info {
		unsigned int total;     //unit kb
		unsigned int free_kb;  // size kb
		unsigned int free_mb;  // size mb
	};


//return total meminfo with MB
int get_ion_total_mem()
{
	int ret = 0;

	int ion_fd = open(DEV_NAME, O_WRONLY);

	if (ion_fd < 0) {
		loge("open ion dev failed, cannot get ion mem.");
		goto err;
	}

	struct sunxi_pool_info binfo = {
		.total   = 0,	// mb
		.free_kb  = 0, //the same to free_mb
		.free_mb = 0,
	};

	struct ion_custom_data cdata;

	cdata.cmd = ION_IOC_SUNXI_POOL_INFO;
	cdata.arg = (unsigned long)&binfo;
	ret = ioctl(ion_fd,ION_IOC_CUSTOM, &cdata);
	if (ret < 0){
		loge("Failed to ioctl ion device, errno:%s\n", strerror(errno));
		goto err;
	}

	logd(" ion dev get free pool [%u MB], total [%u MB]\n", binfo.free_mb, binfo.total / 1024);
	ret = binfo.total;
err:
	if(ion_fd > 0){
		close(ion_fd);
	}
	return ret;
}

typedef struct BUFFER_NODE
{
	struct list_head i_list;
	unsigned long phy;		//phisical address
	unsigned long vir;		//virtual address
	unsigned int size;		//buffer size
#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1689 && CONFIG_CHIP != OPTION_CHIP_1701)
	int dmabuf_fd;	//dma_buffer fd
	ion_user_handle_t handle;		//alloc data handle
#else
	struct ion_fd_data fd_data;
#endif
}buffer_node;

typedef struct ION_ALLOC_CONTEXT
{
	int					fd;			// driver handle
	struct list_head	list;		// buffer list
	int					ref_cnt;	// reference count
}ion_alloc_context;

ion_alloc_context	*	g_alloc_context = NULL;
pthread_mutex_t			g_mutex_alloc = PTHREAD_MUTEX_INITIALIZER;

/*funciton begin*/
int ion_alloc_open()
{
	logd("begin ion_alloc_open \n");

	pthread_mutex_lock(&g_mutex_alloc);
	if (g_alloc_context != NULL)
	{
		logd("ion allocator has already been created \n");
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
		logd("pid: %d, g_alloc_context = %p \n", getpid(), g_alloc_context);
	}

	memset((void*)g_alloc_context, 0, sizeof(ion_alloc_context));
#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673)
	g_alloc_context->fd = open(DEV_NAME, O_RDWR, 0);
#else
	g_alloc_context->fd = open(DEV_NAME, /*O_RDWR */O_RDONLY, 0);
#endif
	if (g_alloc_context->fd <= 0)
	{
		loge("open %s failed \n", DEV_NAME);
		goto ERROR_OUT;
	}

#if DEBUG_ION_REF==1
	cdx_use_mem = 0;
	memset(&ion_buf_nodes_test, sizeof(ion_buf_nodes_test), 0);
	logd("ion_open, cdx_use_mem=[%dByte].", cdx_use_mem);
	get_ion_total_mem();
#endif

	INIT_LIST_HEAD(&g_alloc_context->list);

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

int ion_alloc_close()
{
	struct list_head * pos, *q;
	buffer_node * tmp;

	logv("ion_alloc_close \n");

	pthread_mutex_lock(&g_mutex_alloc);
	if (--g_alloc_context->ref_cnt <= 0)
	{
		logv("pid: %d, release g_alloc_context = %p \n", getpid(), g_alloc_context);

		list_for_each_safe(pos, q, &g_alloc_context->list)
		{
			tmp = list_entry(pos, buffer_node, i_list);
			logv("ion_alloc_close del item phy= 0x%lx vir= 0x%lx, size= %d \n", tmp->phy, tmp->vir, tmp->size);
			list_del(pos);
			free(tmp);
		}

#if DEBUG_ION_REF==1
		logd("ion_close, cdx_use_mem=[%d MB]", cdx_use_mem/1024/1024);
		get_ion_total_mem();
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
		if(ion_buf_nodes_test[i].addr != 0 || ion_buf_nodes_test[i].size != 0){

			loge("ion mem leak????  addr->[0x%x], leak size->[%dByte]", ion_buf_nodes_test[i].addr, ion_buf_nodes_test[i].size);
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


	return 0;
}

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1689 && CONFIG_CHIP != OPTION_CHIP_1701)
// return virtual address: 0 failed
unsigned long ion_alloc_alloc(int size)
{
	int rest_size = 0;
	unsigned long addr_phy = 0;
	unsigned long addr_vir = 0;
	buffer_node * alloc_buffer = NULL;
	logd("0-ion_alloc\n");

	ion_allocation_data_t alloc_data;
	ion_handle_data_t handle_data;
	ion_custom_data_t custom_data;
	ion_fd_data_t fd_data;
	sunxi_phys_data phys_data;
	int fd, ret = 0;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}

	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}

	/*alloc buffer*/
	alloc_data.len = size;
	alloc_data.align = ION_ALLOC_ALIGN ;
	alloc_data.heap_id_mask = ION_HEAP_CARVEOUT_MASK;
	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logv("ION_IOC_ALLOC error \n");
		goto ALLOC_OUT;
	}

	/*get physical address*/
	phys_data.handle = alloc_data.handle;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;
	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if (ret)
	{
		logv("ION_IOC_CUSTOM failed \n");
		goto out1;
	}
	addr_phy = phys_data.phys_addr;

	/*get dma buffer fd*/
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if (ret)
	{
		logv("ION_IOC_MAP failed \n");
		goto out1;
	}

	/*mmap to user space*/
	addr_vir = (unsigned long)mmap(NULL, alloc_data.len, PROT_READ | PROT_WRITE, MAP_SHARED,
					fd_data.fd, 0);
	if ((unsigned long)MAP_FAILED == addr_vir)
	{
		logv("mmap fialed \n");
		goto out2;
	}

	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		logv("malloc buffer node failed \n");
		goto out3;
	}
	alloc_buffer->size	    = size;
	alloc_buffer->phy	    = addr_phy;
	alloc_buffer->vir	    = addr_vir;
	alloc_buffer->handle    = alloc_data.handle;
	alloc_buffer->dmabuf_fd = fd_data.fd;

	logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d \n", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

//------start-----------------
#if DEBUG_ION_REF==1
	cdx_use_mem += size;
	logd("++++++cdx_use_mem = [%d MB], increase size->[%d B], addr_vir=[0x%x], addr_phy=[0x%x]", cdx_use_mem/1024/1024, size, addr_vir, addr_phy);
	int i = 0;
	for(i=0; i<ION_BUF_LEN; i++)
	{
		if(ion_buf_nodes_test[i].addr == 0 && ion_buf_nodes_test[i].size == 0){
			ion_buf_nodes_test[i].addr = addr_vir;
			ion_buf_nodes_test[i].size = size;
			break;
		}
	}

	if(i>= ION_BUF_LEN){
		loge("error, ion buf len is large than [%d]", ION_BUF_LEN);
	}
#endif
//--------------------------------

	goto ALLOC_OUT;
out3:
	/* unmmap */
	ret = munmap((void*)addr_vir, alloc_data.len);
	if(ret) printf("munmap err, ret %d\n", ret);
	printf("munmap succes\n");

out2:
	/* close dmabuf fd */
	close(fd_data.fd);
	printf("close dmabuf fd succes\n");

out1:
	/* free buffer */
	handle_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);
	if(ret)
		printf("ION_IOC_FREE err, ret %d\n", ret);
	printf("ION_IOC_FREE succes\n");

ALLOC_OUT:

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

#else
// return virtual address: 0 failed
unsigned long ion_alloc_alloc(int size)
{
	struct ion_allocation_data alloc_data;
	struct ion_fd_data fd_data;
	struct ion_handle_data handle_data;
	struct ion_custom_data custom_data;
	sunxi_phys_data   phys_data;

	int rest_size = 0;
	unsigned long addr_phy = 0;
	unsigned long addr_vir = 0;
	buffer_node * alloc_buffer = NULL;
    int ret = 0;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logd("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}

	if(size <= 0)
	{
		logd("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}

	/*alloc buffer*/
	alloc_data.len = (size_t)size;
	alloc_data.align = ION_ALLOC_ALIGN ;

#if ((CONFIG_PRODUCT == OPTION_PRODUCT_PAD && CONFIG_CHIP == OPTION_CHIP_1639) || CONFIG_CHIP == OPTION_CHIP_1689  || CONFIG_CHIP == OPTION_CHIP_1701)
	alloc_data.heap_id_mask = ION_HEAP_DMA_MASK;
#else
	alloc_data.heap_id_mask = ION_HEAP_CARVEOUT_MASK;
#endif

	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;

	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logd("ION_IOC_ALLOC error \n");
		goto ALLOC_OUT;
	}

	/* get dmabuf fd */
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if(ret)
	{
		loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.fd);
		goto ALLOC_OUT;
	}


	/* mmap to user */
	addr_vir = (unsigned long)mmap(NULL, alloc_data.len, PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
	if((unsigned long)MAP_FAILED == addr_vir)
	{
		loge("mmap err, ret %d\n", (unsigned int)addr_vir);
		addr_vir = 0;
		goto ALLOC_OUT;
	}
//	logd("mmap succes, get user_addr 0x%08x\n", (unsigned int)addr_vir);

    /* get phy address */
	memset(&phys_data, 0, sizeof(phys_data));
	phys_data.handle = alloc_data.handle;
	phys_data.size = size;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if(ret) {
		loge("ION_IOC_CUSTOM err, ret %d\n", ret);
		addr_phy = 0;
		addr_vir = 0;
		goto ALLOC_OUT;
	}

//	logv("get phys_data.phys_addr: %x\n", phys_data.phys_addr);

	addr_phy = phys_data.phys_addr;
	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		loge("malloc buffer node failed");

		/* unmmap */
		ret = munmap((void*)addr_vir, alloc_data.len);
		if(ret) {
			//loge("munmap err, ret %d\n", ret);
		}

		/* close dmabuf fd */
		close(fd_data.fd);

		/* free buffer */
		handle_data.handle = alloc_data.handle;
		ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);

		if(ret) {
			loge("ION_IOC_FREE err, ret %d\n", ret);
		}

		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0

		goto ALLOC_OUT;
	}
	alloc_buffer->phy	= addr_phy;
	alloc_buffer->vir	= addr_vir;
	alloc_buffer->size	= size;
	alloc_buffer->fd_data.handle = fd_data.handle;
	alloc_buffer->fd_data.fd = fd_data.fd;

	logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

ALLOC_OUT:

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

#endif

int ion_alloc_free(void * pbuf)
{
	int flag = 0;
	unsigned long addr_vir = (unsigned long)pbuf;
	buffer_node * tmp;
	int ret;
	struct ion_handle_data handle_data;
	int nFreeSize = 0;

	if (0 == pbuf)
	{
		loge("can not free NULL buffer \n");
		return 0;
	}

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		return 0;
	}

	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
	{
		if (tmp->vir == addr_vir)
		{
			logv("ion_alloc_free item phy= 0x%lx vir= 0x%lx, size= %d \n", tmp->phy, tmp->vir, tmp->size);
			/*unmap user space*/
			if (munmap(pbuf, tmp->size) < 0)
			{
				loge("munmap 0x%p, size: %u failed \n", (void*)addr_vir, tmp->size);
			}
			nFreeSize = tmp->size;

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1689 && CONFIG_CHIP != OPTION_CHIP_1701)
			/*close dma buffer fd*/
			close(tmp->dmabuf_fd);

			/*free memory handle*/
			handle_data.handle = tmp->handle;
#else
			/*close dma buffer fd*/
			close(tmp->fd_data.fd);

			/* free buffer */
			handle_data.handle = tmp->fd_data.handle;
#endif

			ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);
			if (ret)
			{
				logv("TON_IOC_FREE failed \n");
			}

			list_del(&tmp->i_list);
			free(tmp);

			flag = 1;

			//------start-----------------
#if DEBUG_ION_REF==1
			int i = 0;
			for(i=0; i<ION_BUF_LEN; i++)
			{
				if(ion_buf_nodes_test[i].addr == addr_vir && ion_buf_nodes_test[i].size > 0){

					cdx_use_mem -= ion_buf_nodes_test[i].size;
					logd("--------cdx_use_mem = [%d MB], reduce size->[%d B]", cdx_use_mem/1024/1024, ion_buf_nodes_test[i].size);
					ion_buf_nodes_test[i].addr = 0;
					ion_buf_nodes_test[i].size = 0;

					break;
				}
			}

			if(i>= ION_BUF_LEN){
				loge("error, ion buf len is large than [%d]", ION_BUF_LEN);
			}
#endif
			//--------------------------------

			break;
		}
	}

	if (0 == flag)
	{
		logv("ion_alloc_free failed, do not find virtual address: 0x%lx \n", addr_vir);
	}

	pthread_mutex_unlock(&g_mutex_alloc);
	return nFreeSize;
}

unsigned long ion_alloc_vir2phy(void * pbuf)
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

	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
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
		logv("ion_alloc_vir2phy failed, do not find virtual address: 0x%lx \n", addr_vir);
	}

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_phy;
}

unsigned long ion_alloc_phy2vir(void * pbuf)
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

	list_for_each_entry(tmp, &g_alloc_context->list, i_list)
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
		logv("ion_alloc_phy2vir failed, do not find physical address: 0x%lx \n", addr_phy);
	}

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}

#if (CONFIG_CHIP == OPTION_CHIP_1689 || CONFIG_CHIP == OPTION_CHIP_1701)
void ion_flush_cache(void* startAddr, int size)
{
	sunxi_cache_range range;
	int ret;

	/* clean and invalid user cache */
	range.start = (unsigned long)startAddr;
	range.end = (unsigned long)startAddr + size;

	ret = ioctl(g_alloc_context->fd, ION_IOC_SUNXI_FLUSH_RANGE, &range);
	if (ret)
	{
		logv("ION_IOC_SUNXI_FLUSH_RANGE failed \n");
	}

	return;
}
#else
void ion_flush_cache(void* startAddr, int size)
{
	sunxi_cache_range range;
	struct ion_custom_data custom_data;
	int ret;

	/* clean and invalid user cache */
	range.start = (unsigned long)startAddr;
	range.end = (unsigned long)startAddr + size;

	custom_data.cmd = ION_IOC_SUNXI_FLUSH_RANGE;
	custom_data.arg = (unsigned long)&range;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if (ret)
	{
		logv("ION_IOC_CUSTOM failed \n");
	}

	return;
}
#endif

void ion_flush_cache_all()
{
	ioctl(g_alloc_context->fd, ION_IOC_SUNXI_FLUSH_ALL, 0);
}

unsigned long ion_alloc_alloc_drm(int size)
{
	struct ion_allocation_data alloc_data;
	struct ion_fd_data fd_data;
	struct ion_handle_data handle_data;
	struct ion_custom_data custom_data;
	sunxi_phys_data   phys_data;

	int rest_size = 0;
	unsigned long addr_phy = 0;
	unsigned long addr_vir = 0;
	buffer_node * alloc_buffer = NULL;
	int ret = 0;

	pthread_mutex_lock(&g_mutex_alloc);

	if (g_alloc_context == NULL)
	{
		logv("ion_alloc do not opened, should call ion_alloc_open() before ion_alloc_alloc(size) \n");
		goto ALLOC_OUT;
	}

	if(size <= 0)
	{
		logv("can not alloc size 0 \n");
		goto ALLOC_OUT;
	}

	/*alloc buffer*/
	alloc_data.len = size;
	alloc_data.align = ION_ALLOC_ALIGN ;
	alloc_data.heap_id_mask = ION_HEAP_SECURE_MASK;
	alloc_data.flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
	ret = ioctl(g_alloc_context->fd, ION_IOC_ALLOC, &alloc_data);
	if (ret)
	{
		logv("ION_IOC_ALLOC error %s \n", strerror(errno));
		goto ALLOC_OUT;
	}

	/* get dmabuf fd */
	fd_data.handle = alloc_data.handle;
	ret = ioctl(g_alloc_context->fd, ION_IOC_MAP, &fd_data);
	if(ret)
	{
		//loge("ION_IOC_MAP err, ret %d, dmabuf fd 0x%08x\n", ret, (unsigned int)fd_data.fd);
		goto ALLOC_OUT;
	}


	/* mmap to user */
	addr_vir = (unsigned long)mmap(NULL, alloc_data.len, PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
	if((unsigned long)MAP_FAILED == addr_vir)
	{
		//loge("mmap err, ret %d\n", (unsigned int)addr_vir);
		addr_vir = 0;
		goto ALLOC_OUT;
	}
	//	logd("mmap succes, get user_addr 0x%08x\n", (unsigned int)addr_vir);

	/* get phy address */
	memset(&phys_data, 0, sizeof(phys_data));
	phys_data.handle = alloc_data.handle;
	phys_data.size = size;
	custom_data.cmd = ION_IOC_SUNXI_PHYS_ADDR;
	custom_data.arg = (unsigned long)&phys_data;

	ret = ioctl(g_alloc_context->fd, ION_IOC_CUSTOM, &custom_data);
	if(ret) {
		//loge("ION_IOC_CUSTOM err, ret %d\n", ret);
		addr_phy = 0;
		addr_vir = 0;
		goto ALLOC_OUT;
	}

	//	logv("get phys_data.phys_addr: %x\n", phys_data.phys_addr);

	addr_phy = phys_data.phys_addr;
	alloc_buffer = (buffer_node *)malloc(sizeof(buffer_node));
	if (alloc_buffer == NULL)
	{
		//loge("malloc buffer node failed");

		/* unmmap */
		ret = munmap((void*)addr_vir, alloc_data.len);
		if(ret) {
			//loge("munmap err, ret %d\n", ret);
		}

		/* close dmabuf fd */
		close(fd_data.fd);

		/* free buffer */
		handle_data.handle = alloc_data.handle;
		ret = ioctl(g_alloc_context->fd, ION_IOC_FREE, &handle_data);

		if(ret) {
			//loge("ION_IOC_FREE err, ret %d\n", ret);
		}

		addr_phy = 0;
		addr_vir = 0;		// value of MAP_FAILED is -1, should return 0

		goto ALLOC_OUT;
	}


	alloc_buffer->size	    = size;
	alloc_buffer->phy	    = addr_phy;
	alloc_buffer->vir	    = addr_vir;

#if (CONFIG_CHIP != OPTION_CHIP_1639 && CONFIG_CHIP != OPTION_CHIP_1673 && CONFIG_CHIP != OPTION_CHIP_1689 && CONFIG_CHIP != OPTION_CHIP_1701)
	alloc_buffer->handle    = alloc_data.handle;
	alloc_buffer->dmabuf_fd = fd_data.fd;
#else
	alloc_buffer->fd_data.handle = fd_data.handle;
	alloc_buffer->fd_data.fd = fd_data.fd;
#endif

	//logv("alloc succeed, addr_phy: 0x%08x, addr_vir: 0x%08x, size: %d", addr_phy, addr_vir, size);

	list_add_tail(&alloc_buffer->i_list, &g_alloc_context->list);

	ALLOC_OUT:

	pthread_mutex_unlock(&g_mutex_alloc);

	return addr_vir;
}
