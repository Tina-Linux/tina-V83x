
#include <stdint.h>
#include <stdlib.h>
#include "cea_vic.h"

typedef struct {
    int vic;
    const char *name;
} cea_vic_t;

#define __DEF_VIC(_vic) \
    {_vic, #_vic}

const cea_vic_t hdmi_cea_vics[] = {
    __DEF_VIC(HDMI_Unkown            ),
    __DEF_VIC(HDMI_640x480p60_4x3    ),
    __DEF_VIC(HDMI_720x480p60_4x3    ),
    __DEF_VIC(HDMI_720x480p60_16x9   ),
    __DEF_VIC(HDMI_1280x720p60_16x9  ),
    __DEF_VIC(HDMI_1920x1080i60_16x9 ),
    __DEF_VIC(HDMI_720x480i60_4x3    ),
    __DEF_VIC(HDMI_720x480i60_16x9   ),
    __DEF_VIC(HDMI_720x240p60_4x3    ),
    __DEF_VIC(HDMI_720x240p60_16x9   ),
    __DEF_VIC(HDMI_2880x480i60_4x3   ),
    __DEF_VIC(HDMI_2880x480i60_16x9  ),
    __DEF_VIC(HDMI_2880x240p60_4x3   ),
    __DEF_VIC(HDMI_2880x240p60_16x9  ),
    __DEF_VIC(HDMI_1440x480p60_4x3   ),
    __DEF_VIC(HDMI_1440x480p60_16x9  ),
    __DEF_VIC(HDMI_1920x1080p60_16x9 ),
    __DEF_VIC(HDMI_720x576p50_4x3    ),
    __DEF_VIC(HDMI_720x576p50_16x9   ),
    __DEF_VIC(HDMI_1280x720p50_16x9  ),
    __DEF_VIC(HDMI_1920x1080i50_16x9 ),
    __DEF_VIC(HDMI_720x576i50_4x3    ),
    __DEF_VIC(HDMI_720x576i50_16x9   ),
    __DEF_VIC(HDMI_720x288p_4x3      ),
    __DEF_VIC(HDMI_720x288p_16x9     ),
    __DEF_VIC(HDMI_2880x576i50_4x3   ),
    __DEF_VIC(HDMI_2880x576i50_16x9  ),
    __DEF_VIC(HDMI_2880x288p50_4x3   ),
    __DEF_VIC(HDMI_2880x288p50_16x9  ),
    __DEF_VIC(HDMI_1440x576p_4x3     ),
    __DEF_VIC(HDMI_1440x576p_16x9    ),
    __DEF_VIC(HDMI_1920x1080p50_16x9 ),
    __DEF_VIC(HDMI_1920x1080p24_16x9 ),
    __DEF_VIC(HDMI_1920x1080p25_16x9 ),
    __DEF_VIC(HDMI_1920x1080p30_16x9 ),
    __DEF_VIC(HDMI_2880x480p60_4x3   ),
    __DEF_VIC(HDMI_2880x480p60_16x9  ),
    __DEF_VIC(HDMI_2880x576p50_4x3   ),
    __DEF_VIC(HDMI_2880x576p50_16x9  ),
    __DEF_VIC(HDMI_1920x1080i_t1250_50_16x9 ),
    __DEF_VIC(HDMI_1920x1080i100_16x9 ),
    __DEF_VIC(HDMI_1280x720p100_16x9  ),
    __DEF_VIC(HDMI_720x576p100_4x3    ),
    __DEF_VIC(HDMI_720x576p100_16x9   ),
    __DEF_VIC(HDMI_720x576i100_4x3    ),
    __DEF_VIC(HDMI_720x576i100_16x9   ),
    __DEF_VIC(HDMI_1920x1080i120_16x9 ),
    __DEF_VIC(HDMI_1280x720p120_16x9  ),
    __DEF_VIC(HDMI_720x480p120_4x3    ),
    __DEF_VIC(HDMI_720x480p120_16x9   ),
    __DEF_VIC(HDMI_720x480i120_4x3    ),
    __DEF_VIC(HDMI_720x480i120_16x9   ),
    __DEF_VIC(HDMI_720x576p200_4x3    ),
    __DEF_VIC(HDMI_720x576p200_16x9   ),
    __DEF_VIC(HDMI_720x576i200_4x3    ),
    __DEF_VIC(HDMI_720x576i200_16x9   ),
    __DEF_VIC(HDMI_720x480p240_4x3    ),
    __DEF_VIC(HDMI_720x480p240_16x9   ),
    __DEF_VIC(HDMI_720x480i240_4x3    ),
    __DEF_VIC(HDMI_720x480i240_16x9   ),

    /* Refet to CEA 861-F */
    __DEF_VIC(HDMI_1280x720p24_16x9     ),
    __DEF_VIC(HDMI_1280x720p25_16x9     ),
    __DEF_VIC(HDMI_1280x720p30_16x9     ),
    __DEF_VIC(HDMI_1920x1080p120_16x9   ),
    __DEF_VIC(HDMI_1920x1080p100_16x9   ),
    __DEF_VIC(HDMI_1280x720p24_64x27    ),
    __DEF_VIC(HDMI_1280x720p25_64x27    ),
    __DEF_VIC(HDMI_1280x720p30_64x27    ),
    __DEF_VIC(HDMI_1280x720p50_64x27    ),
    __DEF_VIC(HDMI_1280x720p60_64x27    ),
    __DEF_VIC(HDMI_1280x720p100_64x27   ),
    __DEF_VIC(HDMI_1280x720p120_64x27   ),
    __DEF_VIC(HDMI_1920x1080p24_64x27   ),
    __DEF_VIC(HDMI_1920x1080p25_64x27   ),
    __DEF_VIC(HDMI_1920x1080p30_64x27   ),
    __DEF_VIC(HDMI_1920x1080p50_64x27   ),
    __DEF_VIC(HDMI_1920x1080p60_64x27   ),
    __DEF_VIC(HDMI_1920x1080p100_64x27  ),
    __DEF_VIC(HDMI_1920x1080p120_64x27  ),
    __DEF_VIC(HDMI_1680x720p24_64x27    ),
    __DEF_VIC(HDMI_1680x720p25_64x27    ),
    __DEF_VIC(HDMI_1680x720p30_64x27    ),
    __DEF_VIC(HDMI_1680x720p50_64x27    ),
    __DEF_VIC(HDMI_1680x720p60_64x27    ),
    __DEF_VIC(HDMI_1680x720p100_64x27   ),
    __DEF_VIC(HDMI_1680x720p120_64x27   ),
    __DEF_VIC(HDMI_2560x1080p24_64x27   ),
    __DEF_VIC(HDMI_2560x1080p25_64x27   ),
    __DEF_VIC(HDMI_2560x1080p30_64x27   ),
    __DEF_VIC(HDMI_2560x1080p50_64x27   ),
    __DEF_VIC(HDMI_2560x1080p60_64x27   ),
    __DEF_VIC(HDMI_2560x1080p100_64x27  ),
    __DEF_VIC(HDMI_2560x1080p120_64x27  ),
    __DEF_VIC(HDMI_3840x2160p24_16x9    ),
    __DEF_VIC(HDMI_3840x2160p25_16x9    ),
    __DEF_VIC(HDMI_3840x2160p30_16x9    ),
    __DEF_VIC(HDMI_3840x2160p50_16x9    ),
    __DEF_VIC(HDMI_3840x2160p60_16x9    ),
    __DEF_VIC(HDMI_4096x2160p24_256x135 ),
    __DEF_VIC(HDMI_4096x2160p25_256x135 ),
    __DEF_VIC(HDMI_4096x2160p30_256x135 ),
    __DEF_VIC(HDMI_4096x2160p50_256x135 ),
    __DEF_VIC(HDMI_4096x2160p60_256x135 ),
    __DEF_VIC(HDMI_3840x2160p24_64x27   ),
    __DEF_VIC(HDMI_3840x2160p25_64x27   ),
    __DEF_VIC(HDMI_3840x2160p30_64x27   ),
    __DEF_VIC(HDMI_3840x2160p50_64x27   ),
    __DEF_VIC(HDMI_3840x2160p60_64x27   ),
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
const char *getCeaVicName(int vic) {
    for (size_t i = 0; i < ARRAY_SIZE(hdmi_cea_vics); ++i) {
        if (hdmi_cea_vics[i].vic == vic)
            return hdmi_cea_vics[i].name;
    }
    return NULL;
}
