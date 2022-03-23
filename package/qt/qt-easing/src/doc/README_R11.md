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
qt-easing
~~~

## memory usage
~~~
Name:   qt-easing
State:  S (sleeping)
Tgid:   1502
Pid:    1502
PPid:   630
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    26340 kB
VmSize:    22524 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     12580 kB
VmRSS:     12580 kB
VmData:     4436 kB
VmStk:       136 kB
VmExe:        40 kB
VmLib:     17476 kB
VmPTE:        34 kB
VmSwap:        0 kB
Threads:        1
SigQ:   0/437
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001000
SigCgt: 00000000000a4002
CapInh: 0000000000000000
CapPrm: ffffffffffffffff
CapEff: ffffffffffffffff
CapBnd: ffffffffffffffff
Cpus_allowed:   1
Cpus_allowed_list:      0
voluntary_ctxt_switches:        1064
nonvoluntary_ctxt_switches:     854
~~~

## flash usage
~~~
bin
45.5K   qt-easing
45.5K   total

libs
5.2M    libQt5Core.so.5
4.2M    libQt5Gui.so.5
4.0M    libQt5Widgets.so.5
13.5K   qt5/plugins/generic
201.5K  qt5/plugins/platforms
215.0K  qt5/plugins
215.0K  qt5/
13.6M   total

fonts
9.5M    fonts/
9.5M    total

45.5K+13.6M+9.5M=23.1M
~~~
