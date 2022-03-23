# Utilities

alsa-ucm-aw 是一个供用户使用 ALSA UCM 配置文件的应用程序

## 使用方法
```sh
alsa-ucm-aw [OPTIONS] <COMMAND>
```

### OPTIONS (选项)

#### -h, --help
显示帮助信息

#### -c, --card NAME
指定 card 的名字. 若没指定, 则会根据 UCM 配置文件选择默认的 card

#### -v, --verb NAME
指定 verb 的名字. 若没指定, 则会根据 UCM 配置文件选择默认的 verb

#### -d, --device NAME
指定 device 的名字. 若没指定, 则会根据 UCM 配置文件选择默认的 device
(在某些情况下不能与指定 modifier 的选项 `-m` 同时使用)

#### -m, --modifier NAME
指定 modifier 的名字. 若没指定, 则会根据 UCM 配置文件选择默认的 modifier
(在某些情况下不能与指定 device 的选项 `-d` 同时使用)

#### -p, --play
指定为 "播放", 用于指定程序寻找默认 verb 时优先选择 "Play", 以及在音量操作上选择
"PlaybackVolume" 变量等 (不能与 `-r` 选项同时使用)

#### -r, --record
指定为 "录制", 用于指定程序寻找默认 verb 时优先选择 "Record", 以及在音量操作上选
择 "CaptureVolume" 变量等 (不能与 `-p` 选项同时使用)

#### -V, --verbose
显示提示信息

### COMMAND (命令)

#### list
列出所有的 card, verb, device 和 modifier
(符号 `[c]` 表示 card, `[v]` 表示 verb, `[d]` 表示 device, `[m]` 表示 modifier)

#### enable
执行 "EnableSequence" 中的内容

#### disable
执行 "DisableSequence" 中的内容

#### get IDENTIFIER
获取 UCM 配置文件中某个变量的值, `IDENTIFIER` 可以为 "PlaybackPCM", "CapturePCM",
"PlaybackVolume" 等名字, 视配置文件的具体内容而定

#### getvol
根据 "PlaybackVolume" 或 "CaptureVolume" 中控件的名字获取它在当前系统中实际的值
(需要与 `-p` 或 `-r` 选项同时使用)

#### setvol [VALUE]
根据 "PlaybackVolume" 或 "CaptureVolume" 中控件的名字设置它在当前系统中实际的值,
若没有指定要设置的具体数值 `VALUE` , 则会将其设置为 "PlaybackVolume" 或
"CaptureVolume" 中定义的默认值 (需要与 `-p` 或 `-r` 选项同时使用)

## 例子

以下使用 "card|verb|device|modifier" 的格式表明指定的设备

- 执行 audiocodec|Play|Headphone|(null) 的 EnableSequence
```sh
alsa-ucm-aw -c audiocodec -v Play -d Headphone enable
```

- 指定了部分设备 (null)|(null)|Headphone|(null), 由程序自动选择所缺部分的默认值
(没有完整指定名字时需要使用 `-p` 或 `-r` 选项)
```sh
alsa-ucm-aw -d Headphone -p enable
```

- 完全由程序自主选择默认设备 (null)|(null)|(null)|(null)
(没有完整指定名字时需要使用 `-p` 或 `-r` 选项)
```sh
alsa-ucm-aw -p enable
```

- 获取 UCM 配置文件中 audiocodec|Play|Headphone|(null) 的 "PlaybackVolume" 的值
```sh
alsa-ucm-aw -c audiocodec -v Play -d Headphone get PlaybackVolume
```

- 获取 audiocodec|Call|Headset|(null) 中 "PlaybackVolume" 所指定的控件的值
(选项 `-p` 对应 "PlaybackVolume", `-r` 则对应 "CaptureVolume")
```sh
alsa-ucm-aw -c audiocodec -v Call -d Headset -p getvol
```

- 设置 audiocodec|Call|Headset|(null) 中 "CaptureVolume" 所指定的控件的值为 35
```sh
alsa-ucm-aw -c audiocodec -v Call -d Headset -c setvol 35
```
