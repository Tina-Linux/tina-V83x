#增加说明 luckylau
#修改2016-4-14 增加分区相关说明，删除target_sys中usr.img部分，增加大容量升级说明
#修改2016-3-23  初始版本
#author：henrisk

AW-OTA使用须知：
    由于实际应用中存储介质大小各异，其相应OTA方案也会不同。对于小于32M（一般为spinor）的介质，需要预先设定：
    make menuconfig
    Target Images  --->
        *** Image Options ***
            [*] For storage less than 32M, enable this when using ota
    选中该配置项后，rootfs的/usr会被分拆出部分生成usr.squashfs(usr.img)存放在extend分区并与recovery镜像（boot_initramfs）复用该分区，以此起到节省存储空间的作用。

    而对于大容量介质，建议不选中该配置项，即不需要usr.img和extend分区，而只需要添加recovery分区，这样在OTA升级时会省去很多麻烦。

    不管是小容量还是大容量，都要在make之前选中应用包misc-package：
    make menuconfig
        Allwinner  --->
            <*> misc-upgrade........................... read and write the misc partition

一、分区定义：
    boot分区：存内核镜像
    rootfs分区：基础系统镜像分区（/lib, /bin, /etc, /sbin等非/usr，非挂载其他分区的路径，wifi支持环境，alsa支持环境、OTA环境）
    extend分区：扩展系统镜像分区（/usr 应用分区）
    上面三个分区为升级分区

    private分区：存储SN号分区
    misc分区：系统状态、刷机状态分区
    UDISK分区：用户数据分区（/mnt/UDISK）
    overlayfs分区：存储overlayfs覆盖数据
    上面分区为不升级分区

二、分区大小注意事项：
    分区大小在方案使用sys_partition.fex中定义
    size的算法：如 8192/2/1024 = 4M

    a）配置boot分区大小，boot分区大小需要依赖内核配置，需要小于等于sys_partition.fex中定义的boot标签的定义：
    如：
        [partition]
            name         = boot
            size         = 8192
            downloadfile = "boot.fex"
            user_type    = 0x8000

        boot分区镜像大小需要在menuconfig中预先设定：
        make menuconfig
            Target Images  --->
                *** Image Options ***
                (4) Boot filesystem partition size (in MB)

    b）rootfs分区的大小，不需要通过make menuconfig去设定，直接根据镜像大小修改分区文件即可。
        1）对于一些小容量flash的方案（如16M），需在/bin 下存放联网逻辑程序、版本控制程序、下载镜像程序、播报语音程序以及语音文件（这些文件在编译时应该install到/bin或者/lib下）
       可以在固件编译完后，查看bin/sunxi(sun5i)/下rootfs.img的大小在决定sys_partition.fex中rootfs分区的大小，如
       \*0*/ $ ll bin/sun5i/rootfs.img
        -rw-r--r-- 1 heweihong heweihong  1835008  4月 14 16:44 bin/sun5i/rootfs.img

        2）对于大容量flash的方案（如128M以上，或者有足够的flash空间存相关镜像），不需要1）中那些OTA额外的程序，直接查看rootfs.img的大小设定分区文件即可。

    c）extend分区的大小，需要考虑多个方面：
        1）编译后 usr.img的大小
        2）make_ota_image后initramfs镜像的大小（make_ota_image见后面说明）
        如：
        \*0*/ $ ll build_dir/target-arm_cortex-xxxxxxxx/linux-sun5i（linux-sunxi）/
        -rw-r--r--  1 heweihong heweihong   479232  4月 14 16:44 usr.squashfs
        -rwxr-xr-x  1 heweihong heweihong  5510192  4月 14 16:44 zImage-initramfs*
        取两个最大值，并增加一些余量即可

        并把这个值设置为initramfs镜像的大小
        make menuconfig
            Target Images  --->
                *** Image Options ***
                (8) Boot-Recovery initramfs filesystem partition size (in MB)

    d）其他分区如private、misc等使用默认的大小即可
    e）剩下的空间全部自动分配进入UDISK分区（一定要留取一定空间给UDISK分区，至少可以格式化，挂载，一些OTA过程会在里面写一些中间文件，小容量flash的方案，也要保证有256K~512K的空间）

    特别注意：这些分区大小不能通过OTA去修改的，所以对于大容量flash的方案，应该在满足分区条件限制（如上面adc三点）的情况下留有足够的余量，满足后续OTA增加内容的需求。
    对于小容量flash的方案，需要在增加内容是调节相关分区的大小。

三、misc-upgrade升级
1. misc-upgrade 是基于小容量flash方案重新划分分区后，以misc分区、extend分区为媒介设计的OTA方案

2. OTA镜像包SDK编译说明：(SDK根目录)
   环境变量：
   source scripts/setenv.sh

   编译命令：
   make_ota_image (在新版本代码已经成功编译出烧录固件的环境的基础上，打包OTA镜像)
   make_ota_image --force (重新编译新版本代码，然后再打包OTA镜像)

   注：在执行make_ota_image之前需要配置支持ramdisk并选用xz压缩cpio
            make menuconfig
                target Images  --->
                    [*] ramdisk  --->
                        --- ramdisk
                            Compression (xz)  --->

3. OTA镜像包说明：
    \*0*/ $ ll bin/sunxi（sun5i）/ota/
    ?????? 20856
    -rw-rw-r-- 1 heweihong heweihong  5731339  3?? 23 15:48 ramdisk_sys.tar.gz
    -rw-rw-r-- 1 heweihong heweihong 10335244  3?? 23 15:48 target_sys.tar.gz
    -rw-rw-r-- 1 heweihong heweihong  5116895  3?? 23 15:48 usr_sys.tar.gz

    三个tar包就是OTA的压缩镜像包
    ramdisk_sys.tar.gz：ramdisk镜像（要升级内核分区、rootfs分区时使用，防止烧写过程掉电，导致机器变砖）
    target_sys.tar.gz： 系统镜像（升级内核分区、rootfs分区）
    usr_sys.tar.gz：    应用分区镜像（升级extend分区，只需要使用这个镜像）
    ----------------------------------------------------------------
    \*0*/ $ ll bin/sunxi/ota/*_sys/
    bin/sunxi/ota/ramdisk_sys/:
    -rw-r--r-- 1 heweihong heweihong 7340032  4月 16 12:50 boot_initramfs.img

    bin/sunxi/ota/target_sys/:
    -rw-r--r-- 1 heweihong heweihong 3145728  4月 16 12:49 boot.img
    -rw-r--r-- 1 heweihong heweihong 2883584  4月 16 12:49 rootfs.img

    bin/sunxi/ota/usr_sys/:
    -rw-r--r-- 1 heweihong heweihong 2752512  4月 16 12:49 usr.img

    这四个镜像包为不压缩的img包。

4. 小机端OTA升级命令：
    必选参数：-f -p 二选一
    aw_upgrade_process.sh -f 升级完整系统（内核分区、rootfs分区、extend分区）
    aw_upgrade_process.sh -p 升级应用分区（extend分区）
    注：对于大容量，用aw_upgrade_normal.sh替代aw_upgrade_process.sh
    可选参数: -l，-d -u, -n

    a）对于大容量flash方案可以使用本地镜像，如主程序下载校验好三个镜像后（ramdisk_sys.tar.gz，target_sys.tar.gz、usr_sys.tar.gz），
       存在/mnt/UDISK/misc-upgrade中，调用上的命令，对于自动烧写分区，就算期间掉电，重启后升级程序也能自动完成烧写，不需要依赖网络。
    -l arg，带路径参数。
    如：aw_upgrade_process.sh -p(-f) -l /mnt/UDISK/misc-upgrade (注：mnt前的根目录"/"最好带上，misc-upgrade后不要带"/")
    （-l参数，其他-d、-u、-n参数无效，使用压缩镜像包）

    b）对于小容量flash方案不能使用-l参数，升级区间出错重启后，还需要根据相关的联网下载程序获取镜像（见第5点说明）

    -d arg -u arg，同时使用，-d 参数为可以ping通的OTA服务器的地址、-u 参数为镜像的下载地址
    -n 一些小ddr的方案（如剩余可使用内存在20m以下的方案），可以使用这个参数，shell会直接请求下载不压缩的4个img文件，这样子设备下载后不需要tar解压，减少内存使用。

    如：aw_upgrade_process -f -d 192.168.1.140 -u http://192.168.1.140/
    升级shell会先ping -d 参数（ping 192.168.1.140），ping通过后，会根据升级命令和系统当前场景请求下载：
        无-n参数：
        http://192.168.1.140/ramdisk_sys.tar.gz
        http://192.168.1.140/target_sys.tar.gz
        http://192.168.1.140/usr_sys.tar.gz
        有-n参数：
        http://192.168.1.140/boot_initramfs.img
        http://192.168.1.140/boot.img
        http://192.168.1.140/rootfs.img
        http://192.168.1.140/usr.img

    使用-n参数的方案需要部署上服务器上的镜像是：boot_initramfs.img, boot.img, rootfs.img, usr.img
    不使用-n参数的方案需要部署上服务器上的镜像是：ramdisk_sys.tar.gz, target_sys.tar.gz, usr_sys.tar.gz

5. 脚本接口说明：
    对于小容量flash的方案，没有空间存储镜像，相关镜像只会存在ram中，断电就会丢失。
    假如升级过程断电，需要在重启后重新下载镜像。
    aw_upgrade_vendor.sh设计为各个厂家实现的钩子，SDK上只是个demo可以随意修改。

    实现联网逻辑
    check_network_vendor(){
        return 0 联网成功（如：可以ping通OTA镜像服务器）
        return 1 联网失败
    }

    请求下载目标镜像， $1：ramdisk_sys.tar.gz $2：/tmp
    download_image_vendor(){
        # $1 image name  $2 DIR  $@ others
        rm -rf $2/$1
        echo "wget $ADDR/$1"
        wget $ADDR/$1 -P $2
    }

    开始烧写分区状态：
    aw_upgrade_process.sh -p 主动升级应用分区的模式下，返回0开始写分区  1不写分区
    aw_upgrade_process.sh -f 不理会这个返回值
    upgrade_start_vendor(){
        # $1 mode: upgrade_pre,boot-recovery,upgrade_post
        #return   0 -> start upgrade;  1 -> no upgrade
        #reutrn value only work in nornal mode
        #nornal mode: $NORMAL_MODE
        echo upgrade_start_vendor $1
        return 0
    }

    写分区完成
    upgrade_finish_vendor(){
        #set version or others
        reboot -f
    }

    -f (-n)调用顺序：
        check_network_vendor ->
          upgrade_start_vendor ->
            download_image_vendor (ramdisk_sys.tar.gz, -n 为 boot_initramfs.img)->
              内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                download_image_vendor(target_sys.tar.gz, -n 为 boot.img rootfs.img) ->
                  内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                    download_image_vendor(usr_sys.tar.gz, -n 为 usr.img) ->
                      内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                        upgrade_finish_vendor
    -p调用顺序
        check_network_vendor ->
          download_image_vendor (usr_sys.tar.gz) ->
            upgrade_start_vendor ->
              检测返回值，烧写 ->
                upgrade_finish_vendor

6. 相关系统状态读写
   相关的信息存储在misc分区，OTA升级不会清除这个分区（重新烧写镜像会擦除）

   读 read_misc [command] [status] [version]
   command 表示升级的系统状态（shell脚本处理使用）
   status  自由使用，表示用户自定义状态
   version 自由使用，表示用户自定义状态

   写 write_misc [ -c command ] [ -s status ] [ -v version ]
   -c 不能随意修改，只能有aw-upgrade shell修改
   -s -v 自定义使用
