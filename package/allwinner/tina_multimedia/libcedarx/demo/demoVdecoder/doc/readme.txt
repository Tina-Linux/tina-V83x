step 1:  copy demon to SD card or U disk
step 2:  mount /dev/mmcblk0p1 /mnt
	or   mount /dev/sda1 /mnt
step 3:
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/app/lib
step 4:  ./demoVdecoder

./demoVdecoder -h   #show help

example
./demoVdecoder -i /mnt/TestStream/h265.mp4 -n 50 #decode h265.mp4 50 frames
