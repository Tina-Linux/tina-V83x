#ifndef _EQ_MID_H_
#define _EQ_MID_H_

#define SYNC_TEST_EQ_AND_PLAYER
#define eqlog(fmt, arg...) printf("line<%d>, func<%s> : "fmt"\n", __LINE__, __func__, ##arg)
#define EQ_PROCCESS_MAX_FREQ_NUM	(7)
void eq_init(unsigned int samplerate_tmp, unsigned int chan_tmp);
void eq_mid_proccess(char *buffer, unsigned int len);
void eq_mid_destroy(void);
#endif
