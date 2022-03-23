#ifndef __MPP_OSD__
#define __MPP_OSD__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include <utils/plat_log.h>
#include "media/mpi_sys.h"
#include "osd_helper.h"
#include "mpi_region.h"
#include "BITMAP_S.h"
#include "mm_comm_sys.h"
#include "mm_common.h"

typedef struct _OSD_Params
{
	RGN_HANDLE     mOverlayHandle;

	unsigned char* pTextBuffer;
	RGN_ATTR_S     stRegionDate;
	BITMAP_S       stBmpDate;
	unsigned char  szText[256];
	MPP_CHN_S      mMppChn;
}OSD_Params;


int create_osd(OSD_Params*     pOSDParams);
int update_osd(OSD_Params*     pOSDParams, void* pTime);
int destroy_osd(OSD_Params* pOSDParams);


#endif
