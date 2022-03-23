sample_ao2ai测试流程：
	根据配置参数读取pcm数据，然后播放声音，从耳机口输出声音。同时ai采集音频数据，并保存为本地文件。

读取测试参数的流程：
	sample提供了sample_ao2ai.conf，测试参数包括：pcm音频文件路径(dst_file)、采样率(pcm_sample_rate)、
	通道数目(pcm_channel_cnt)、数据位宽(pcm_bit_width)、每次取pcm的桢数目(pcm_frame_size)。
	启动sample_ao2ai时，在命令行参数中给出sample_ao2ai.conf的具体路径，sample_ao2ai会读取该文件，完成参数解析。
	然后按照参数运行测试。

从命令行启动sample_ao2ai的指令：
	./sample_ao2ai -path /mnt/extsd/sample_ao2ai/sample_ao2ai.conf
	"-path /mnt/extsd/sample_ao2ai/sample_ao2ai.conf"指定了测试参数配置文件的路径。

测试参数的说明：
(1)pcm_src_path：指定音频pcm文件的路径，该文件是包含wave头(大小为44Bytes)的wav格式文件，如果找不到这种格式文件，可以用sample_ai生成一个。
(2)pcm_dst_path：指定目标文件的路径，该文件是ai组件采集音频生成的文件，不带wav头，如果想听该音频，需手动加上wave头。
(3)pcm_sample_rate：指定采样率，设置为文件中的采样率的值。
(4)pcm_channel_cnt：指定通道数目，设置为文件中的通道数。
(5)pcm_bit_width：指定位宽，设置为文件中的位宽。
(6)pcm_frame_size：固定指定为1024。