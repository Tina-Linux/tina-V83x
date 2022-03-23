#!/bin/ash

function showJpg()
{
	fbviewer /usr/lib/tt-data/sky-800x480.jpg &
	ttrue "Could you see this jpg picture clearly?"
	if [ $? = 0 ]
	then
	echo "GOOD, jpg picture show success."
	dd if=/dev/zero of=/dev/fb0
	return 0
	else
	echo "ERROR, jpg picture show failed."
	dd if=/dev/zero of=/dev/fb0
	exit 1
	fi
}

function showPng()
{
	fbviewer /usr/lib/tt-data/time-800x480.png &
	ttrue "Could you see this png picture clearly?"
	if [ $? = 0 ]
	then
	echo "GOOD, png picture show success."
	showJpg
	return 0
	else
	echo "ERROR, png picture show failed."
	dd if=/dev/zero of=/dev/fb0
	exit 1
	fi
}

fbviewer /usr/lib/tt-data/bird-800x480.bmp &
ttrue "Could you see this bmp picture clearly?"
if [ $? = 0 ]
then
echo "GOOD, bmp picture show success."
showPng
return 0
else
echo "ERROR, bmp picture show failed."
dd if=/dev/zero of=/dev/fb0
return 1
fi
