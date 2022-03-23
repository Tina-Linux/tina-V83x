一、简介
    tina 使用busybox init方式启动，首先调用执行pseudo_init（挂载文件系统，如/proc、/tmp、/sys
/etc、/usr），接着会调用/sbin/init进程，而init进程调用的第一个启动脚本为/etc/init.d/rcS。


------------------------------------------------------------------------------------------
二、平台的自定义
    不同的平台文件系统具有其共性与特殊性。tina/packge/busybox-init-base-files/files下提供了
所有平台的基础文件。而在tina/target/allwinner/XXX/busybox-init-base-files下存放的是平台特性
文件，其优先级高于前者,即当前者目录和后者存在有相同文件时，以后者为准。如有以下两个文件:

A：tina/target/allwinner/r11-R11_pref1/busybox-init-base-files/etc/banner

B：tina/package/busybox-init-base-files/files/etc/banner

最终拷贝到文件系统中的为A。


------------------------------------------------------------------------------------------
三、pseudo_init与rcS
    pseudo_init与rcS文件中存在很多平台共性的代码，避免系统充斥大量冗余代码,以及方便基础文件的维护和开发。
所以不允许在特定平台下自定义pseudo_init、rcS文件（必须使用tina/packge/busybox-init-base-files/files下
的pseudo_init、rcS）。
    如果需要添加平台特定配置（pseudo_init，rcS没有配置），可将其写到rc.preboot,rc.final中，参考第四节。


------------------------------------------------------------------------------------------
四、rcS脚本
1.功能描述
（1）执行/etc/init.d/rc.preboot。
	为了满足开机快速启动的需求，提供了用户可自定义rc.preboot文件，
即在tina/target/allwinner/XXX/busybox-init-base-files/etc/init.d/目录下创建rc.preboot脚本文件，
将会被rcS最先调用执行。

（2）配置打印级别，主机名称。

（3）执行/etc/init.d/rc.log，配置系统log信息。

	系统默认使用的是tina/package/busybox-init-base-files/files/etc/init.d/rc.log脚本进行
配置系统log信息。用户可在tina/target/allwinner/XXX/busybox-init-base-files/etc/init.d/
下创建rc.log，自定义rc.log。

	如果需要使用默认rc.log，需要在make menuconfig配置。
	Base system  --->
	 busybox-init-base-files......................... Busybox init base system  --->
	  [*]   Use the rc.log

（4）挂载UDISK。

（5）执行/etc/init.d/rc.modules，加载内核模块。
	系统默认使用的是tina/package/busybox-init-base-files/files/etc/init.d/rc.modules脚本
进行内核模块自加载，用户可在tina/target/allwinner/XXX/busybox-init-base-files/etc/init.d/
下创建rc.modules，自定义rc.modules。

	如果需要使用默认rc.modules，需要在make menuconfig配置如下。
	Base system  --->
	 busybox-init-base-files......................... Busybox init base system  --->
	   [*]   Use the rc.modules


（6）启动/etc/rc.d下的脚本。

	关于执行rc.d下的启动脚本，目的为兼容procd式的应用脚本。/etc/rc.d下的脚本是链接到/etc/init.d/下，
默认情况下只执行adbd,如果需要执行其他脚本，需要在tina/target/allwinner/XXX/busybox-init-base-files/etc/init.d/下，
自定义load_script.conf文件，文件内容中写上要启动的应用，如adbd（注意，每一个应用占一行）。可参考：
tina/packge/busybox-init-base-files/files/etc/init.d/load_script.conf。

	如果需要执行rc.d下的启动脚本，需要在make menuconfig做如下配置。
	Base system  --->
	 busybox-init-base-files......................... Busybox init base system  --->
	   [*]   Auto load the script in /etc/rc.d

（7）ota初始化。

（8）执行/etc/init.d/rc.final,用户自定义启动脚本。
	用户可在tina/packge/busybox-init-base-files/files/etc/init.d/下创建一个rc.final脚本，自定义
启动应用程序,该脚本将会被rcS最后调用执行。


2.rc.preboot与rc.final的区别？
   rc.preboot比rc.final先运行，在执行rc.preboot脚本的时候，系统的一些初始化操作还没完成，如挂载UDISK、
内核模块自加载、ota等等操作。而rc.final执行的时候，以上的初始化操作已经完成。

五.如何写应用的启动脚本

example：开机自启动smartlinkd（tina/package/allwinner/smartlinkd/files/smartlinkd.init）
1.方法一（特定格式要求）

详细的格式参考：
https://wiki.openwrt.org/inbox/procd-init-scripts
https://wiki.openwrt.org/doc/techref/initscripts

（1）procd式
------------------------------------------------------------------------------------------
#!/bin/sh /etc/rc.common   #本质为script脚本,以#!开头, 之后执行/etc/rc.common
START=98		#开机启动优先级(序列) [数值越小, 越先启动]
STOP=98			#关机停止优先级(序列) [数值越小, 越先关闭]

USE_PROCD=1
PROG=smartlinkd

start_service() {    #启动函数
    procd_open_instance
    procd_set_param command $PROG -d
    procd_close_instance
}

shutdown() {
    echo shutdown
}
------------------------------------------------------------------------------------------

（2）Sys式
------------------------------------------------------------------------------------------
#!/bin/sh /etc/rc.common
START=98
STOP=98

PROG=smartlinkd

start() {
  smartlinkd -d &
}

------------------------------------------------------------------------------------------

使用上述procd式和sys式脚本，既能兼容procd init启动和busybox init的启动方式。
另外如果使用的是busybox init的启动方式，还需要在load_script.conf文件中换行添加内容：smartlinkd



2.方法二（无特定格式要求）
创建rc.preboot或者rc.final脚本，添加启动smartlinkd的内容。
