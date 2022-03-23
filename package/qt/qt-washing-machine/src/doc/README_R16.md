# qt-washing-machine
Test the R16 platform

## configuration
### set the qt environment variable
~~~
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_WIDTH=800
export QT_QPA_EGLFS_HEIGHT=800
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
qt-washing-machine
~~~

## memory usage
### in MainPage, not click FunWin
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1410
Pid:    1410
PPid:   1297
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    62752 kB
VmSize:    45664 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     20196 kB
VmRSS:     20196 kB
VmData:    17640 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24184 kB
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
voluntary_ctxt_switches:        148
nonvoluntary_ctxt_switches:     48
~~~

### click FunWin, in FunWin
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1410
Pid:    1410
PPid:   1297
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    65032 kB
VmSize:    45992 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     22776 kB
VmRSS:     21012 kB
VmData:    17816 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24296 kB
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
voluntary_ctxt_switches:        276
nonvoluntary_ctxt_switches:     74
~~~

### return MainPage
~~~
Name:   qt-washing-mach
State:  S (sleeping)
Tgid:   1410
Pid:    1410
PPid:   1297
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    67132 kB
VmSize:    48220 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     23244 kB
VmRSS:     23244 kB
VmData:    20044 kB
VmStk:       136 kB
VmExe:      2992 kB
VmLib:     24296 kB
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
voluntary_ctxt_switches:        403
nonvoluntary_ctxt_switches:     117
~~~

## flash usage
~~~
bin
2.9M    qt-washing-machine
2.9M    total

libs
5.2M    libQt5Core.so.5
305.5K  libQt5DBus.so.5
389.5K  libQt5EglFSDeviceIntegration.so.5
4.6M    libQt5Gui.so.5
937.5K  libQt5Network.so.5
2.4M    libQt5Qml.so.5
2.7M    libQt5Quick.so.5
133.5K  libQt5QuickControls2.so.5
777.5K  libQt5QuickTemplates2.so.5
35.0K   qt5/plugins/egldeviceintegrations
13.5K   qt5/plugins/generic
9.5K    qt5/plugins/platforms
58.0K   qt5/plugins
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
4.0M    qt5/qml
4.0M    qt5
21.5M   total

fonts
9.5M    fonts/
9.5M    total

2.9M+21.5M+9.5M=33.9M
~~~
