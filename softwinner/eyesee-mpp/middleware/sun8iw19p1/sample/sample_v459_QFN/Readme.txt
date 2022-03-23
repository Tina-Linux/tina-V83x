sample_v459_QFN测试流程：
  VIPP0, vir0：bufferNum=4，采集分辨率1920x1080，格式LBC2.5，h264编码帧率20，码率1Mbit/s，threshSize：w*h/10, 普通NormalP编码，vbv缓存1秒。
  VIPP1-vir0: bufferNum=4，采集分辨率640x480，格式NV21，h264编码帧率20，码率500Kbit/s，threshSize：w*h/10, 普通NormalP编码，vbv缓存1秒。
  VIPP1-vir1: h264编码帧率20，码率500Kbit/s，threshSize：w*h/10, 普通NormalP编码，vbv缓存1秒。
  VIPP1-vir2: h264编码帧率20，码率500Kbit/s，threshSize：w*h/10, 普通NormalP编码，vbv缓存1秒。
  VIPP2-vir0：bufferNum=4，采集分辨率640x480，格式NV21，暂不计算检测库内部占用的内存数量。

读取测试参数的流程：
	sample提供了sample_v459_QFN.conf，测试参数都写在该文件中。
	启动sample_v459_QFN时，在命令行参数中给出sample_v459_QFN.conf的具体路径，sample_v459_QFN会读取sample_v459_QFN.conf，完成参数解析。
	然后按照参数运行测试。
	从命令行启动sample_v459_QFN的指令：
	./sample_v459_QFN -path /mnt/extsd/sample_v459_QFN.conf
	"-path /mnt/extsd/sample_v459_QFN.conf"指定了测试参数配置文件的路径。

测试参数的说明：
(1)dev_node：视频数据的获取节点（0:表示/dev/video0  1:表示/dev/video1）
(2)src_size：设定源视频数据的大小，如1080/720
(3)video_dst_file: 生成的视频文件路径（默认录制一个文件的时长为1分钟，如果sample测试超过一分钟，则下个文件会自动的在此文件名上加上数字后缀，以示区分）
(4)video_size: 生成的视频文件大小，如1080p
(5)video_encoder: 视频编码方式
(6)video_framerate: 生成视频文件的帧率
(7)video_bitrate: 生成视频文件的码率
(8)video_duration: 生成的单个视频文件最大时长（单位：ms）
(9)test_duration: sample一次测试时间（单位：s）
(10)color2grey: 彩转灰
(11)3dnr:3D去噪声
(12)AdvancedRef_Base:高级跳帧参考的base设置,0表示关闭; 如果开启高级跳帧参考,建议10.
(13)AdvancedRef_Enhance:高级跳帧参考的enhance设置,建议1.
(14)AdvancedRef_RefBaseEn:高级跳帧参考的RefBase设置. 1:开启,0:关闭.