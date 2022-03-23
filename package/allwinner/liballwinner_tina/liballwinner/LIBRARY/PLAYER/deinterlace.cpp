#include <deinterlace.h>
#include <deinterlaceHw.h>

#if (CONFIG_DEINTERLACE == OPTION_SW_DEINTERLACE)
#include <sw-deinterlace/deinterlaceSw.h>
#endif

Deinterlace *DeinterlaceCreate()
{
    Deinterlace *di = NULL;

#if (CONFIG_DEINTERLACE == OPTION_HW_DEINTERLACE)
    di = new DeinterlaceHw();
#elif (CONFIG_DEINTERLACE == OPTION_SW_DEINTERLACE)
    /* PD should add sw interlace here... */
    di = new DeinterlaceSw();
#endif
    return di;
}
