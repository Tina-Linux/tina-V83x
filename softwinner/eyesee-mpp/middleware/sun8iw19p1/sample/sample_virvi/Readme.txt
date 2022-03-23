sample_virvi测试流程：
         该sample测试mpi_vi组件,打开sample_virvi.conf指定的dev节点,mpi_vi采集图像,到达测试次数后,停止运行并销毁vi组件,可以手动按ctrl-c，终止测试。

读取测试参数的流程：
    sample提供了sample_virvi.conf，测试参数都写在该文件中。
启动sample_virvi时，在命令行参数中给出sample_virvi.conf的具体路径，sample_virvi会读取sample_virvi.conf，完成参数解析。
然后按照参数运行测试。
         从命令行启动sample_virvi的指令：
        ./sample_virvi -path /home/sample_virvi.conf
        "-path /home/sample_virvi.conf"指定了测试参数配置文件的路径。

测试参数的说明：
(1)auto_test_count:指定自动测试次数
(2)get_frame_count:指定每次测试获取图像的次数
(3)dev_num：指定VI Dev设备节点 
(4)pic_width：指定camera采集的图像宽度
(5)pic_height：指定camera采集的图像高度
(6)frame_rate:指定camera采集图像的帧率
(7)pic_format:指定camera采集图像像素格式