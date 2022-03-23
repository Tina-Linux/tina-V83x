# qt-washing-machine
Test the R11 platform

## configuration
### set the qt environment variable
~~~
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_GENERIC_PLUGINS=tslib
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

### ttf font
~~~
adb push yourTtfFont.ttf /usr/share/fonts/
~~~

### running program
~~~
qt-washing-machine
~~~

## memory usage
### in MainPage, not click FunWin
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1454
Pid:    1454
PPid:   117
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    62728 kB
VmSize:    45656 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     19484 kB
VmRSS:     19484 kB
VmData:    17612 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24196 kB
VmPTE:        62 kB
VmSwap:        0 kB
Threads:        5
SigQ:   1/2971
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001000
SigCgt: 00000000800a4002
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   f
Cpus_allowed_list:      0-3
voluntary_ctxt_switches:        132
nonvoluntary_ctxt_switches:     50
~~~

### click FunWin, in FunWin
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1454
Pid:    1454
PPid:   117
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    65016 kB
VmSize:    45936 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     22380 kB
VmRSS:     20564 kB
VmData:    17740 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24308 kB
VmPTE:        66 kB
VmSwap:        0 kB
Threads:        5
SigQ:   1/2971
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001000
SigCgt: 00000000800a4002
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   f
Cpus_allowed_list:      0-3
voluntary_ctxt_switches:        237
nonvoluntary_ctxt_switches:     70
~~~

### return MainPage
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1454
Pid:    1454
PPid:   117
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    67012 kB
VmSize:    48164 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     22796 kB
VmRSS:     22796 kB
VmData:    19968 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24308 kB
VmPTE:        66 kB
VmSwap:        0 kB
Threads:        5
SigQ:   1/2971
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001000
SigCgt: 00000000800a4002
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   f
Cpus_allowed_list:      0-3
voluntary_ctxt_switches:        363
nonvoluntary_ctxt_switches:     143
~~~

## cpu usage
swipe back and forth in the main window
~~~
Mem: 59896K used, 336K free, 40K shrd, 8044K buff, 27440K cached
CPU:  88% usr   0% sys   0% nic  10% idle   0% io   0% irq   0% sirq
Load average: 0.84 0.35 0.13 2/57 1450
  PID  PPID USER     STAT   VSZ %VSZ %CPU COMMAND
 1448   641 root     R    47252  78%  88% qt-washing-machine
  712     1 root     S     1212   2%   0% /bin/adbd -D
 1450  1440 root     R     1072   2%   0% top
   13     2 root     SW       0   0%   0% [kworker/0:1]
    5     2 root     SW       0   0%   0% [kworker/u:0]
  744     1 root     S     1584   3%   0% /sbin/udevd -d
    1     0 root     S     1328   2%   0% /sbin/procd
  773     1 root     S     1120   2%   0% /sbin/logread -f -F /tmp/.lastlog -p /
  641     1 root     S     1076   2%   0% /bin/ash --login
 1440   712 root     S     1068   2%   0% /bin/sh
  772     1 root     S     1008   2%   0% /sbin/logd -S 64
  630     1 root     S      960   2%   0% /sbin/ubusd
  347     2 root     SW       0   0%   0% [nand4]
  344     2 root     SW       0   0%   0% [nand3]
  356     2 root     SW       0   0%   0% [nand7]
    3     2 root     SW       0   0%   0% [ksoftirqd/0]
  302     2 root     SW       0   0%   0% [kswapd0]
  473     2 root     SW       0   0%   0% [jbd2/nande-8]
  341     2 root     SW       0   0%   0% [nand2]
  353     2 root     SW       0   0%   0% [nand6]
~~~

## flash usage
~~~
bin
2.9M    qt-washing-machine
2.9M    total

libs
5.2M    libQt5Core.so.5
4.2M    libQt5Gui.so.5
941.5K  libQt5Network.so.5
2.4M    libQt5Qml.so.5
2.4M    libQt5Quick.so.5
133.5K  libQt5QuickControls2.so.5
777.5K  libQt5QuickTemplates2.so.5
13.5K   qt5/plugins/generic
201.5K  qt5/plugins/platforms
215.0K  qt5/plugins
31.0K   qt5/qml/QtQml/Models.2
14.0K   qt5/qml/QtQml/RemoteObjects
56.5K   qt5/qml/QtQml/StateMachine
110.0K  qt5/qml/QtQml
234.0K  qt5/qml/QtQuick/Controls.2/Fusion
1.9M    qt5/qml/QtQuick/Controls.2/Imagine
294.5K  qt5/qml/QtQuick/Controls.2/Material
245.0K  qt5/qml/QtQuick/Controls.2/Universal
148.5K  qt5/qml/QtQuick/Controls.2/designer/images
298.0K  qt5/qml/QtQuick/Controls.2/designer
3.3M    qt5/qml/QtQuick/Controls.2
329.0K  qt5/qml/QtQuick/Templates.2
22.5K   qt5/qml/QtQuick/Window.2
3.7M    qt5/qml/QtQuick
188.0K  qt5/qml/QtQuick.2
3.9M    qt5/qml
4.2M    qt5/
20.2M   total

fonts
9.5M    fonts/
9.5M    total

2.9M+20.2M+9.5M=32.6M
~~~
