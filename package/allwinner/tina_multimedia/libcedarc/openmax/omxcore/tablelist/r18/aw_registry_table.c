/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : Aw_registry_table.c
* Description :
* History :
*   Author  : AL3
*   Date    : 2013/05/05
*   Comment : complete the design code
*/

/*============================================================================
                            O p e n M A X   w r a p p e r s
                             O p e n  M A X   C o r e

  This module contains the registry table for the QCOM's OpenMAX core.

*//*========================================================================*/

#include "aw_omx_core.h"

#define OMX_VDEC_LIB_NAME  "libOmxVdec.so"
#define OMX_VENC_LIB_NAME  "libOmxVenc.so"
#define OMX_ADEC_LIB_NAME  "libOmxAdec.so"

OmxCoreType core[] =
{
    //*1. mjpeg decoder
    {
        "OMX.allwinner.video.decoder.mjpeg",
        NULL,// Create instance function
        // Unique instance handle
        {
            NULL,NULL,NULL,NULL
        },
        NULL, // Shared object library handle
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.mjpeg"
        }
    },

    //*2. mpeg1 decoder
    {
        "OMX.allwinner.video.decoder.mpeg1",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.mpeg1"
        }
    },

    //*3. mpeg2 decoder
    {
        "OMX.allwinner.video.decoder.mpeg2",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.mpeg2"
        }
    },

    //*4. mpeg4 decoder
    {
        "OMX.allwinner.video.decoder.mpeg4",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.mpeg4"
        }
    },

    //*5. msmpeg4v1 decoder
    {
        "OMX.allwinner.video.decoder.msmpeg4v1",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.msmpeg4v1"
        }
    },

    //*6. msmpeg4v2 decoder
    {
        "OMX.allwinner.video.decoder.msmpeg4v2",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.msmpeg4v2"
        }
    },

    //*7. divx decoder
    {
        "OMX.allwinner.video.decoder.divx",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.divx"
        }
    },

    //*8. xvid decoder
    {
        "OMX.allwinner.video.decoder.xvid",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.xvid"
        }
    },
#if 0
    //*9. h263 decoder
    {
        "OMX.allwinner.video.decoder.h263",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.h263"
        }
    },
#endif
    //*10. h263 decoder
    {
        "OMX.allwinner.video.decoder.s263",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.s263"
        }
    },

    //*11. rxg2 decoder
    {
        "OMX.allwinner.video.decoder.rxg2",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.rxg2"
        }
    },

    //*12. wmv1 decoder
    {
        "OMX.allwinner.video.decoder.wmv1",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.wmv1"
        }
    },

    //*13. wmv2 decoder
    {
        "OMX.allwinner.video.decoder.wmv2",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.wmv2"
        }
    },

    //*14. vc1 decoder
    {
        "OMX.allwinner.video.decoder.vc1",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.vc1"
        }
    },

    //*15. vp6 decoder
    {
        "OMX.allwinner.video.decoder.vp6",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.vp6"
        }
    },

    //*16. vp8 decoder
    {
        "OMX.allwinner.video.decoder.vp8",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.vp8"
        }
    },

#if 0
    //*17. vp9 decoder
    {
        "OMX.allwinner.video.decoder.vp9",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.vp9"
        }
    },
#endif
    //*18. avc decoder
    {
        "OMX.allwinner.video.decoder.avc",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.avc"
        }
    },

    //*19. avc decoder (secure)
    {
        "OMX.allwinner.video.decoder.avc.secure",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.avc"
        }
    },

    //*20. h265 decoder
    {
        "OMX.allwinner.video.decoder.hevc",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.hevc"
        }
    },

    //*21. h265 decoder (secure)
    {
        "OMX.allwinner.video.decoder.hevc.secure",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.hevc"
        }
    },
#if 0
    //*22. msmpeg4v3 decoder
    {
        "OMX.allwinner.video.decoder.msmpeg4v3",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.msmpeg4v3"
        }
    },
    //*23. divx4 decoder
    {
        "OMX.allwinner.video.decoder.divx4",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.divx4"
        }
    },

    //*24. rx decoder
    {
        "OMX.allwinner.video.decoder.rx",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.divx4"
        }
    },

    //*25. avs decoder
    {
        "OMX.allwinner.video.decoder.avs",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VDEC_LIB_NAME,
        {
            "video_decoder.avs"
        }
    },
#endif
    //*1. avc encoder
    {
        "OMX.allwinner.video.encoder.avc",
        NULL,
        {
            NULL,NULL,NULL,NULL
        },
        NULL,
        OMX_VENC_LIB_NAME,
        {
            "video_encoder.avc"
        }
    },

};

const unsigned int NUM_OF_CORE = sizeof(core) / sizeof(OmxCoreType);
