#ifndef _ISP_HANDLE_H_V100_
#define _ISP_HANDLE_H_V100_

#include "../../isp.h"
#include "../../include/isp_tuning.h"

/*
 * get current isp id
 * returns isp id, <0 if not init yet
 */
int get_isp_id();
/*
 * get isp version
 */
 void get_isp_version(char* version);

/*
 * init
 * returns 0 if init success, <0 if something went wrong
 */
int init_isp_module();
/*
 * exit
 * returns 0 if init success, <0 if something went wrong
 */
int exit_isp_module();
/*
 * select isp node
 * returns 0 if init success, <0 if something went wrong
 */
int select_isp(int id);

/*
 * convert tuning configs from network to local
 */
void convert_tuning_cfg_to_local(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data);
/*
 * convert tuning configs from local to network
 */
void convert_tuning_cfg_to_network(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data);
/*
 * convert 3a statistics from local to network
 */
void hton_3a_info(void *stat_info, int type);
/*
 * convert 3a statistics from network to local
 */
void ntoh_3a_info(void *stat_info, int type);
/*
 * print 3a statistics
 */
void output_3a_info(const void *stat_info, int type);



#endif /* _ISP_HANDLE_H_V100_ */
