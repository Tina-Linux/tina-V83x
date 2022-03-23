2014/5/5 v0.5

新增功能：
1. 支持A33
	1) totddr：系统总带宽
	2) cpuddr：CPU带宽
	3) gpuddr：GPU带宽
	4) de_ddr：DE带宽
	5) ve_ddr：VE带宽
	6) othddr：其他带宽

2014/3/21 v0.4

新增功能：
1. 统计打开mtop到当前的平均值
2. 统计当前模块带宽最大值

显示格式说明
  --打开mtop到当前的最大值
  |
Max: 1150, Average: 623 --打开mtop到当前的平均值
Total: 766, Delta max: 198 --当前模块带宽最大值
  |
  --当前的总带宽
cpuddr0 gpuddr0 de_ddr0 vcfddr0 othddr0 cpuddr1 gpuddr1 de_ddr1 vcfddr1 othddr1
     68     198     118       0       0      68     196     118       0       0
   8.88   25.85   15.40    0.00    0.00    8.88   25.59   15.40    0.00    0.00

2014/3/12 v0.3.1

修复功能：
1. 添加统计Latency, Command Number和Clock Cycle Number使能开关

2014/3/12 v0.3

新增功能：
1. 调整显示方式
2. 支持统计Latency, Command Number和Clock Cycle Number
3. 支持统计模块带宽百分比

2014/3/11 v0.2

新增功能：
1. 支持显示从mtop打开到当前的最大带宽
2. 支持显示当前时间间隔内的总带宽

2014/3/8 v0.1

主要功能：
1. 支持统计CPU, GPU, DE, VE/CSI/FD, Other master带宽
2. 不支持单独统计带宽
3. 支持输出结果到文件
4. 支持设置统计单位
5. 支持设置统计时间间隔

Usage: mtop [-n iter] [-d delay] [-m] [-o FILE] [-h]
    -n NUM   Updates to show before exiting.
    -d NUM   Seconds to wait between update.
    -m unit: MB
    -o FILE  Output to a file.
    -v Display mtop version.
    -h Display this help screen.
