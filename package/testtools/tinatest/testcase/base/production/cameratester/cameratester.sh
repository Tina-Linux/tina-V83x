#!/bin/sh

tt_base="/base/production/cameratester"
dir=`mjson_fetch ${tt_base}/image_directory`

! [ -d "${dir}" ] \
	&& echo "No Found: ${dir}, quit!!" \
	&& exit 1

### run camerademo test camera
### argv[0]:camerademo
### argv[1]:tinatest flag
### argv[2]:/dev/videoX
### argv[3]:image directory
for i in `seq 0 5`
do
	[ ! -c "/dev/video$i" ] && [ $i = 0 ] && echo "/dev/video$i does not exist" && exit 1
	[ ! -c "/dev/video$i" ] && break
    echo "------------------- INFO -------------------"
	echo " [cameratester] test /dev/video$i"
	echo " [cameratester] image directory: $dir"
    echo "------------------- END -------------------"
	camerademo tinatest $i ${dir}

	[ $? = 0 ] || exit 1

	file=`ls ${dir}/bmp_*`
	array=${file// /}
	num=0
	for var in $array
	do
		mv $var ${dir}/video$i\_test$num.bmp
		let num+=1
	done
### get image data
	tshowimg "${dir}/video$i" "/dev/video$i captured image is normal?"
	[ $? = 0 ] || exit 1
done
