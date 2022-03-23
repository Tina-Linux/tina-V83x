

#ifndef MEM_CTRL_DEF
#define MEM_CTRL_DEF
typedef struct ipuexc_mem_ctrl_info{
	unsigned int size;
    void* data_addr;
}MEM_CTRL,*P_MEM_CTRL;

extern void mem_init(P_MEM_CTRL ptr_info);
extern void mem_deinit(P_MEM_CTRL ptr_info);

extern void mem_alloc(P_MEM_CTRL ptr_info,unsigned int size);
extern void mem_free(P_MEM_CTRL ptr_info);

extern void mem_load(P_MEM_CTRL ptr_info, const char* ptr_path);
extern void mem_load_f(P_MEM_CTRL ptr_info, const char* ptr_path);
extern int mem_load_bin(P_MEM_CTRL ptr_info, const char* ptr_path);
#endif



