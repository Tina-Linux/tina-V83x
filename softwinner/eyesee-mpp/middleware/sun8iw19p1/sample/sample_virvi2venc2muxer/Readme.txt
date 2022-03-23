sample_vi2venc2muxer测试流程：
	从camera节点取vi输入数据，并对数据进行编码封装，生成对应的视频输出文件。
	
20180818:
	添加对彩转灰和3D去噪声的功能测试。

读取测试参数的流程：
	sample提供了sample_vi2venc2muxer.conf，测试参数都写在该文件中。
	启动sample_vi2venc2muxer时，在命令行参数中给出sample_vi2venc2muxer.conf的具体路径，sample_vi2venc2muxer会读取sample_vi2venc2muxer.conf，完成参数解析。
	然后按照参数运行测试。
	从命令行启动sample_vi2venc2muxer的指令：
	./sample_vi2venc2muxer -path /mnt/extsd/sample_vi2venc2muxer.conf
	"-path /mnt/extsd/sample_vi2venc2muxer.conf"指定了测试参数配置文件的路径。

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