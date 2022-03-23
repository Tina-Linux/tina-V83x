# qt-washing-machine
Test the R16 platform

## configuration
### set the qt environment variable
~~~
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_WIDTH=0
export QT_QPA_EGLFS_HEIGHT=0
export QT_QPA_EGLFS_PHYSICAL_WIDTH=0
export QT_QPA_EGLFS_PHYSICAL_HEIGHT=0
export QT_QPA_EGLFS_INTEGRATION=none
export XDG_RUNTIME_DIR=/dev/shm
export QT_QPA_GENERIC_PLUGINS=tslib
~~~

### set the tslib environment variable
~~~
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event4
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
Tgid:   1340
Pid:    1340
PPid:   126
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    54236 kB
VmSize:    27212 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     15888 kB
VmRSS:     15736 kB
VmData:     6080 kB
VmStk:       136 kB
VmExe:        40 kB
VmLib:     20448 kB
VmPTE:        52 kB
VmSwap:        0 kB
Threads:        3
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
voluntary_ctxt_switches:        998
nonvoluntary_ctxt_switches:     870
~~~

## flash usage
~~~
bin
45.5K   qt-easing
45.5K   total

libs
5.2M    libQt5Core.so.5
305.5K  libQt5DBus.so.5
389.5K  libQt5EglFSDeviceIntegration.so.5
4.6M    libQt5Gui.so.5
4.1M    libQt5Widgets.so.5
35.0K   qt5/plugins/egldeviceintegrations
13.5K   qt5/plugins/generic
9.5K    qt5/plugins/platforms
58.0K   qt5/plugins
58.0K   qt5/
14.7M   total

fonts
9.5M    fonts/
9.5M    total

45.5K+14.7M+9.5M=24.2M
~~~
