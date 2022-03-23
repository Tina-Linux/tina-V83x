#ifndef __MS_SN__
#define __MS_SN__
#include "ms_common.h"

ms_result_t ms_check_is_air_condition_sn(uint8_t *str , uint8_t *airSN);
ms_result_t ms_sn_decrypt(uint8_t *p_sn_in, int inlen, uint8_t *p_sn_out, int *poutlen);

#endif
