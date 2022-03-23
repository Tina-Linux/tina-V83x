#ifndef __HW_ADAPTOR__
#define __HW_ADAPTOR__
#ifdef __cplusplus
extern "C"{
#endif
	
#define CONV_ACT_SIG      1
#define CONV_ACT_POOL_SIG 2

typedef struct REG_LIST
{
   unsigned int num;
   unsigned int addr[1024];
   unsigned int val[1024];
}REG_LIST,*PREG_LIST;

    void hw_init();
    void hw_reset();
	void hw_deinit();
	void writel(unsigned int reg, unsigned int addr);
	void dwritel(unsigned int reg, unsigned int addr);
	unsigned int readl(unsigned int addr);
	unsigned int readl_sig();

    void dma_mem_alloc(unsigned int size,void** p_vaddr,void** p_paddr);
	void dma_loadin(void* ptr_src_data, unsigned int size, unsigned int addr);
    void dma_loadout(unsigned int addr, unsigned int size, void* ptr_dst_data);
	void dma_mem_free(void* vaddr);
	void delay_us(unsigned int us);
	
#ifdef __cplusplus
}
#endif
#endif
