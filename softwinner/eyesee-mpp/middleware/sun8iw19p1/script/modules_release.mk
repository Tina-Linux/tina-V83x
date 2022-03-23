/* sample */

/* early depend, so build first */
MODULE:system/public/liblog/build.mk
MODULE:system/public/libion/build.mk
MODULE:system/public/libcutils/build.mk
MODULE:system/public/display/build.mk

MODULE:middleware/lib/build.mk

MODULE:middleware/media/LIBRARY/libcedarx/config/build.mk
MODULE:middleware/media/LIBRARY/libai_common/build.mk
MODULE:middleware/media/LIBRARY/libaiBDII/build.mk
MODULE:middleware/media/LIBRARY/libaiHCNT/build.mk
MODULE:middleware/media/LIBRARY/libaiMOD/build.mk
MODULE:middleware/media/LIBRARY/libevekernel/build.mk
MODULE:middleware/media/LIBRARY/libeveface/build.mk
MODULE:middleware/media/LIBRARY/libisp/isp_cfg/build.mk
MODULE:middleware/media/LIBRARY/libVLPR/build.mk
MODULE:middleware/media/LIBRARY/libkfc/build.mk

MODULE:middleware/sample/configfileparser/build.mk
#MODULE:middleware/sample/sample_adec/build.mk
#MODULE:middleware/sample/sample_ai/build.mk
#MODULE:middleware/sample/sample_ao/build.mk
#MODULE:middleware/sample/sample_demux/build.mk
#MODULE:middleware/sample/sample_demux2vdec/build.mk
#MODULE:middleware/sample/sample_isp/build.mk
#MODULE:middleware/sample/sample_osd/build.mk
#MODULE:middleware/sample/sample_uvcin/build.mk
#MODULE:middleware/sample/sample_venc/build.mk
#MODULE:middleware/sample/sample_vi2vo/build.mk
#MODULE:middleware/sample/sample_vipp2venc/build.mk
#MODULE:middleware/sample/sample_virvi/build.mk
#MODULE:middleware/sample/sample_vo/build.mk
#MODULE:middleware/sample/sample_audioeffectlib/build.mk
#MODULE:middleware/sample/sample_venc2muxer/build.mk
#MODULE:middleware/sample/sample_vi2venc2muxer/build.mk
#MODULE:middleware/sample/sample_demux2vdec2vo/build.mk
