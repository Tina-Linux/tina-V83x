
# Author: chenxiaochuan
# Date:   2014-06-18

    EXTERNAL目录存放LIBRARY下各个功能模块依赖的外部库机器头文件，目前包括:
        1. openssl;
        2. zlib;
        3. libxml2;
以下是关于如何从这些外部库的源码编译生成本目录存放的库文件的说明。

1. openssl
    a. 下载openssl 1.0.1e版本的源码，解压；
    b. 在解压目录下执行./config no-asm shared --prefix=/home/cxc/openssl进行配置，
       其中--prefix=/home/cxc/openssl指明库和头文件的输出路径，需根据实际情况修改。
    c. 修改Makefile文件，包括
        CC= gcc 一行修改为
            CC= arm-none-linux-gnueabi-gcc
            如果使用arm-linux-gnueabihf-交叉编译器，则修改为
            CC= arm-linux-gnueabihf-gcc
        AR= ar $(ARFLAGS) r 一行修改为
            AR= arm-none-linux-gnueabi-ar $(ARFLAGS) r
            如果使用arm-linux-gnueabihf-交叉编译器，则修改为
            AR= arm-linux-gnueabihf-ar $(ARFLAGS) r
        RANLIB= /usr/bin/ranlib 一行修改为
            RANLIB= arm-none-linux-gnueabi-ranlib
            如果使用arm-linux-gnueabihf-交叉编译器，则修改为
            RANLIB= arm-linux-gnueabihf-ranlib
        如果对CFLAG的设置包含-m64选项，而目标机器为32位，删除该选项
    d. 在源码目录下执行make，等待编译完成后执行make install
    e. make install后/home/cxc/openssl目录包含库和头文件，将其拷贝到需要放置的地方。

2. zlib
    a. 下载zlib 1.2.8版本的源码，解压；
    b. 在shell中执行
            export CC=arm-none-linux-gnueabi-gcc
       如果使用arm-linux-gnueabihf-交叉编译器，则执行
            export CC=arm-linux-gnueabihf-gcc
    c. 在解压目录下执行
            ./configure --prefix=/home/cxc/zlib
       其中/home/cxc/zlib指明库和头文件的输出路径，需根据实际情况修改；
    d. 在源码目录下执行make，等待编译完成后执行make install
    e. make install后/home/cxc/zlib目录包含库和头文件，将其拷贝到需要放置的地方。

2. libxml2
    a. 下载libxml2 v2.7.8版本源码；
    b. 在源码目录下执行
            ./audogen.sh
    c. 在源码目录下执行
            ./configure --host=arm-none-linux-gnueabi CC=arm-none-linux-gnueabi-gcc \
            LD=arm-none-linux-gnueabi-ld RANLIB=arm-none-linux-gnueabi-ranlib \
            --prefix=/home/AL3/libxml2 --without-zlib
       如果使用arm-linux-gnueabihf-交叉编译器，则执行
            ./configure --host=arm-linux-gnueabihf CC=arm-linux-gnueabihf-gcc \
            LD=arm-linux-gnueabihf-ld RANLIB=arm-linux-gnueabihf-ranlib \
            --prefix=/home/AL3/libxml2 --without-zlib
       其中/home/cxc/zlib指明库和头文件的输出路径，需根据实际情况修改；
    d. 在源码目录下执行make，等待编译完成后执行make install
    e. make install后/home/cxc/libxml2目录包含库和头文件，将其拷贝到需要放置的地方。
