# r11-board
Test the R11 platform

## configuration
### set the qt environment variable
~~~
export MG_DOUBLEBUFFER=1
~~~

### set the tslib environment variable
~~~
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event2
~~~

### calibrate the touch screen
~~~
ts_calibrate
~~~

### running program
~~~
r11-board
~~~

## memory usage
### in MainPage, not click FunWin
~~~
Name:   r11-board
State:  S (sleeping)
Tgid:   1382
Pid:    1382
PPid:   1372
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    29692 kB
VmSize:    26692 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     12372 kB
VmRSS:     12372 kB
VmData:     8604 kB
VmStk:       136 kB
VmExe:       172 kB
VmLib:     15676 kB
VmPTE:        36 kB
VmSwap:        0 kB
Threads:        5
SigQ:   0/437
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000000005402
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   1
Cpus_allowed_list:      0
voluntary_ctxt_switches:        4808
nonvoluntary_ctxt_switches:     700
~~~

### click FunWin, in FunWin
~~~
Name:   r11-board
State:  S (sleeping)
Tgid:   1382
Pid:    1382
PPid:   1372
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    33264 kB
VmSize:    30264 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     15984 kB
VmRSS:     15984 kB
VmData:    12176 kB
VmStk:       136 kB
VmExe:       172 kB
VmLib:     15676 kB
VmPTE:        38 kB
VmSwap:        0 kB
Threads:        5
SigQ:   0/437
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000000005402
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   1
Cpus_allowed_list:      0
voluntary_ctxt_switches:        8307
nonvoluntary_ctxt_switches:     931
~~~

### return MainPage
~~~
Name:   r11-board
State:  S (sleeping)
Tgid:   1382
Pid:    1382
PPid:   1372
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    33264 kB
VmSize:    27136 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     15984 kB
VmRSS:     12564 kB
VmData:     9048 kB
VmStk:       136 kB
VmExe:       172 kB
VmLib:     15676 kB
VmPTE:        36 kB
VmSwap:        0 kB
Threads:        5
SigQ:   0/437
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000000005402
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   1
Cpus_allowed_list:      0
voluntary_ctxt_switches:        8428
nonvoluntary_ctxt_switches:     1047
~~~

## cpu usage
swipe back and forth in the main window
~~~
Mem: 56772K used, 3460K free, 52K shrd, 12104K buff, 19104K cached
CPU:  49% usr   4% sys   0% nic  46% idle   0% io   0% irq   0% sirq
Load average: 0.57 0.35 0.14 1/238 1578
  PID  PPID USER     STAT   VSZ %VSZ %CPU COMMAND
 1274  1264 root     S    46528  77%  52% r11-board
    5     2 root     SW       0   0%   1% [kworker/u:0]
  721     1 root     S     1144   2%   0% /bin/adbd -D
 1404  1397 root     R     1072   2%   0% top
    3     2 root     SW       0   0%   0% [ksoftirqd/0]
  331     2 root     SW       0   0%   0% [kworker/0:2]
  732     1 root     S     1608   3%   0% /sbin/udevd -d
    1     0 root     S     1316   2%   0% /sbin/procd
 1264     1 root     S     1168   2%   0% {S99r11board} /bin/sh /etc/rc.common /
  762     1 root     S     1120   2%   0% /sbin/logread -f -F /tmp/.lastlog -p /
 1397   721 root     S     1072   2%   0% /bin/sh
  631     1 root     S     1068   2%   0% /bin/ash --login
  761     1 root     S     1008   2%   0% /sbin/logd -S 64
  626     1 root     S      960   2%   0% /sbin/ubusd
  344     2 root     SW       0   0%   0% [nand3]
  347     2 root     SW       0   0%   0% [nand4]
   13     2 root     SW       0   0%   0% [kworker/0:1]
  471     2 root     SW       0   0%   0% [jbd2/nande-8]
  341     2 root     SW       0   0%   0% [nand2]
  353     2 root     SW       0   0%   0% [nand6]
~~~

## flash usage
~~~
bin
177.0K  r11-board
177.0K  total

libs
69.0K   libmgeff-1.2.so.0.0.0
1.3M    libmgi-2.0.so.4.0.0
1.3M    libmgncs-1.2.so.0.0.0
185.0K  libmgncs4touch-1.2.so.0.0.0
1.5K    libmgncs4touch.la
680.0K  libmgplus-1.4.so.0.0.0
80.5K   libmgutils-1.2.so.0.0.0
1.3M    libminigui_ths-3.2.so.0.0.0
4.9M    total

res
512     res/Pointercal
2.5K    res/SetAudioPass
7.0K    res/config
9.5M    res/font
2.9M    res/menu
512     res/networkres
11.0M   res/other
137.5K  res/sound
23.6M   res/
23.6M   total

177.0K+4.9M+23.6M=28.7M
~~~
