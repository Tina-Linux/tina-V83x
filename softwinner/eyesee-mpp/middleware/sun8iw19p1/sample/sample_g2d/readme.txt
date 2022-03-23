sample_g2d 通过直接处理image图片来演示g2d模块常用功能的使用方式：
g2d 模块功能的选择主要通过参数的组合来实现，如：

1) rotate
	配置conf文件中的dst_rotate 配置项，设置要旋转的角度：0：none,1:90,2:180,3:270.
	Note： 此时 dst_width 及 dst_height配置项要分别与src_width及src_height一致，否则
	会同时启动scale功能，但此时会产生错误结果(g2d模块不能同时做rotation及scaling).

2) scale
	配置conf文件中的dst_rotate为0（disable rotation）,
	配置dst_width dst_height目标大小。
3) cut
    cut功能用来截取部分src内容，具体区域由
    src_rect_x = 
	src_rect_y = 
	src_rect_w = 
	src_rect_h = 
	设定，注意：若无效启动rotation, dst_rotate = 0
4) pixel format格式转换
	该功能通过参数：
	pic_format = nv21
	dst_format = nv12
	来实现。

    
