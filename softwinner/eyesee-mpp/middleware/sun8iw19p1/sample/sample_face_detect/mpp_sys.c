#include "mpp_sys.h"

int mpp_init()
{
	// 初始化MPP
    MPP_SYS_CONF_S mSysConf;
    memset(&mSysConf, 0, sizeof(MPP_SYS_CONF_S));
    mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&mSysConf);
    AW_MPI_SYS_Init();
	
	return 0;
}

int mpp_destroy()
{
	AW_MPI_SYS_Exit();
	return 0;
}

int64_t mpp_getCurTimeUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}