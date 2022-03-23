软件版本
#define PROGRAM_VERSION			"v0.1"

目录结构
./bin                  目标执行文件
./obj                  编译临时文件
./Tools                安卓测试软件，安装版本4.42以上
./radiotap             radiotap库，详情请google radiotap
./wpa_supplicant.conf  wpa_supplicant的配置文件，make的时候会将其拷贝到目标板

说明
1)、目前使用的Airkiss是未通信 AES加密的，测试软件请勿设置AES密码否则，无法配网
2)、发现设备是广播UDP包，发现成功测试软件会个短暂提示BINGO

开启配网通信加密步骤
1)、makefile 文件 LIBS = -lairkiss   改为 LIBS = -lairkiss_aes
2)、include 文件夹内 airkiss.h头文件开启   #define AIRKISS_ENABLE_CRYPT 1
3)、重新编译

网络连接说明
1）、支持WPA、WEP、OPEN

运行参数
1)、详情请通过参数 -h获取
