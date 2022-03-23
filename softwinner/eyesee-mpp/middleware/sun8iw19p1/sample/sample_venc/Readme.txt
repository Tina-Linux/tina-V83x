sample_venc测试流程：
	从yuv(sample中格式限定为yuv420p)原始数据文件xxx.yuv中读取视频帧，编码，将取得的编码往输出文件里面直接写。
	生成裸码流视频文件。如果是h264 or h265编码sample会自动在生成文件的开始加上spspps信息，其他格式则不加。

读取测试参数的流程：
	sample提供了sample_venc.conf，测试参数都写在该文件中。
	启动sample_venc时，在命令行参数中给出sample_venc.conf的具体路径，sample_venc会读取sample_venc.conf，完成参数解析。
	然后按照参数运行测试。
	从命令行启动sample_venc的指令：
	./sample_venc -path /mnt/extsd/sample_venc.conf
	"-path /mnt/extsd/sample_venc.conf"指定了测试参数配置文件的路径。

测试参数的说明：
(1)src_file：指定原始yuv文件的路径
(2)src_size：指定原始视频文件的视频大小，如1080p
(3)dst_file: 生成的裸码流视频文件路径
(4)dst_size: 生成的裸码流视频文件大小，如1080p
(5)encoder: 编码方式
(6)dst_framerate: 生成视频文件的帧率
(7)dst_bitrate: 生成视频文件的码率
(8)rc_mode: RC模式
(9)test_duration: sample一次测试时间（单位：s）