Function : Yuview is used to display various image including rgb/i420/yv12/nv12/nv21.

Usage :

1. display image : yuview -format nv12 -size 640 480 -file /pic_nv12_640x480.dat
    -format : not necessary, default option is VIDEO_PIXEL_FORMAT_DEFAULT
            options : i420/yv12/nv12/nv21/yuyv
    -size   : necessary, the size of image
    -file   : necessary, filename including path

2. clean frame : yuview clear 1
