sample_g2d测试流程：
    该sample对两幅图进行处理，可以叠加、缩放、旋转。

读取测试参数的流程：
    sample提供了sample_g2d.conf，测试参数都写在该文件中。
    启动sample_g2d时，在命令行参数中给出sample_g2d.conf的具体路径，sample_g2d会读取sample_g2d.conf，完成参数解析。
    然后按照参数运行测试。
    从命令行启动sample_g2d的指令：
    ./sample_g2d -path /mnt/extsd/sample_g2d.conf
    "-path /mnt/extsd/sample_g2d.conf"指定了测试参数配置文件的路径。
	使用"./sample_g2d -h"命令来获取帮助

测试参数的说明：
(1)src_img：指定原始图像路径
(2)src_img_w：指定原始图像的宽度
(3)src_img_h：指定原始图像的高度
(4)src_rect_x：原始图像的矩形框X坐标
(5)src_rect_y：原始图像的矩形框Y坐标
(6)src_rect_w：原始图像矩形框宽度
(7)src_rect_h：原始图像矩形框高度
(8)dst_img：指定目的图像路径
(9)dst_img_w：指定目的图像的宽度
(10)dst_img_h：指定目的图像的高度
(11)dst_rect_x：原始目的的矩形框X坐标
(12)dst_rect_y：原始目的的矩形框Y坐标
(13)dst_rect_w：原始目的矩形框宽度
(14)dst_rect_h：原始目的矩形框高度
(15)g2d_mode：G2D处理模式（0:blt 旋转 2:stretchblt 缩放叠加）
(16)output：处理后的图像输出路径

当前所支持的模式如下：
0：blt 做图像旋转
    对应的配置如下(注意源与目的要长宽相反)：
    g2d_mode = 0
    src_img_w = 1920
    src_img_h = 1080 
    src_rect_x = 0   
    src_rect_y = 0   
    src_rect_w = 1920
    src_rect_h = 1080

    dst_img_w = 1080
    dst_img_h = 1920 
    dst_rect_x = 0 
    dst_rect_y = 0
    dst_rect_w = 1080
    dst_rect_h = 1920
2：stretchblt 图像缩放与叠加
    对应的配置如下：
    g2d_mode = 2
    src_img_w = 1920
    src_img_h = 1080 
    src_rect_x = 0   
    src_rect_y = 0   
    src_rect_w = 1920
    src_rect_h = 1080

    dst_img_w = 1280
    dst_img_h = 720 
    dst_rect_x = 0 
    dst_rect_y = 0
    dst_rect_w = 1280 #表示显示区域
    dst_rect_h = 720  #可与宽高不同
