sample_ai2ao测试流程：
	根据配置参数采集对应的pcm数据，并将其送往ao输出音频。

读取测试参数的流程：
	sample提供了sample_ai2ao.conf，测试参数包括：生成的pcm文件路径(pcm_file_path)、采样率(pcm_sample_rate)、
	通道数目(pcm_channel_cnt)、数据位宽(pcm_bit_width),以及测试模式(tunnel_flag)
	启动sample_ai2ao时，在命令行参数中给出sample_ai2ao.conf的具体路径，sample_ai2ao会读取该文件，完成参数解析。
	然后按照参数运行测试。

从命令行启动sample_ai的指令：
	./sample_ai2ao -path /mnt/extsd/sample_ai2ao/sample_ai2ao.conf
	"-path /mnt/extsd/sample_ai2ao/sample_ai2ao.conf"指定了测试参数配置文件的路径。

测试参数的说明：
(1)pcm_file_path：指定目标pcm文件的路径，该文件是包含wave头的wav格式文件。
(2)pcm_sample_rate：指定采样率，通常设置为8000。
(3)pcm_channel_cnt：指定通道数目，通常为1或2。
(4)pcm_bit_width：指定位宽，必须设置为16。
(5)pcm_frame_size：指定frame_size，此值可不指定。
(6)tunnel_flag：指定tunnel模式，为1或0。1代表绑定模式，即ai绑定ao，通过手动ctrl+c结束；0代表非绑定模式，采集10秒的音频并推出。