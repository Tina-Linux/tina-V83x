# qt-textures
Test the R16 platform

## configuration
### set the qt environment variable
~~~
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_INTEGRATION=none
export XDG_RUNTIME_DIR=/dev/shm
~~~

### running program
~~~
qt-textures
~~~

## memory usage
~~~
Name:   qt-textures
State:  S (sleeping)
Tgid:   1277
Pid:    1277
PPid:   117
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 32
Groups:
VmPeak:    64952 kB
VmSize:    26392 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:     13488 kB
VmRSS:     13488 kB
VmData:     5720 kB
VmStk:       136 kB
VmExe:        32 kB
VmLib:     20044 kB
VmPTE:        68 kB
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
voluntary_ctxt_switches:        996
nonvoluntary_ctxt_switches:     274
~~~

## flash usage
~~~
bin
37.5K   qt-textures
37.5K   total

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

37.5K+14.7M=14.7M
~~~
