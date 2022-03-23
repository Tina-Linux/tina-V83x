#!/bin/ash

yuview -format nv21 -size 640 480 -file /usr/lib/tt-data/pic_nv21_640x480_1.dat

ttrue "Could you see this yuv picture clearly?"
if [ $? = 0 ]
then
echo "GOOD, yuv picture show success."
yuview clear 1 -size 640 480
return 0
else
echo "ERROR, yuv picture show failed."
yuview clear 1 -size 640 480
return 1
fi
